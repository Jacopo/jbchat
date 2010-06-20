#ifndef _UTIL_H
#define _UTIL_H

#include "pthread.h"
#include "semaphore.h"
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


///
/// \brief Wrapper for semaphores
///
class Semaphore
{
public:
	bool try_P() {
		errno = 0;
		if (sem_trywait(&s) != 0) {
			if (errno == EAGAIN)
				return false;
			else throw sys_error("Semaphore P");
		}
		return true;
	}
	void P() { if (sem_wait(&s) != 0) throw sys_error("Semaphore P"); }
	void V() { if (sem_post(&s) != 0) throw sys_error("Semaphore V"); }
	Semaphore(unsigned int initval = 0) { if (sem_init(&s, 0, initval) != 0) throw sys_error("Semaphore init"); }
	~Semaphore() { if (sem_destroy(&s) != 0) throw sys_error("Semaphore destroy"); }
private:
	sem_t s;
};

#endif


