#pragma once

// set if the user is using boost or standalone asio

#ifdef USE_STANDALONE_ASIO

#include <asio.hpp>

#else

// using boost
#include <boost/asio.hpp>
namespace asio = boost::asio;

#endif