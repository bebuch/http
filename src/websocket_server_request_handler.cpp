//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/websocket_server_request_handler.hpp>

#include <http/websocket_server_service.hpp>
#include <http/mime_types.hpp>
#include <http/reply.hpp>
#include <http/request.hpp>

#include <boost/uuid/sha1.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>


namespace http::websocket::server{


	namespace impl{


		inline std::string base64_encoded(std::string const& text){
			using namespace boost::archive::iterators;
			using base64_text = insert_linebreaks<
				base64_from_binary< transform_width< char const*, 6, 8 > >,
				72 >;

			std::ostringstream os;
			std::copy(
				base64_text(text.c_str()),
				base64_text(text.c_str() + text.size()),
				std::ostream_iterator< char >(os)
			);
			return os.str();
		}

		std::string sha1_hash(std::string const& text){
			boost::uuids::detail::sha1 hasher;
			std::string hash(20, '\0');
			hasher.process_bytes(text.c_str(), text.size());
			unsigned int digest[5];
			hasher.get_digest(digest);
			for(int i = 0; i < 5; ++i){
				char const* tmp = reinterpret_cast< char* >(digest);
				hash[i * 4]     = tmp[i * 4 + 3];
				hash[i * 4 + 1] = tmp[i * 4 + 2];
				hash[i * 4 + 2] = tmp[i * 4 + 1];
				hash[i * 4 + 3] = tmp[i * 4];
			}
			return hash;
		}


	}


	bool request_handler::handle_request(
		http::server::connection_ptr const& connection_ptr,
		http::request const& req, http::reply& rep
	){
		// Request path must be absolute.
		if(req.uri.empty() || req.uri[0] != '/'){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}

		// Check for the required header field "Connection"
		auto connection_header = req.headers.find("Connection");
		if(connection_header == req.headers.end()){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}
		std::vector< std::string > tokens;
		boost::algorithm::split(tokens, connection_header->second,
			boost::algorithm::is_any_of(","));
		for(std::string& token: tokens) boost::algorithm::trim(token);
		if(find(tokens.begin(), tokens.end(), "Upgrade") == tokens.end()){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}

		// Check for the required header field "Upgrade"
		auto upgrade_header = req.headers.find("Upgrade");
		if(upgrade_header == req.headers.end()
			|| upgrade_header->second != "websocket"
		){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}

		// Check for correct protocol version.
		auto version = req.headers.find("Sec-WebSocket-Version");
		if(version == req.headers.end() || version->second != "13"){
			rep = http::reply::stock_reply(http::reply::bad_request);
			rep.headers.insert(std::make_pair("Sec-WebSocket-Version", "13"));
			return false;
		}

		// Check if a key is available.
		auto key = req.headers.find("Sec-WebSocket-Key");
		if(key == req.headers.end()){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}

		// Erase the beginning '/' from resource name
		std::string name = req.uri;
		name.erase(0, 1);

		// Cut parameters
		// TODO: Get parameter to the service
		auto parameter = name.find("?");
		if(parameter != std::string::npos){
			name = name.substr(0, parameter);
		}

		// Check if there is a websocket service with this name
		auto ws_service = services_.find(name);
		if(ws_service == services_.end()){
			rep = http::reply::stock_reply(http::reply::not_found);
			return false;
		}

		// Add the connection to websocket service
		ws_service->second.get().add_connection(connection_ptr);

		// Accept the protocol switching.
		rep = http::reply::stock_reply(http::reply::switching_protocols);
		rep.headers.insert(std::make_pair("Connection", "Upgrade"));
		rep.headers.insert(*upgrade_header);

		// Create and add the respond key.
		std::string magic_key = key->second
			+ "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		rep.headers.insert(make_pair("Sec-WebSocket-Accept",
			impl::base64_encoded(impl::sha1_hash(magic_key)) + "="));

		return true;
	}

	void request_handler::shutdown(){
		for(auto& service: services_){
			service.second.get().shutdown(1001, "Server shutdown");
		}
	}

	bool request_handler::register_service(
		std::string const& name,
		service& reg
	){
		return services_.emplace(name, std::ref(reg)).second;
	}

	bool request_handler::shutdown_service(std::string const& name){
		auto iter = services_.find(name);

		if(iter == services_.end()) return false;

		iter->second.get().shutdown(1001, "Websocket shutdown");

		services_.erase(iter);
		return true;
	}



}
