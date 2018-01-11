//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_basic_file_request_handler__hpp_INCLUDED_
#define _http__server_basic_file_request_handler__hpp_INCLUDED_

#include "server_request_handler.hpp"


namespace http::server{


	/// \brief Basic helper methods for file-handling
	class basic_file_request_handler: public request_handler{
	public:
		/// \brief Handle a request and produce a reply.
		virtual bool handle_request(
			connection_ptr const& connection,
			http::request const& req,
			http::reply& rep
		) = 0;

	protected:
		/// \brief Request path must be absolute and not contain "/..".
		bool check_uri(http::request const& req, http::reply& rep)const;

		/// \brief Determine the file extension.
		std::string get_file_extension(std::string const& filename)const;

		/// \brief Set content length and mime type
		void set_http_header(
			http::reply& rep,
			std::string const& mime_type
		)const;
	};


}


#endif
