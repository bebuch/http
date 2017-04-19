//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/server_request_parser.hpp>

#include <http/request.hpp>

#include <boost/algorithm/string.hpp>

#include <sstream>


namespace http::server{


	request_parser::request_parser():
		state_(method_start)
		{}

	void request_parser::reset(){
		state_ = method_start;
	}

	boost::tribool request_parser::consume(http::request& req, char input){
		switch(state_){
		case method_start:
			if(!is_char(input) || is_ctl(input) || is_tspecial(input)){
				return false;
			}else{
				state_ = method;
				req.method.push_back(input);
				return boost::indeterminate;
			}
		case method:
			if(input == ' '){
				state_ = uri;
				return boost::indeterminate;
			}else if (!is_char(input) || is_ctl(input) || is_tspecial(input)){
				return false;
			}else{
				req.method.push_back(input);
				return boost::indeterminate;
			}
		case uri:
			if(input == ' '){
				if(!url_decode(req.uri, req.uri)) return false;
				state_ = http_version_h;
				return boost::indeterminate;
			}else if(is_ctl(input)){
				return false;
			}else{
				req.uri.push_back(input);
				return boost::indeterminate;
			}
		case http_version_h:
			if(input == 'H'){
				state_ = http_version_t_1;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_t_1:
			if(input == 'T'){
				state_ = http_version_t_2;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_t_2:
			if(input == 'T'){
				state_ = http_version_p;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_p:
			if (input == 'P'){
				state_ = http_version_slash;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_slash:
			if(input == '/'){
				req.http_version_major = 0;
				req.http_version_minor = 0;
				state_ = http_version_major_start;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_major_start:
			if(is_digit(input)){
				req.http_version_major = req.http_version_major * 10 + input
					- '0';
				state_ = http_version_major;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_major:
			if(input == '.'){
				state_ = http_version_minor_start;
				return boost::indeterminate;
			}else if(is_digit(input)){
				req.http_version_major = req.http_version_major * 10 + input
					- '0';
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_minor_start:
			if(is_digit(input)){
				req.http_version_minor = req.http_version_minor * 10 + input
					- '0';
				state_ = http_version_minor;
				return boost::indeterminate;
			}else{
				return false;
			}
		case http_version_minor:
			if(input == '\r'){
				state_ = expecting_newline_1;
				return boost::indeterminate;
			}else if(is_digit(input)){
				req.http_version_minor = req.http_version_minor * 10 + input
					- '0';
				return boost::indeterminate;
			}else{
				return false;
			}
		case expecting_newline_1:
			if(input == '\n'){
				state_ = header_line_start;
				return boost::indeterminate;
			}else{
				return false;
			}
		case header_line_start:
			if(input == '\r'){
				state_ = expecting_newline_3;
				return boost::indeterminate;
			}else if(!req.headers.empty() && (input == ' ' || input == '\t')){
				state_ = header_lws;
				return boost::indeterminate;
			}else if(!is_char(input) || is_ctl(input) || is_tspecial(input)){
				return false;
			}else{
				state_ = header_name;
				name_ = input;
				value_ = "";
				return boost::indeterminate;
			}
		case header_lws:
			if(input == '\r'){
				state_ = expecting_newline_2;
				return boost::indeterminate;
			}else if(input == ' ' || input == '\t'){
				return boost::indeterminate;
			}else if(is_ctl(input)){
				return false;
			}else{
				state_ = header_value;
				value_ += input;
				return boost::indeterminate;
			}
		case header_name:
			if(input == ':'){
				state_ = space_before_header_value;
				return boost::indeterminate;
			}else if(!is_char(input) || is_ctl(input) || is_tspecial(input)){
				return false;
			}else{
				name_ += input;
				return boost::indeterminate;
			}
		case space_before_header_value:
			if(input == ' '){
				state_ = header_value;
				return boost::indeterminate;
			}else{
				return false;
			}
		case header_value:
			if(input == '\r'){
				state_ = expecting_newline_2;
				return boost::indeterminate;
			}else if(is_ctl(input)){
				return false;
			}else{
				value_ += input;
				return boost::indeterminate;
			}
		case expecting_newline_2:
			if(input == '\n'){
				boost::algorithm::trim(name_);
				boost::algorithm::trim(value_);
				req.headers.insert(make_pair(name_, value_));
				state_ = header_line_start;
				return boost::indeterminate;
			}else{
				return false;
			}
		case expecting_newline_3:
			return (input == '\n');
		default:
			return false;
		}
	}

	bool request_parser::is_char(int c){
		return c >= 0 && c <= 127;
	}

	bool request_parser::is_ctl(int c){
		return (c >= 0 && c <= 31) || (c == 127);
	}

	bool request_parser::is_tspecial(int c){
		switch(c){
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
		}
	}

	bool request_parser::is_digit(int c){
		return c >= '0' && c <= '9';
	}

	bool request_parser::url_decode(std::string const& in, std::string& out){
		std::string result;
		result.reserve(in.size());
		for(std::size_t i = 0; i < in.size(); ++i){
			if(in[i] == '%'){
				if(i + 3 <= in.size()){
					int value = 0;
					std::istringstream is(in.substr(i + 1, 2));
					if(is >> std::hex >> value){
						result += static_cast< char >(value);
						i += 2;
					}else{
						return false;
					}
				}else{
					return false;
				}
			}else if(in[i] == '+'){
				result += ' ';
			}else{
				result += in[i];
			}
		}
		out = result;
		return true;
	}


}
