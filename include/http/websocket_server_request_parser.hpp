//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__websocket_server_request_parser__hpp_INCLUDED_
#define _http__websocket_server_request_parser__hpp_INCLUDED_

#include "websocket_frame.hpp"

#include <boost/logic/tribool.hpp>

#include <tuple>
#include <cstdint>


namespace http::websocket::server{


	struct request;

	/// \brief Parser for incoming requests.
	class request_parser{
	public:
		/// \brief Construct ready to parse the request method.
		request_parser();

		/// \brief Reset to initial parser state.
		void reset();

		/// \brief Parse some data.
		///
		/// The tribool return value is true when a complete request has been
		/// parsed, false if the data is invalid, indeterminate when more data
		/// is required. The InputIterator return value indicates how much of
		/// the input has been consumed.
		template <typename InputIterator>
		std::tuple< boost::tribool, InputIterator >
		parse(websocket::frame& frame, InputIterator begin, InputIterator end){
			while(begin != end){
				boost::tribool result = consume(frame, *begin++);
				if(result || !result){
					return std::make_tuple(result, begin);
				}
			}
			boost::tribool result = boost::indeterminate;
			return std::make_tuple(result, begin);
		}

	private:
		///\brief  Handle the next character of input.
		boost::tribool consume(websocket::frame& frame, char input);

		/// \brief The current state of the parser.
		enum state{
			frame_start,
			length_1,
			length_2,
			length_8,
			masking_key,
			payload_data
		} state_;

		/// \brief true, if this is the final frame of the message
		bool fin_;

		/// \brief The type of the message
		websocket::frame::opcode opcode_;

		/// \brief Count of bytes of the data
		std::size_t length_;

		/// \brief The key to decode the payload data
		std::uint32_t masking_key_;

		/// \brief The recived message
		std::string payload_data_;

		/// \brief Which byte in the actual state
		std::size_t counter_;
	};


}


#endif
