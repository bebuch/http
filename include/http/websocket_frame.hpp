//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__websocket_frame__hpp_INCLUDED_
#define _http__websocket_frame__hpp_INCLUDED_

#include <string>


namespace http::websocket{

	/// \brief A WebSocket frame
	struct frame{
		/// \brief final frame?
		bool fin;

		/// \brief Which kind of frame is it?
		enum opcode{
			continuation_frame = 0x0,
			text_frame = 0x1,
			binary_frame = 0x2,
			connection_close = 0x8,
			ping = 0x9,
			pong = 0xA
		} code;

		/// \brief Data of the frame
		std::string data;
	};


}


#endif
