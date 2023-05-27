#ifndef GRADUATEWORK_PCH_H
#define GRADUATEWORK_PCH_H

#include <array>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ba = boost::asio;
namespace bfs = boost::filesystem;
namespace bj = boost::json;
namespace bpo = boost::program_options;

using namespace std::literals::string_literals;
using namespace std::chrono_literals;

enum class stream {
    flush
};

#endif // GRADUATEWORK_PCH_H