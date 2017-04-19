//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
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


	class connection;

	typedef std::shared_ptr< connection > connection_ptr;
	typedef std::weak_ptr< connection > weak_connection_ptr;

	/// \brief Represents a single connection from a client.
	///
	/// final because of enable_shared_from_this
	class connection final: private boost::noncopyable, public std::enable_shared_from_this< connection >{
	public:
		typedef std::function< void(connection_ptr const&, boost::system::error_code const&) > callback_write_function;
		typedef std::function< void(connection_ptr const&, std::string const&, boost::system::error_code const&) > callback_read_function;


		/// \brief Shutdown the socket.
		~connection();

		/// \brief Get the socket associated with the connection.
		boost::asio::ip::tcp::socket& socket();

		/// \brief Start the first asynchronous operation for the connection.
		void start(request_handler& handler);

		/// \brief The callback is called, when the start function has finished
		void ready_callback(callback_write_function callback);

		/// \brief Start another asynchronous read operation
		void read(callback_read_function callback);

		/// \brief Start a synchronous write operation
		boost::system::error_code write(std::shared_ptr< std::string const > const& data);

	protected:
		/// \brief Construct a connection with the given io_service.
		explicit connection(boost::asio::io_service& io_service);


	private:
		/// \brief Handle completion of the first read operation.
		void handle_first_read(
			request_handler& handler,
			std::shared_ptr< http::request > const& request,
			std::shared_ptr< request_parser > const& request_parser,
			std::shared_ptr< http::reply > const& reply,
			boost::system::error_code const& err,
			std::size_t bytes_transferred
		);

		/// \brief Handle completion of the first write operation.
		void handle_first_write(boost::system::error_code const& err);

		/// \brief Strand to ensure the connection's handlers are not called concurrently.
		boost::asio::io_service::strand strand_;

		/// \brief Socket for the connection.
		boost::asio::ip::tcp::socket socket_;

		/// \brief Buffer for incoming data.
		std::array< char, 8192 > buffer_;

		/// \brief Is called after handle_first_write
		callback_write_function ready_callback_;


		friend connection_ptr make_shared_connection(boost::asio::io_service& io_service);
	};

	inline connection_ptr make_shared_connection(boost::asio::io_service& io_service){
		return connection_ptr(new connection(io_service));
	}


}


#endif
