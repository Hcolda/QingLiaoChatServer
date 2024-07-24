#ifndef DEFINITION_HPP
#define DEFINITION_HPP

#ifdef MSC_VER
    #include <format>
    #include <filesystem>
    #ifdef _HAS_CXX23
        #include <stacktrace>
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
    #else
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
    #endif // !_HAS_CXX23

#else // !MSC_VER
    #include <format>
    #include <filesystem>
    #if defined(__cplusplus) && __cplusplus >= 202011L
        #include <stacktrace>
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\nstack trace: \n{}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__, std::to_string(std::stacktrace::current()))
    #else
        #define ERROR_WITH_STACKTRACE(errmsg) std::format("error: {}\nin file \"{}\" line {}\n", \
            errmsg, std::filesystem::path(__FILE__).filename().string(), __LINE__)
    #endif // !_HAS_CXX23
#endif // !MSC_VER

#endif // !DEFINITION_HPP