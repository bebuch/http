//-----------------------------------------------------------------------------
// Copyright (c) 2012-2017 Benjamin Buch
//
// https://github.com/bebuch/http
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#include <http/websocket_server_request_parser.hpp>

#include <boost/algorithm/string.hpp>

#include <array>


namespace http::websocket::server{


	namespace impl{ // Never use these functions direct


		std::string data_decode(
			std::string const& in,
			std::uint32_t masking_key
		){
			std::array< unsigned char, 4 > key = {{
				static_cast< unsigned char >((masking_key >> 24) & 0xFF),
				static_cast< unsigned char >((masking_key >> 16) & 0xFF),
				static_cast< unsigned char >((masking_key >> 8 ) & 0xFF),
				static_cast< unsigned char >((masking_key >> 0 ) & 0xFF),
			}};

			std::string result;
			result.reserve(in.size());
			for(std::size_t i = 0; i < in.size(); ++i){
				result += static_cast< unsigned char >(in[i] ^ key[i % 4]);
			}

			return result;
		}


	}



	request_parser::request_parser():
		state_(frame_start),
		length_(0),
		masking_key_(0),
		payload_data_(""),
		counter_(0)
		{}

	void request_parser::reset(){
		state_ = frame_start;
	}

	boost::tribool request_parser::consume(websocket::frame& frame, char input){
		switch(state_){
			case frame_start:{
				bool rsv1 = (input & 0x40) != 0;
				bool rsv2 = (input & 0x20) != 0;
				bool rsv3 = (input & 0x10) != 0;
				if(rsv1 || rsv2 || rsv3) return false;

				fin_ = (input & 0x80) != 0;
				opcode_ = websocket::frame::opcode(input & 0x0F);
				state_ = length_1;

				length_       = 0;
				counter_      = 0;
				masking_key_  = 0;
				payload_data_ = "";

				return boost::indeterminate;
			}
			case length_1:{
				bool mask = (input & 0x80) != 0;
				if(!mask) return false;

				length_ = input & 0x7F;
				if(length_ == 126){
					state_ = length_2;
					length_  = 0;
				}else if(length_ == 127){
					state_ = length_8;
					length_  = 0;
				}else{
					state_ = masking_key;
				}

				counter_ = 0;

				return boost::indeterminate;
			}
			case length_2:{
				++counter_;
				length_ = (length_ << 8) | static_cast< std::uint8_t >(input);

				if(counter_ == 2){
					state_ = masking_key;
					counter_ = 0;
				}

				return boost::indeterminate;
			}
			case length_8:{
				++counter_;
				length_ = (length_ << 8) | static_cast< std::uint8_t >(input);

				if(counter_ == 8){
					state_ = masking_key;
					counter_ = 0;
				}

				return boost::indeterminate;
			}
			case masking_key:{
				++counter_;
				masking_key_ =
					(masking_key_ << 8) | static_cast< std::uint8_t >(input);

				if(counter_ == 4){
					state_ = payload_data;
					counter_ = 0;
				}

				return boost::indeterminate;
			}
			case payload_data:{
				++counter_;
				payload_data_ += input;

				if(counter_ == length_){
					state_ = frame_start;

					frame.fin  = fin_;
					frame.code = opcode_;
					frame.data = impl::data_decode(payload_data_, masking_key_);

					return true;
				}

				return boost::indeterminate;
			}
			default:
				return false;
		}
	}


}
