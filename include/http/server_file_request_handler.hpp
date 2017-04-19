//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_file_request_handler__hpp_INCLUDED_
#define _http__server_file_request_handler__hpp_INCLUDED_

#include "server_basic_file_request_handler.hpp"


namespace http::server{


	/// \brief Handles file-request
	class file_request_handler: public basic_file_request_handler{
	public:
		/// \brief Construct with a directory containing files to be served.
		explicit file_request_handler(std::string const& doc_root);

		/// \brief Handle a request and produce a reply.
		virtual bool handle_request(connection_ptr const& connection, http::request const& req, http::reply& rep) override;

	protected:
		/// \brief Open the file to send back and fill out the reply to be sent to the client.
		bool read_file(http::reply& rep, std::string const& filename)const;

		/// \brief The directory containing the files to be served.
		std::string const doc_root_;
	};


}


#endif
