//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/server_basic_file_request_handler.hpp>

#include <http/reply.hpp>
#include <http/request.hpp>


namespace http::server{


	bool basic_file_request_handler::check_uri(
		http::request const& req,
		http::reply& rep
	)const{
		if(req.uri.empty() || req.uri[0] != '/' ||
			req.uri.find("/..") != std::string::npos
		){
			rep = reply::stock_reply(reply::bad_request);
			return false;
		}

		return true;
	}

	std::string basic_file_request_handler::get_file_extension(
		std::string const& filename
	)const{
		std::size_t last_slash_pos = filename.find_last_of("/");
		std::size_t last_dot_pos = filename.find_last_of(".");

		std::string extension;
		if(last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos){
			return filename.substr(last_dot_pos + 1);
		}

		return "";
	}

	void basic_file_request_handler::set_http_header(
		http::reply& rep,
		std::string const& mime_type
	)const{
		rep.headers.clear();
		rep.headers.insert(make_pair("Content-Length",
			std::to_string(rep.content.size())));
		rep.headers.insert(make_pair("Content-Type", mime_type));
	}


}
