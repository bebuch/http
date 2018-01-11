//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__reply__hpp_INCLUDED_
#define _http__reply__hpp_INCLUDED_

#include "header.hpp"

#include <boost/asio.hpp>

#include <string>
#include <vector>


namespace http{


	namespace asio = boost::asio;


	/// \brief A reply to be sent to a client.
	struct reply{
		/// \brief The status of the reply.
		enum status_type{
			// Informational
			continue_ = 100,
			switching_protocols = 101,
			// Successful
			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,
			// Redirection
			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,
			// Client Error
			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_time_out = 408,
			conflict = 409,
			gone = 410,
			length_required = 411,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_url_too_long = 414,
			unsupported_media_type = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed = 417,
			// Server Error
			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_time_out = 504,
			http_version_not_supported = 505
		} status;

		/// \brief The headers to be included in the reply.
		header headers;

		/// \brief The content to be sent in the reply.
		std::string content;

		/// \brief Convert the reply into a vector of buffers.
		///
		/// The buffers do not own the underlying memory blocks,
		/// therefore the reply object must remain valid and
		/// not be changed until the write operation has completed.
		std::vector< asio::const_buffer > to_buffers() const;

		/// \brief Get a stock reply.
		static reply stock_reply(status_type status);
	};


}


#endif
