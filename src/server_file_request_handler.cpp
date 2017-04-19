//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/server_file_request_handler.hpp>

#include <http/mime_types.hpp>
#include <http/reply.hpp>
#include <http/request.hpp>

#include <fstream>


namespace http::server{


	file_request_handler::file_request_handler(std::string const& doc_root):
		doc_root_(doc_root)
		{}

	bool file_request_handler::handle_request(
		connection_ptr const&,
		http::request const& req,
		http::reply& rep
	){
		// Request path must be absolute and not contain "/..".
		if(!check_uri(req, rep)) return false;

		std::string file = req.uri;

		// If path ends in slash (i.e. is a directory) then add "index.html".
		if(file[file.size() - 1] == '/'){
			file += "index.html";
		}

		// Read file from hard disk and set it as content
		if(!read_file(rep, file)) return false;

		// Determine the file extension.
		std::string extension = get_file_extension(file);

		/// Set content length and mime type
		set_http_header(rep, mime_types::extension_to_type(extension));

		return true;
	}

	bool file_request_handler::read_file(
		http::reply& rep,
		std::string const& filename
	)const{
		// Open the file to send back.
		std::string full_path = doc_root_ + filename;
		std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
		if(!is){
			rep = reply::stock_reply(reply::not_found);
			return false;
		}

		// Fill out the reply to be sent to the client.
		rep.status = reply::ok;
		rep.content = "";
		char buf[512];
		while(is.read(buf, sizeof(buf)).gcount() > 0){
			rep.content.append(buf, static_cast< unsigned >(is.gcount()));
		}

		return true;
	}


}
