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


	class json_service;

	using json_service_ptr = std::shared_ptr< json_service >;
	using weak_json_service_ptr = std::weak_ptr< json_service >;

	/// \brief Sending and receiving JSON-encoded data instead of plain text
	class json_service: public service{
	public:
		using json_callback_function = std::function< void(boost::property_tree::ptree const&, http::server::connection_ptr const&) >;


		/// \brief Send a property_tree-object as UTF-8 encoded text message in json-form to all Clients
		void send_json(boost::property_tree::ptree const& data);

		/// \brief Send a property_tree-object as UTF-8 encoded text message in json-form to a Client
		void send_json(boost::property_tree::ptree const& data, http::server::connection_ptr const& connection);


	protected:
		///\brief  Construct the service
		template < typename F1, typename F2, typename F3, typename F4 >
		json_service(F1&& json_callback, F2&& binary_callback, F3&& new_connection_callback, F4&& connection_close_callback) :
			service(
				json_callback_type(std::forward< F1 >(json_callback)),
				std::forward< F2 >(binary_callback),
				std::forward< F3 >(new_connection_callback),
				std::forward< F4 >(connection_close_callback)
			){}

		///\brief  Construct the service
		template < typename F1, typename F2, typename F3 >
		json_service(F1&& json_callback, F2&& binary_callback, F3&& new_connection_callback) :
			service(
				json_callback_type(std::forward< F1 >(json_callback)),
				std::forward< F2 >(binary_callback),
				std::forward< F3 >(new_connection_callback)
			){}

		///\brief  Construct the service
		template < typename F1, typename F2 >
		json_service(F1&& json_callback, F2&& binary_callback) :
			service(
				json_callback_type(std::forward< F1 >(json_callback)),
				std::forward< F2 >(binary_callback)
			){}

		///\brief  Construct the service
		template < typename F1 >
		json_service(F1&& json_callback) :
			service(
				json_callback_type(std::forward< F1 >(json_callback))
			){}

		///\brief  Construct the service
		json_service(){}


	private:
		class json_callback_type{
		public:
			json_callback_type(json_callback_function const& f) : json_callback_(f) {}
			json_callback_type(json_callback_function&& f) : json_callback_(std::move(f)) {}

			void operator()(std::string const& message, http::server::connection_ptr const& connection)const;

		private:
			json_callback_function json_callback_;
		};

		using service::send_utf8;


		template < typename ... Params >
		friend service_ptr make_shared_json_service(Params&& ... params);
	};


}


#endif
