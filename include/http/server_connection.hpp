//-----------------------------------------------------------------------------
// Copyright (c) 2012-2018 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _http__server_connection__hpp_INCLUDED_
#define _http__server_connection__hpp_INCLUDED_

#include "reply.hpp"
#include "request.hpp"
#include "server_request_handler.hpp"
#include "server_request_parser.hpp"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <mutex>


namespace http::server{


	using boost::system::error_code;
	using asio::ip::tcp;


	class connection;

	using connection_ptr = std::shared_ptr< connection >;
	using weak_connection_ptr = std::weak_ptr< connection >;


	using callback_write_fn = std::function<
		void(connection_ptr const&, error_code const&) >;

	using callback_read_fn = std::function<
		void(connection_ptr const&, std::string const&, error_code const&) >;


	/// \brief Represents a single connection from a client.
	///
	/// final because of enable_shared_from_this
	class connection final:
		private boost::noncopyable,
		public std::enable_shared_from_this< connection >{
	public:
		/// \brief Shutdown the socket.
		~connection();

		/// \brief Get the socket associated with the connection.
		tcp::socket& socket();

		/// \brief Start the first asynchronous operation for the connection.
		void start(request_handler& handler);

		/// \brief The callback is called, when the start function has finished
		void ready_callback(callback_write_fn callback);

		/// \brief Start another asynchronous read operation
		void read(callback_read_fn callback);

		/// \brief Start a synchronous write operation
		error_code write(std::shared_ptr< std::string const > const& data);

	protected:
		/// \brief Construct a connection with the given io_service.
		explicit connection(asio::io_service& io_service);


	private:
		/// \brief Handle completion of the first read operation.
		void handle_first_read(
			request_handler& handler,
			std::shared_ptr< http::request > const& request,
			std::shared_ptr< request_parser > const& request_parser,
			std::shared_ptr< http::reply > const& reply,
			error_code const& err,
			std::size_t bytes_transferred
		);

		/// \brief Handle completion of the first write operation.
		void handle_first_write(error_code const& err);

		/// \brief Strand to ensure the connection's handlers are not called
		///        concurrently.
		asio::io_service::strand strand_;

		/// \brief Socket for the connection.
		tcp::socket socket_;

		/// \brief Buffer for incoming data.
		std::array< char, 8192 > buffer_;

		/// \brief Is called after handle_first_write
		callback_write_fn ready_callback_;


		friend connection_ptr
			make_shared_connection(asio::io_service& io_service);
	};

	inline connection_ptr make_shared_connection(asio::io_service& io_service){
		return connection_ptr(new connection(io_service));
	}


}


#endif
