//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__header__hpp_INCLUDED_
#define _http__header__hpp_INCLUDED_

#include <string>
#include <map>


namespace http{


	/// \brief HTTP-Header
	using header = std::multimap< std::string, std::string >;


}


#endif
