//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_lambda_request_handler__hpp_INCLUDED_
#define _http__server_lambda_request_handler__hpp_INCLUDED_

#include "server_request_handler.hpp"

#include <functional>


namespace http::server{


	///\brief  A simple callback handler
	class lambda_request_handler: public request_handler{
	public:
		/// \brief Constructs a handler, that calls the given function
		lambda_request_handler(
			std::function< bool(connection_ptr const&, http::request const&, http::reply&) > handler,
			std::function< void() > shutdown = std::function< void() >()
		):
			handler_function_(handler),
			shutdown_function_(shutdown)
			{}

		/// \brief Handle a request and produce a reply.
		virtual bool handle_request(connection_ptr const& connection, http::request const& req, http::reply& rep) override{
			return handler_function_(connection, req, rep);
		}

		/// \brief Call the shutdown function
		virtual void shutdown() override{
			if(shutdown_function_) shutdown_function_();
		}

	private:
		/// \brief The functor who is called from the handler
		std::function< bool(connection_ptr const&, http::request const&, http::reply&) > handler_function_;

		/// \brief The functor who is called from the handler
		std::function< void() > shutdown_function_;
	};


}


#endif
