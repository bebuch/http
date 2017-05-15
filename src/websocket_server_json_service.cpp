//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/websocket_server_json_service.hpp>

#include <sstream>

namespace http::websocket::server{


	json_service::json_service(
		json_callback_fn const& json_callback,
		data_callback_fn const& binary_callback,
		info_callback_fn const& new_connection_callback,
		info_callback_fn const& connection_close_callback
	):
		service(
			[json_callback](
				std::string const& message,
				http::server::connection_ptr const& connection
			){
				if(!json_callback) return;

				ptree data;
				std::istringstream is(message);
				read_json(is, data);
				json_callback(data, connection);
			},
			binary_callback,
			new_connection_callback,
			connection_close_callback
		) {}


	std::string to_json(ptree const& data){
		std::ostringstream message;
		write_json(message, data);
		return message.str();
	}

	void json_service::send_json(ptree const& data){
		send_utf8(to_json(data));
	}

	void json_service::send_json(
		ptree const& data,
		http::server::connection_ptr const& connection
	){
		send_utf8(to_json(data), connection);
	}


}
