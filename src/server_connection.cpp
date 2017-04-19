//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/server_connection.hpp>

#include <http/server_request_handler.hpp>


namespace http::server{


	connection::connection(asio::io_service& io_service):
		strand_(io_service),
		socket_(io_service)
		{}

	connection::~connection(){
		// Initiate graceful connection closure.
		error_code ignored_error;
		socket_.shutdown(tcp::socket::shutdown_both, ignored_error);
	}

	tcp::socket& connection::socket(){
		return socket_;
	}

	void connection::start(request_handler& request_handler){
		auto shared_this = shared_from_this();

		/// The incoming request.
		auto request = std::make_shared< http::request >();

		/// The parser for the incoming request.
		auto request_parser =
			std::make_shared< http::server::request_parser >();

		/// The reply to be sent back to the client.
		auto reply = std::make_shared< http::reply >();

		socket_.async_read_some(
			asio::buffer(buffer_),
			strand_.wrap(
				[&request_handler, request, request_parser, reply, shared_this](
					error_code const& err, std::size_t bytes_transferred
				){
					shared_this->handle_first_read(
						request_handler, request, request_parser,
						reply, err, bytes_transferred);
				})
		);
	}

	void connection::handle_first_read(
		request_handler& request_handler,
		std::shared_ptr< http::request > const& request,
		std::shared_ptr< http::server::request_parser > const& request_parser,
		std::shared_ptr< http::reply > const& reply,
		error_code const& err,
		std::size_t bytes_transferred
	){
		if(!err){
			boost::tribool result;
			std::tie(result, std::ignore) = request_parser->parse(
				*request,
				buffer_.data(),
				buffer_.data() + bytes_transferred
			);

			auto shared_this = shared_from_this();
			if (result){
				// handle the request
				request_handler.handle_request(shared_this, *request, *reply);
				asio::async_write(
					socket_,
					reply->to_buffers(),
					strand_.wrap(
						[shared_this, reply](
							error_code const& err, std::size_t
						){
							shared_this->handle_first_write(err);
						})
				);
			}else if(!result){
				// request parsing failed
				*reply = reply::stock_reply(reply::bad_request);
				asio::async_write(
					socket_,
					reply->to_buffers(),
					strand_.wrap(
						[shared_this, reply](
							error_code const& err, std::size_t
						){
							shared_this->handle_first_write(err);
						})
				);
			}else{
				// wait for the rest
				socket_.async_read_some(
					asio::buffer(buffer_),
					strand_.wrap(
						[&request_handler, request, request_parser, reply,
							shared_this](
							error_code const& err, std::size_t bytes_transferred
						){
							shared_this->handle_first_read(
								request_handler, request, request_parser,
								reply, err, bytes_transferred);
						})
				);
			}
		}

		// If an error occurs then no new asynchronous operations are started.
		// This means that all shared_ptr references to the connection object
		// will disappear and the object will be destroyed automatically after
		// this handler returns. The connection class's destructor closes the
		// socket.
	}

	void connection::handle_first_write(error_code const& err){
		if (ready_callback_) ready_callback_(shared_from_this(), err);

		// No new asynchronous operations are started. This means that all
		// shared_ptr references to the connection object will disappear and
		// the object will be destroyed automatically after this handler
		// returns. The connection class's destructor closes the socket.
	}

	void connection::ready_callback(callback_write_fn callback){
		ready_callback_ = callback;
	}

	error_code connection::write(
		std::shared_ptr< std::string const > const& data
	){
		error_code err;
		asio::write(socket_, asio::buffer(*data), err);
		return err;
	}

	void connection::read(callback_read_fn callback){
		auto shared_this = shared_from_this();
		socket_.async_read_some(
			asio::buffer(buffer_),
			strand_.wrap(
				[shared_this, callback](
					error_code const& err, std::size_t bytes_transferred
				){
					std::string data(
						shared_this->buffer_.data(),
						shared_this->buffer_.data() + bytes_transferred);
					callback(shared_this, data, err);
				})
		);
	}


}
