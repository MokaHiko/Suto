#pragma once

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include <array>
#include <functional>
#include <vector>
#include <thread>
#include <memory>
