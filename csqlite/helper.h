#pragma once
#include <source_location>
#include <iostream>
#include <format>

//#include <string>
//#include <vector>
//
//#include <cstdint>
//#include <filesystem>
//#include <numeric>

//inline static void LOG_ERROR(std::source_location sl = std::source_location::current()) {
//    auto const str = std::format("SQLite Error: {} ({}) => fn::{}().{}[{}]",
//                                 err_msg(),
//                                 err_code(),
//                                 sl.function_name(),
//                                 sl.line(),
//                                 sl.file_name()
//                                 );
//    std::cerr << str << '\n';
//}

#define LOG_ERROR() \
    auto fn = std::string(__FILE__); \
    if (auto pos = fn.find_last_of('/'); pos != std::string::npos) \
        fn = fn.substr(pos+1); \
    std::cerr << "SQLITE ERROR: " << err_msg() << " (" << err_code() << ") =>" << fn << "::" << __FUNCTION__ \
    << "()." << __LINE__ << "[" << __FILE__ << "]\n";

