//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__websocket_server_service__hpp_INCLUDED_
#define _http__websocket_server_service__hpp_INCLUDED_

#include "server_connection.hpp"
#include "websocket_server_request_parser.hpp"

#include <map>
#include <mutex>


namespace http::websocket::server{


	using data_callback_fn = std::function<
		void(std::string const&, http::server::connection_ptr const&) >;

	using info_callback_fn = std::function<
		void(http::server::connection_ptr const&) >;


	/// \brief Sending and receiving messages via Websocket
	class service{
	public:
		/// \brief Construct the service
		service(
			data_callback_fn const& utf8_callback = data_callback_fn(),
			data_callback_fn const& binary_callback = data_callback_fn(),
			info_callback_fn const& new_connection_callback =
				info_callback_fn(),
			info_callback_fn const& connection_close_callback =
				info_callback_fn()
		);


		/// \brief Add a TCP connection, which is initialized as a WebSocket
		///        connection
		void add_connection(http::server::connection_ptr const& connection);

		/// \brief Send a UTF-8 encoded text message to all Clients
		void send_utf8(std::string const& message);

		/// \brief Send a UTF-8 encoded text message to a Client
		void send_utf8(
			std::string const& message,
			http::server::connection_ptr connection
		);

		/// \brief Send binary data to all Clients
		void send_binary(std::string const& data);

		/// \brief Send binary data to a Client
		void send_binary(
			std::string const& data,
			http::server::connection_ptr connection
		);

		/// \brief Send a close frame to all clients
		void close(
			std::uint16_t status,
			std::string const& reason = std::string()
		);

		/// \brief Send a close frame to a client
		void close(
			std::uint16_t status,
			std::string const& data,
			http::server::connection_ptr const& connection
		);

		/// \brief Reject all new connections and send a close frame to all
		///        clients
		void shutdown(
			std::uint16_t status,
			std::string const& reason = std::string()
		);


	private:
		/// \brief Every connection has its own parser and list of continuation
		///        frames
		struct connection_info{
			/// \brief The parser for iterpreating the incomming data
			request_parser parser;

			/// \brief The list of the last data frames who where not final
			std::vector< frame > continuation_frames;
		};

		using connection_info_ptr = std::shared_ptr< connection_info >;

		/// \brief The list of all initialized connections
		std::map< http::server::connection_ptr, connection_info_ptr >
			connections_;

		/// \brief Protect connections_
		std::mutex mutex_;

		/// \brief function is called when the service receives a text message
		data_callback_fn utf8_callback_;

		/// \brief function is called when the service receives a binary message
		data_callback_fn binary_callback_;

		/// \brief function is called when the service receives a new connection
		info_callback_fn new_connection_callback_;

		/// \brief function is called when the service removes a connection
		info_callback_fn connection_close_callback_;

		/// \brief true, if the function shutdown() was called
		std::atomic< bool > shutdown_;

		/// \brief Callback for connection::ready_callback
		void initialized(http::server::connection_ptr const& connection);

		/// \brief Send a pong frame to a client
		void pong(
			std::string const& message,
			http::server::connection_ptr const& connection
		);

		/// \brief Receives incomming data
		void receive(
			http::server::connection_ptr const& connection,
			std::string const& data
		);

		/// \brief Handles an incomming frame
		bool handle_frame(
			std::vector< frame >& continuation_frames,
			websocket::frame const& frame,
			http::server::connection_ptr const& connection
		);

		/// \brief Send blank data to all client, disconnect after operation in
		///        error case
		void send_frame_data(std::shared_ptr< std::string const > const& data);

		/// \brief Send blank data to a client, disconnect after operation in
		///        error case
		void send_frame_data(
			std::shared_ptr< std::string const > const& data,
			http::server::connection_ptr const& connection
		);

		/// \brief Send blank close frame data to all client, disconnect after
		///        operation
		void send_close_frame_data(
			std::shared_ptr< std::string const > const& data
		);

		/// \brief Send blank close frame data to a client, disconnect after
		///        operation
		void send_close_frame_data(
			std::shared_ptr< std::string const > const& data,
			http::server::connection_ptr const& connection
		);

		/// \brief Removes a connection from the list
		void remove_connection(http::server::connection_ptr const& connection);

		/// \brief Get all keys of connections_
		std::vector< http::server::connection_ptr > get_connections();
	};


}


#endif
