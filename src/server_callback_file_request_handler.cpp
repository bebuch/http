//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/server_callback_file_request_handler.hpp>

#include <http/mime_types.hpp>
#include <http/reply.hpp>
#include <http/request.hpp>


namespace http::server{


	callback_file_request_handler::callback_file_request_handler(
		std::string const& dir
	):
		dir_(dir)
		{}

	bool callback_file_request_handler::handle_request(
		connection_ptr const&,
		http::request const& req,
		http::reply& rep
	){
		// Request path must be absolute and not contain "/..".
		if(!check_uri(req, rep)) return false;

		// Check if requestet file in virtual subdirectory
		std::string filename = dir_.empty() ? req.uri : req.uri.substr(1);
		std::size_t dir_length = dir_.size() + 1;
		if(dir_length > filename.size() ||
			filename.compare(0, dir_length, dir_ + "/") != 0
		){
			rep = http::reply::stock_reply(http::reply::bad_request);
			return false;
		}

		// Erase subdirectory from filename
		filename = filename.substr(dir_length);

		// Check if file exists
		auto file = files_.find(filename);
		if(file == files_.end()){
			rep = http::reply::stock_reply(http::reply::not_found);
			return false;
		}

		/// Set content length and mime type
		rep.status = reply::ok;
		rep.content = std::get< 1 >(file->second)(req);
		set_http_header(rep,
			mime_types::extension_to_type(std::get< 0 >(file->second)));

		return true;
	}

	/// Add a new virtual file
	bool callback_file_request_handler::add(
		std::string const& filename,
		std::string const& mime_type,
		std::function< std::string(http::request const& req) > const& callback
	){
		if(files_.find(filename) != files_.end()) return false;

		files_.insert(make_pair(filename, make_pair(mime_type, callback)));
		return true;
	}

	/// erase a file
	bool callback_file_request_handler::erase(std::string const& filename){
		return files_.erase(filename) > 0;
	}

	/// erase all files
	void callback_file_request_handler::clear(){
		files_.clear();
	}


}
