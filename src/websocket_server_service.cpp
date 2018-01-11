//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/websocket_server_service.hpp>

#include <logsys/log.hpp>
#include <logsys/stdlogb.hpp>

#include <cassert>


namespace http::websocket::server{


	using boost::system::error_code;


	namespace{


		std::shared_ptr< std::string const > build_data_frame_data(
			websocket::frame::opcode opcode,
			std::string const& message
		){
			std::string data;

			// preallocate string memory
			if(message.size() < 126){
				data.reserve(message.size() + 2);
			}else if(message.size() < 0x10000){
				data.reserve(message.size() + 4);
			}else{
				data.reserve(message.size() + 10);
			}

			data += static_cast< char >(0x80 | opcode);

			if(message.size() < 126){
				data += static_cast< char >(message.size());
			}else if(message.size() < 0x10000){
				data += static_cast< char >(126);
				data += static_cast< char >(message.size() >> 0x08);
				data += static_cast< char >(message.size() >> 0x00);
			}else{
				data += static_cast< char >(127);
				data += static_cast< char >((message.size() >> 0x1C >> 0x1C)
					& 0x8F);
				data += static_cast< char >(message.size() >> 0x1C >> 0x14);
				data += static_cast< char >(message.size() >> 0x1C >> 0x0C);
				data += static_cast< char >(message.size() >> 0x1C >> 0x04);
				data += static_cast< char >(message.size() >> 0x18);
				data += static_cast< char >(message.size() >> 0x10);
				data += static_cast< char >(message.size() >> 0x08);
				data += static_cast< char >(message.size() >> 0x00);
			}

			data += message;

			return std::make_shared< std::string const >(std::move(data));
		}

		std::shared_ptr< std::string const > build_control_frame_data(
			websocket::frame::opcode opcode,
			std::string const& message
		){
			std::string data;
			data += static_cast< char >(0x80 | opcode);

			if(message.size() < 126){
				data += static_cast< char >(message.size());
				data += message;
			}else{
				throw std::logic_error(
					"control frame message is larger than 125 bytes: "
					+ std::to_string(message.size()));
			}

			return std::make_shared< std::string const >(std::move(data));
		}

		std::shared_ptr< std::string const > build_close_frame_data(
			websocket::frame::opcode opcode,
			std::uint16_t status,
			std::string const& reason
		){
			std::string status_string = {
				static_cast< char >((status >> 8) & 0xFF),
				static_cast< char >((status     ) & 0xFF),
			};

			return build_control_frame_data(opcode, status_string + reason);
		}


	}


	service::service(
		data_callback_fn const& utf8_callback,
		data_callback_fn const& binary_callback,
		info_callback_fn const& new_connection_callback,
		info_callback_fn const& connection_close_callback
	):
		utf8_callback_(utf8_callback),
		binary_callback_(binary_callback),
		new_connection_callback_(new_connection_callback),
		connection_close_callback_(connection_close_callback),
		shutdown_(false)
		{}

	void service::add_connection(
		http::server::connection_ptr const& connection
	){
		if(shutdown_) return;

		connection->ready_callback([this](
			http::server::connection_ptr const& connection,
			error_code const& err){
				if(!err){
					initialized(connection);
				}else{
					logsys::log([&err](logsys::stdlogb& os){
						os << "Error: WebSocket ready_callback: "
							<< err.message();
					});
				}
			});
	}

	void service::send_utf8(std::string const& message){
		send_frame_data(build_data_frame_data(
			websocket::frame::text_frame, message));
	}

	void service::send_utf8(
		std::string const& message,
		http::server::connection_ptr connection
	){
		send_frame_data(build_data_frame_data(
			websocket::frame::text_frame, message), connection);
	}

	void service::send_binary(std::string const& data){
		send_frame_data(build_data_frame_data(
			websocket::frame::binary_frame, data));
	}

	void service::send_binary(
		std::string const& data,
		http::server::connection_ptr connection
	){
		send_frame_data(build_data_frame_data(
			websocket::frame::binary_frame, data), connection);
	}

	void service::shutdown(std::uint16_t status, std::string const& reason){
		shutdown_ = true;
		close(status, reason);
	}

	void service::initialized(http::server::connection_ptr const& connection){
		if(shutdown_) return;

		{
			std::lock_guard< std::mutex > lock(mutex_);
			logsys::log([&connection](logsys::stdlogb& os){
				os << "Add connection " << connection.get();
			});
			connections_.insert(std::make_pair(connection,
				std::make_shared< connection_info >()));
		}

		if(new_connection_callback_) new_connection_callback_(connection);

		receive(connection, "");
	}

	void service::receive(
		http::server::connection_ptr const& connection,
		std::string const& data
	){
		boost::tribool result;
		auto info = [this, &connection]{
			std::lock_guard< std::mutex > lock(mutex_);

			auto connection_iter = connections_.find(connection);
			if (connection_iter == connections_.end()){
				return std::shared_ptr< connection_info >();
			}

			return connection_iter->second;
		}();

		if (!info) return;

		auto& parser = info->parser;
		auto& continuation_frames = info->continuation_frames;

		std::string::const_iterator iter = data.begin();
		do{
			websocket::frame frame;
			std::tie(result, iter) = parser.parse(frame, iter, data.end());

			// if result equals true, a complete frame was recieved
			if(result){
				if(!handle_frame(continuation_frames, frame, connection)){
					return;
				}
			}
		}while(result && iter != data.end());

		if(!result){
			// There was an error while parsing
			close(1002, "Read-Error: Parsing-Error in '" + data + "'",
				connection);
		}else{
			// wait for more messages
			connection->read([this](
				http::server::connection_ptr connection,
				std::string const& data,
				error_code const& err
			){
				if(!err){
					receive(connection, data);
				}else{
					close(1002, "Read-Error: " + err.message(), connection);
				}
			});
		}
	}

