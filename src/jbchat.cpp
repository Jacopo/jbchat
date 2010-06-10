#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <fcgiapp.h>
#include <memory>
#include "util.h"
#include "richiesta.h"
using namespace std;


#define BUFSIZE 25000
#define KEEPALIVE_TIMEOUT 120		// In secondi


static int punto_inserimento = 0;
static char buffer[BUFSIZE];
static pthread_mutex_t mutex_inserimento = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t nuovi_messaggi = PTHREAD_COND_INITIALIZER;


void *thread_ricezione(void *arg)
{
	auto_ptr<Richiesta> preq = auto_ptr<Richiesta>((Richiesta*) arg);

	bool invia_keepalive = false;
	timespec waketime;

	if (clock_gettime(CLOCK_MONOTONIC, &waketime) != 0)
		throw sys_error("clock_gettime");
	waketime.tv_sec += KEEPALIVE_TIMEOUT;

	{
		HoldingMutex ml(&mutex_inserimento);
		while (preq->da() >= punto_inserimento)
			if (pthread_cond_timedwait(&nuovi_messaggi, &mutex_inserimento, &waketime) != 0) {
				if (errno == ETIMEDOUT)
					invia_keepalive = true;
				else throw sys_error("phtread_cond_wait");
			}
	}


	if (invia_keepalive) {
		// Timeout raggiunto, nessun messaggio nuovo da inviare:
		// ci limitiamo a inviare un messaggio di keepalive
		if (FCGX_FPrintF(preq->out(),
						 "Content-Type: text/xml; charset=\"utf-8\"\r\n"
						 "\r\n"
						 "<keepalive></keepalive>\r\n") == -1)
			throw fcgi_error("FPrintF");
		return 0;
	}

	// Leggiamo i messaggi da inviare dal buffer
	if (FCGX_PutStr(&buffer[preq->da()], punto_inserimento - preq->da(), preq->out()) != (punto_inserimento - preq->da()))
		throw fcgi_error("PutStr");

	return 0;
}



int main()
{
	if (FCGX_Init() != 0)
		throw fcgi_error("Init");

	// TODO: crea il thread per l'invio dei messaggi

	for (;;) {
		Richiesta *preq = new Richiesta();
		preq->accetta_nuova();

		pthread_t newt;
		switch (preq->tipo()) {
		case Richiesta::INVIO:
			// TODO: forwarda la richiesta al thread di invio messaggi
			delete preq;
			break;
		case Richiesta::RICEZIONE:
			if (pthread_create(&newt, NULL, thread_ricezione, preq) != 0)
				throw sys_error("pthread_create");
			if (pthread_detach(newt) != 0)
				throw sys_error("pthread_detach");
			break;
		default:
			preq->rispondi_con_400();
			delete preq;
			break;
		}
	}
	return 0;
}

