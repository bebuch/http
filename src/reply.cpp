//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
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


	namespace impl{ // Never use these functions direct


		std::pair< reply::status_type, std::string > make_mapping_pair(reply::status_type status, std::string const& text){
			return std::make_pair(status, "HTTP/1.1 " + std::to_string(status) + " " + text + "\r\n");
		}

		std::string const internal_server_error = "Internal Server Error";


	}


	std::map< reply::status_type, std::string > const type_to_string_mapper = {
		// Informational
		impl::make_mapping_pair(reply::continue_,                       "Continue"),
		impl::make_mapping_pair(reply::switching_protocols,             "Switching Protocols"),
		// Successful
		impl::make_mapping_pair(reply::ok,                              "OK"),
		impl::make_mapping_pair(reply::created,                         "Created"),
		impl::make_mapping_pair(reply::accepted,                        "Accepted"),
		impl::make_mapping_pair(reply::non_authoritative_information,   "Non-Authoritative Information"),
		impl::make_mapping_pair(reply::no_content,                      "No Content"),
		impl::make_mapping_pair(reply::reset_content,                   "Reset Content"),
		impl::make_mapping_pair(reply::partial_content,                 "Partial Content"),
		// Redirection
		impl::make_mapping_pair(reply::multiple_choices,                "Multiple Choices"),
		impl::make_mapping_pair(reply::moved_permanently,               "Moved Permanently"),
		impl::make_mapping_pair(reply::found,                           "Found"),
		impl::make_mapping_pair(reply::see_other,                       "See Other"),
		impl::make_mapping_pair(reply::not_modified,                    "Not Modified"),
		impl::make_mapping_pair(reply::use_proxy,                       "Use Proxy"),
		impl::make_mapping_pair(reply::temporary_redirect,              "Temporary Redirect"),
		// Client Error
		impl::make_mapping_pair(reply::bad_request,                     "Bad Request"),
		impl::make_mapping_pair(reply::unauthorized,                    "Unauthorized"),
		impl::make_mapping_pair(reply::payment_required,                "Payment Required"),
		impl::make_mapping_pair(reply::forbidden,                       "Forbidden"),
		impl::make_mapping_pair(reply::not_found,                       "Not Found"),
		impl::make_mapping_pair(reply::method_not_allowed,              "Method Not Allowed"),
		impl::make_mapping_pair(reply::not_acceptable,                  "Not Acceptable"),
		impl::make_mapping_pair(reply::proxy_authentication_required,   "Proxy Authentication Required"),
		impl::make_mapping_pair(reply::request_time_out,                "Request Time-out"),
		impl::make_mapping_pair(reply::conflict,                        "Conflict"),
		impl::make_mapping_pair(reply::gone,                            "Gone"),
		impl::make_mapping_pair(reply::length_required,                 "Length Required"),
		impl::make_mapping_pair(reply::precondition_failed,             "Precondition Failed"),
		impl::make_mapping_pair(reply::request_entity_too_large,        "Request Entity Too Large"),
		impl::make_mapping_pair(reply::request_url_too_long,            "Request-URL Too Long"),
		impl::make_mapping_pair(reply::unsupported_media_type,          "Unsupported Media Type"),
		impl::make_mapping_pair(reply::requested_range_not_satisfiable, "Requested range not satisfiable"),
		impl::make_mapping_pair(reply::expectation_failed,              "Expectation Failed"),
		// Server Error
		impl::make_mapping_pair(reply::internal_server_error,           impl::internal_server_error),
		impl::make_mapping_pair(reply::not_implemented,                 "Not Implemented"),
		impl::make_mapping_pair(reply::bad_gateway,                     "Bad Gateway"),
		impl::make_mapping_pair(reply::service_unavailable,             "Service Unavailable"),
		impl::make_mapping_pair(reply::gateway_time_out,                "Gateway Time-out"),
		impl::make_mapping_pair(reply::http_version_not_supported,      "HTTP Version not supported")
	};


	namespace status_strings{


		namespace impl{ // Never use these functions direct


			std::string const internal_server_error_string = "HTTP/1.1 500 Internal Server Error\r\n";


		}


		boost::asio::const_buffer to_buffer(reply::status_type status){
			auto iter = type_to_string_mapper.find(status);
			if(iter != type_to_string_mapper.end()){
				return boost::asio::buffer(iter->second);
			}

			return boost::asio::buffer(impl::internal_server_error_string);
		}


	}


	namespace misc_strings{


		std::string const name_value_separator = ": ";
		std::string const crlf = "\r\n";


	}


	std::vector< boost::asio::const_buffer > reply::to_buffers() const{
		std::vector< boost::asio::const_buffer > buffers;
		buffers.emplace_back(status_strings::to_buffer(status));
		for(auto const& header: headers){
			buffers.emplace_back(boost::asio::buffer(header.first));
			buffers.emplace_back(boost::asio::buffer(misc_strings::name_value_separator));
			buffers.emplace_back(boost::asio::buffer(header.second));
			buffers.emplace_back(boost::asio::buffer(misc_strings::crlf));
		}
		buffers.emplace_back(boost::asio::buffer(misc_strings::crlf));
		buffers.emplace_back(boost::asio::buffer(content));
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
				text = impl::internal_server_error;
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
			rep.headers.insert(make_pair("Content-Length", std::to_string(rep.content.size())));
			rep.headers.insert(make_pair("Content-Type", "text/html"));
		}
		return rep;
	}


}
