
#pragma once

#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <string>

inline std::mutex printf_lock;

inline void log(std::string fmt, ...) {
	std::lock_guard lock(printf_lock);
	va_list args;
	va_start(args, fmt);
	vprintf(fmt.c_str(), args);
	va_end(args);
	fflush(stdout);
}

inline std::string last_file(std::string path) {
	size_t p = path.find_last_of("\\/") + 1;
	return path.substr(p, path.size() - p);
}

#define info(fmt, ...) (void)( \
	log("%s:%u [info] " fmt "\n", last_file(__FILE__).c_str(), __LINE__, ##__VA_ARGS__))

#define warn(fmt, ...) (void)( \
	log("\033[0;31m%s:%u [warn] " fmt "\033[0m\n", last_file(__FILE__).c_str(), __LINE__, ##__VA_ARGS__))

#define die(fmt, ...) (void)( \
	log("\033[0;31m%s:%u [fatal] " fmt "\033[0m\n", last_file(__FILE__).c_str(), __LINE__, ##__VA_ARGS__), \
	std::exit(__LINE__));

#ifdef _WIN32
#define fail_assert(msg, file, line) (void)( \
	log("\033[1;31m%s:%u [ASSERT] " msg "\033[0m\n", file, line), __debugbreak(), std::exit(__LINE__), 0)
#elif defined(__linux__)
#include <signal.h>
#define fail_assert(msg, file, line) (void)( \
	log("\033[1;31m%s:%u [ASSERT] " msg "\033[0m\n", file, line), raise(SIGTRAP), std::exit(__LINE__), 0)
#endif

#undef assert
#define assert(expr) (void)( \
		(!!(expr)) || \
		(fail_assert(#expr, last_file(__FILE__).c_str(), __LINE__), 0))
