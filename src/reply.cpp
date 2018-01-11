//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/reply.hpp>

#include <string>
#include <map>


namespace http{


	namespace{ // Never use these functions direct


		std::pair< reply::status_type, std::string > make_mapping_pair(
			reply::status_type status, std::string const& text
		){
			return std::make_pair(
				status,
				"HTTP/1.1 " + std::to_string(status) + " " + text + "\r\n");
		}

		std::string const internal_server_error = "Internal Server Error";


	}


	std::map< reply::status_type, std::string > const type_to_string_mapper = {
		// Informational
		make_mapping_pair(reply::continue_,
			"Continue"),
		make_mapping_pair(reply::switching_protocols,
			"Switching Protocols"),
		// Successful
		make_mapping_pair(reply::ok,
			"OK"),
		make_mapping_pair(reply::created,
			"Created"),
		make_mapping_pair(reply::accepted,
			"Accepted"),
		make_mapping_pair(reply::non_authoritative_information,
			"Non-Authoritative Information"),
		make_mapping_pair(reply::no_content,
			"No Content"),
		make_mapping_pair(reply::reset_content,
			"Reset Content"),
		make_mapping_pair(reply::partial_content,
			"Partial Content"),
		// Redirection
		make_mapping_pair(reply::multiple_choices,
			"Multiple Choices"),
		make_mapping_pair(reply::moved_permanently,
			"Moved Permanently"),
		make_mapping_pair(reply::found,
			"Found"),
		make_mapping_pair(reply::see_other,
			"See Other"),
		make_mapping_pair(reply::not_modified,
			"Not Modified"),
		make_mapping_pair(reply::use_proxy,
			"Use Proxy"),
		make_mapping_pair(reply::temporary_redirect,
			"Temporary Redirect"),
		// Client Error
		make_mapping_pair(reply::bad_request,
			"Bad Request"),
		make_mapping_pair(reply::unauthorized,
			"Unauthorized"),
		make_mapping_pair(reply::payment_required,
			"Payment Required"),
		make_mapping_pair(reply::forbidden,
			"Forbidden"),
		make_mapping_pair(reply::not_found,
			"Not Found"),
		make_mapping_pair(reply::method_not_allowed,
			"Method Not Allowed"),
		make_mapping_pair(reply::not_acceptable,
			"Not Acceptable"),
		make_mapping_pair(reply::proxy_authentication_required,
			"Proxy Authentication Required"),
		make_mapping_pair(reply::request_time_out,
			"Request Time-out"),
		make_mapping_pair(reply::conflict,
			"Conflict"),
		make_mapping_pair(reply::gone,
			"Gone"),
		make_mapping_pair(reply::length_required,
			"Length Required"),
		make_mapping_pair(reply::precondition_failed,
			"Precondition Failed"),
		make_mapping_pair(reply::request_entity_too_large,
			"Request Entity Too Large"),
		make_mapping_pair(reply::request_url_too_long,
			"Request-URL Too Long"),
		make_mapping_pair(reply::unsupported_media_type,
			"Unsupported Media Type"),
		make_mapping_pair(reply::requested_range_not_satisfiable,
			"Requested range not satisfiable"),
		make_mapping_pair(reply::expectation_failed,
			"Expectation Failed"),
		// Server Error
		make_mapping_pair(reply::internal_server_error,
			internal_server_error),
		make_mapping_pair(reply::not_implemented,
			"Not Implemented"),
		make_mapping_pair(reply::bad_gateway,
			"Bad Gateway"),
		make_mapping_pair(reply::service_unavailable,
			"Service Unavailable"),
		make_mapping_pair(reply::gateway_time_out,
			"Gateway Time-out"),
		make_mapping_pair(reply::http_version_not_supported,
			"HTTP Version not supported")
	};


	namespace status_strings{


		namespace{ // Never use these functions direct


			std::string const internal_server_error_string =
				"HTTP/1.1 500 Internal Server Error\r\n";


		}


		asio::const_buffer to_buffer(reply::status_type status){
			auto iter = type_to_string_mapper.find(status);
			if(iter != type_to_string_mapper.end()){
				return asio::buffer(iter->second);
			}

			return asio::buffer(internal_server_error_string);
		}


	}


	namespace misc_strings{


		std::string const name_value_separator = ": ";
		std::string const crlf = "\r\n";


	}


	std::vector< asio::const_buffer > reply::to_buffers() const{
		std::vector< asio::const_buffer > buffers;
		buffers.emplace_back(status_strings::to_buffer(status));
		for(auto const& header: headers){
			buffers.emplace_back(asio::buffer(header.first));
			buffers.emplace_back(asio::buffer(
				misc_strings::name_value_separator));
			buffers.emplace_back(asio::buffer(header.second));
			buffers.emplace_back(asio::buffer(misc_strings::crlf));
		}
		buffers.emplace_back(asio::buffer(misc_strings::crlf));
		buffers.emplace_back(asio::buffer(content));
		return buffers;
	}


	namespace stock_replies{


		std::string to_string(reply::status_type status){
			switch(status){
				case reply::continue_:
				case reply::switching_protocols:
				case reply::ok:
					return "";
				default:;
			}

			auto iter = type_to_string_mapper.find(status);
			std::string text;
			if(iter == type_to_string_mapper.end()){
				status = reply::internal_server_error;
				text = internal_server_error;
			}else{
				text = iter->second;
			}

			return
				"<!DOCTYPE html>"
				"<html>"
				"<head><title>" + text + "</title></head>"
				"<body><h1>" + text + "</h1></body>"
				"</html>";
		}


	}


	reply reply::stock_reply(reply::status_type status){
		using std::make_pair;

		reply rep;
		rep.status = status;
		rep.content = stock_replies::to_string(status);
		rep.headers.clear();
		if(rep.content.size() > 0){
			rep.headers.insert(make_pair("Content-Length",
				std::to_string(rep.content.size())));
			rep.headers.insert(make_pair("Content-Type", "text/html"));
		}
		return rep;
	}


}
