//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__websocket_server_json_service__hpp_INCLUDED_
#define _http__websocket_server_json_service__hpp_INCLUDED_

#include "websocket_server_service.hpp"

#define NOMINMAX 1
#define BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
#include <boost/property_tree/json_parser.hpp>
#undef interface


namespace http::websocket::server{


	using boost::property_tree::ptree;


	using json_callback_fn = std::function<
		void(ptree const&, http::server::connection_ptr const&) >;


	/// \brief Sending and receiving JSON-encoded data instead of plain text
	class json_service: public service{
	public:
		/// \brief Construct the service
		json_service(
			json_callback_fn const& json_callback = json_callback_fn(),
			data_callback_fn const& binary_callback = data_callback_fn(),
			info_callback_fn const& new_connection_callback =
				info_callback_fn(),
			info_callback_fn const& connection_close_callback =
				info_callback_fn()
		);


		/// \brief Send a property_tree-object as UTF-8 encoded text message in
		///        json-form to all Clients
		void send_json(ptree const& data);

		/// \brief Send a property_tree-object as UTF-8 encoded text message in
		///        json-form to a Client
		void send_json(
			ptree const& data,
			http::server::connection_ptr const& connection
		);


	private:
		using service::send_utf8;
	};


}


#endif
