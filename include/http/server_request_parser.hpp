//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_request_parser__hpp_INCLUDED_
#define _http__server_request_parser__hpp_INCLUDED_

#include "request.hpp"

#include <boost/logic/tribool.hpp>

#include <tuple>


namespace http::server{


	/// \brief Parser for incoming requests.
	class request_parser{
	public:
		/// \brief Construct ready to parse the request method.
		request_parser();

		/// \brief Reset to initial parser state.
		void reset();

		/// \brief Parse some data.
		///
		/// The tribool return value is true when a complete request
		/// has been parsed, false if the data is invalid, indeterminate when more
		/// data is required. The InputIterator return value indicates how much of the
		/// input has been consumed.
		template < typename InputIterator >
		std::tuple< boost::tribool, InputIterator > parse(http::request& req, InputIterator begin, InputIterator end){
			while(begin != end){
				boost::tribool result = consume(req, *begin++);
				if(result || !result){
					return std::make_tuple(result, begin);
				}
			}
			boost::tribool result = boost::indeterminate;
			return std::make_tuple(result, begin);
		}

	private:
		/// \brief Handle the next character of input.
		boost::tribool consume(http::request& req, char input);

		std::string name_;
		std::string value_;

		/// \brief Check if a byte is an HTTP character.
		static bool is_char(int c);

		/// \brief Check if a byte is an HTTP control character.
		static bool is_ctl(int c);

		/// \brief Check if a byte is defined as an HTTP tspecial character.
		static bool is_tspecial(int c);

		/// \brief Check if a byte is a digit.
		static bool is_digit(int c);

		/// \brief Perform URL-decoding on a string.
		///
		/// Returns false if the encoding was invalid.
		static bool url_decode(std::string const& in, std::string& out);

		/// \brief The current state of the parser.
		enum state{
			method_start,
			method,
			uri,
			http_version_h,
			http_version_t_1,
			http_version_t_2,
			http_version_p,
			http_version_slash,
			http_version_major_start,
			http_version_major,
			http_version_minor_start,
			http_version_minor,
			expecting_newline_1,
			header_line_start,
			header_lws,
			header_name,
			space_before_header_value,
			header_value,
			expecting_newline_2,
			expecting_newline_3
		} state_;
	};


}


#endif
