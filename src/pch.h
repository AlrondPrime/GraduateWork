#ifndef GRADUATEWORK_PCH_H
#define GRADUATEWORK_PCH_H

#include <array>
#include <bitset>
#include "boost/program_options.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/string_view.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
namespace ba = boost::asio;

using namespace std::literals::string_literals;

enum class stream {
    flush
};

#endif // GRADUATEWORK_PCH_H