	bool service::handle_frame(
		std::vector< frame >& continuation_frames,
		frame const& frame,
		http::server::connection_ptr const& connection
	){
		// if opcode is a control frame
		if(frame.code >= 0x8 && !frame.fin){
			close(1002, "Read-Error: Control-frames must not be fragmentet.",
				connection);
			return false;
		}

		switch(frame.code){
			// data frames
			case frame::continuation_frame:{
				if(continuation_frames.size() == 0){
					close(1002, "Read-Error: Continuation-frame without "
						"initial-frame received.", connection);
					return false;
				}

				continuation_frames.push_back(frame);
				break;
			}
			case frame::text_frame:
			case frame::binary_frame:{
				if(continuation_frames.size() > 0){
					close(1002, "Read-Error: Data-frame while waiting for a "
						"continuation-frame.", connection);
					return false;
				}

				continuation_frames.push_back(frame);
				break;
			}

			// control frames
			case frame::connection_close:{
				if(frame.data.size() < 2) return false;
				std::uint16_t status_code =
					(static_cast< uint8_t >(frame.data[0]) << 8) |
					static_cast< uint8_t >(frame.data[1]);
				std::string message = frame.data.substr(2);
				close(1000, "Client request: " + message +
					" (Code: " + std::to_string(status_code) + ")", connection);
				return false;
			}
			case frame::ping:{
				// say 'i am here'
				pong(frame.data, connection);
				return true;
			}
			case frame::pong:{
				return true;
			}
			default:{
				close(1003, "Read-Error: Unknown opcode '"
					+ std::to_string(frame.code) + "' received.", connection);
				return false;
			}
		}

		// Wait for more fragments
		if(!frame.fin) return true;

		assert(continuation_frames.size() > 0);

		std::string message;
		for(auto const& frame: continuation_frames){
			message += frame.data;
		}

		switch(continuation_frames[0].code){
			case frame::text_frame:{
				if(utf8_callback_) utf8_callback_(message, connection);
				break;
			}
			case frame::binary_frame:{
				if(binary_callback_) binary_callback_(message, connection);
				break;
			}
			default:{
				close(1003, "Read-Error: Data-frame Opcode must be text_frame "
					"or binary_frame.", connection);
				return false;
			}
		}

		continuation_frames.clear();

		return true;
	}

	void service::close(std::uint16_t status, std::string const& reason){
		logsys::log([&reason](logsys::stdlogb& os){
			os << "Close all connections: " << reason;
		});
		std::shared_ptr< std::string const > data =
			build_close_frame_data(
				websocket::frame::connection_close, status, reason);
		send_close_frame_data(data);
	}

	void service::close(
		std::uint16_t status,
		std::string const& reason,
		http::server::connection_ptr const& connection
	){
		logsys::log([&connection, &reason](logsys::stdlogb& os){
			os << "Close connection " << connection.get() << ": " << reason;
		});
		std::shared_ptr< std::string const > data =
			build_close_frame_data(
				websocket::frame::connection_close, status, reason);
		send_close_frame_data(data, connection);
	}

	void service::pong(
		std::string const& message,
		http::server::connection_ptr const& connection
	){
		std::shared_ptr< std::string const > data =
			build_control_frame_data(
				websocket::frame::connection_close, message);
		send_frame_data(data, connection);
	}

	void service::send_frame_data(
		std::shared_ptr< std::string const > const& data
	){
		for(auto connection: get_connections()){
			send_frame_data(data, connection);
		}
	}

	void service::send_frame_data(
		std::shared_ptr< std::string const > const& data,
		http::server::connection_ptr const& connection
	){
		if(auto err = connection->write(data)){
			logsys::log([&err](logsys::stdlogb& os){
				os << "Error: WebSocket service write: " << err.message();
			});
			remove_connection(connection);
		}
	}

	void service::send_close_frame_data(
		std::shared_ptr< std::string const > const& data
	){
		for(auto connection: get_connections()){
			send_close_frame_data(data, connection);
		}
	}

	void service::send_close_frame_data(
		std::shared_ptr< std::string const > const& data,
		http::server::connection_ptr const& connection
	){
		connection->write(data);
		remove_connection(connection);
	}

	void service::remove_connection(
		http::server::connection_ptr const& connection
	){
		if(connection_close_callback_) connection_close_callback_(connection);

		std::lock_guard< std::mutex > lock(mutex_);
		connections_.erase(connection);
	}

	std::vector< http::server::connection_ptr > service::get_connections(){
		std::vector< http::server::connection_ptr > keys;
		keys.reserve(connections_.size());

		std::lock_guard< std::mutex > lock(mutex_);
		for(auto connection: connections_){
			keys.push_back(connection.first);
		}

		return keys;
	}


}
