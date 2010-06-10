#include <pthread.h>
#include <sys/types.h>
#include <fcgiapp.h>
#include <stdint.h>
#include <memory>
#include "util.h"
#include "richiesta.h"
using namespace std;


#define BUFSIZE 8192


static volatile int punto_inserimento = 0;
//static uint8_t buffer[BUFSIZE];


void *thread_ricezione(void *arg)
{
	auto_ptr<Richiesta> preq = auto_ptr<Richiesta>((Richiesta*) arg);
	preq->da();

	sleep(10);

	if (FCGX_FPrintF(preq->out(),
		"Content-Type: text/xml; charset=\"utf-8\"\r\n"
		"\r\n"
		"<keepalive></keepalive>\r\n") == -1)
		throw fcgi_error("FPrintF");

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

