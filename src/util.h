#ifndef _UTIL_H
#define _UTIL_H

#include <stdexcept>

struct fcgi_error : std::runtime_error {
	explicit fcgi_error(const char *msg) : runtime_error(msg) { }
};

struct sys_error : std::runtime_error {
	explicit sys_error(const char *msg) : runtime_error(msg) { }
};

#endif


