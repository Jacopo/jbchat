#ifndef _CODAMESSAGGI_H
#define _CODAMESSAGGI_H

#include <queue>
#include "util.h"

// Coda circolare, singolo produttore, singolo consumatore
template <class T, unsigned int ARR_SIZE = 200> class CodaMessaggi {
public:
	bool accoda(T n) {
		if (spazio.try_P() == false)
			return false;
		arr[w++] = n;
		if (w == ARR_SIZE)
			w = 0;
		piena.V();
		return true;
	}

	T ricevi() {
		piena.P();
		T ret = arr[r++];
		if (r == ARR_SIZE)
			r = 0;
		spazio.V();
		return ret;
	}

	CodaMessaggi() : r(0), w(0), piena(0), spazio(ARR_SIZE) { }

private:
	int r;
	int w;
	T arr[ARR_SIZE];
	Semaphore piena;
	Semaphore spazio;
};

#endif
