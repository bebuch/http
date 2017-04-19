//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/mime_types.hpp>

#include <boost/algorithm/string.hpp>

#include <map>


namespace http::mime_types{



	std::map< std::string, std::string > const& extension_to_type_mapper(){
		static std::map< std::string, std::string > const result = config::mime_types::init_extension_to_type_mapper();
		return result;
	}

	std::string extension_to_type(std::string const& extension){
		auto iter = extension_to_type_mapper().find(boost::algorithm::to_lower_copy(extension));
		if(iter != extension_to_type_mapper().end()){
			return iter->second;
		}

		return "text/plain";
	}


}


std::map< std::string, std::string > http::config::mime_types::init_extension_to_type_mapper(){
	return std::map< std::string, std::string >{
		{"gif" , "image/gif"       },
		{"htm" , "text/html"       },
		{"html", "text/html"       },
		{"js"  , "text/javascript" },
		{"css" , "text/css"        },
		{"jpg" , "image/jpeg"      },
		{"svg" , "image/svg+xml"   },
		{"png" , "image/png"       },
		{"gz"  , "application/gzip"}
	};
}
