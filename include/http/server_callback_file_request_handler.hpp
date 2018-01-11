//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_virtual_file_request_handler__hpp_INCLUDED_
#define _http__server_virtual_file_request_handler__hpp_INCLUDED_

#include "server_basic_file_request_handler.hpp"

#include <map>
#include <tuple>
#include <functional>


namespace http::server{


	/// \brief Handles callback-file-request
	class callback_file_request_handler: public basic_file_request_handler {
	public:
		/// \brief Construct with a subdirectory containing virtual files to be
		///        served.
		callback_file_request_handler(std::string const& dir = "");

		/// \brief Handle a request and produce a reply.
		virtual bool handle_request(
			connection_ptr const& connection,
			http::request const& req,
			http::reply& rep
		) override;

		/// \brief Add a new virtual file
		bool add(
			std::string const& filename,
			std::string const& mime_type,
			std::function< std::string(http::request const& req) > const&
				callback
		);

		/// \brief erase a file
		bool erase(std::string const& filename);

		/// \brief erase all files
		void clear();

	private:
		/// \brief Sub directory for virtual files
		std::string const dir_;

		/// \brief map< filename, tuple< mime_type, content > >
		std::map< std::string, std::tuple<
				std::string,
				std::function< std::string(http::request const& req) >
			> > files_;
	};


}


#endif
