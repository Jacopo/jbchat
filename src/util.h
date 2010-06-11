#ifndef _UTIL_H
#define _UTIL_H

#include "pthread.h"
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>

struct fcgi_error : std::runtime_error {
	explicit fcgi_error(const char *msg) : runtime_error(msg) { }
};

struct sys_error : std::runtime_error {
	int code;
	explicit sys_error(const char *msg) : runtime_error(msg), code(errno) {
		std::perror(msg);
	}
};



///
/// \brief A scoped lock (of a pthread mutex)
///
class HoldingMutex
{
public:
	///
	/// Constructor that locks the passed mutex
	HoldingMutex(pthread_mutex_t *nm) : m(nm) { if (pthread_mutex_lock(m)) throw sys_error("Mutex lock"); }
	///
	/// Destructor that unlocks the mutex
	~HoldingMutex() { if (pthread_mutex_unlock(m)) throw sys_error("Mutex unlock"); }
private:
	pthread_mutex_t* m; ///< The underlying pthread mutex

	// Copy is forbidden
	HoldingMutex(const HoldingMutex&);
	HoldingMutex& operator=(const HoldingMutex&);
};

#endif


