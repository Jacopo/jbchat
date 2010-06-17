#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <time.h>

#include <fcgiapp.h>

#include <memory>

#include "util.h"
#include "richiesta.h"
#include "CodaMessaggi.h"
using namespace std;


#define BUFSIZE 10000000
#define MAX_INDICE 2000
#define KEEPALIVE_TIMEOUT 30		// In secondi


static int prossimo_indice;
static char buffer[BUFSIZE];
static char* inizio[MAX_INDICE+10];

static pthread_mutex_t mutex_inserimento = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t nuovi_messaggi = PTHREAD_COND_INITIALIZER;

static CodaMessaggi<Richiesta*> messaggi_in_attesa;

static void *thread_ricezione(void *arg)
{
	auto_ptr<Richiesta> preq = auto_ptr<Richiesta>((Richiesta*) arg);

	bool invia_keepalive = false;
	timespec waketime;

	if (clock_gettime(CLOCK_REALTIME, &waketime) != 0)
		throw sys_error("clock_gettime");
	waketime.tv_sec += KEEPALIVE_TIMEOUT;

	{
		HoldingMutex ml(&mutex_inserimento);
		while (preq->da() >= prossimo_indice)
			if (pthread_cond_timedwait(&nuovi_messaggi, &mutex_inserimento, &waketime) != 0) {
				// XXX: errno non viene impostato?!?
				invia_keepalive = true;
				break;
			}
	}


	if (invia_keepalive) {
		// Timeout raggiunto, nessun messaggio nuovo da inviare:
		// ci limitiamo a inviare un messaggio di keepalive
		char keepalive_msg[] = "Content-Type: text/xml; charset=\"utf-8\"\r\n\r\n<keepalive></keepalive>\r\n";
		if (FCGX_PutS(keepalive_msg, preq->out()) == -1)
			throw fcgi_error("PutS keepalive_msg");
		return 0;
	}


	// Inviamo al client i messaggi nel range [preq->da(), prossimo_indice)

	// 1) Header
	char xml_header[] = "Content-Type: text/xml; charset=\"utf-8\"\r\n\r\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<messaggi>\r\n";
	if (FCGX_PutS(xml_header, preq->out()) == -1)
		throw fcgi_error("PutS xml_header");

	// 2) Messaggi (presi direttamente dal buffer)
	//    Nota: prossimo_indice potrebbe essere stato incrementato nel frattempo, ma non c'è problema
	int len_invio = (int) (inizio[prossimo_indice] - inizio[preq->da()]);
	if (FCGX_PutStr(inizio[preq->da()], len_invio, preq->out()) != len_invio)
		throw fcgi_error("PutStr");

	// 3) Footer
	char xml_footer[] = "</messaggi>";
	if (FCGX_PutS(xml_footer, preq->out()) == -1)
		throw fcgi_error("PutS xml_footer");
	return 0;
}

static bool parse_invio(Richiesta *preq, string *ptesto, string *pautore)
{
	string stringa = preq->contenuto_post(), testo, autore;
	size_t indiceAutore;        //posizione in cui si trova il nome dell'autore
								//dovrebbe essere zero
	size_t indiceTesto;         //posizione in cui si trova il testo

	indiceTesto=stringa.find("testo=");
	indiceAutore=stringa.find("autore=");
	if ((indiceTesto == string::npos) || (indiceTesto == string::npos) || ((indiceAutore != 0) && (indiceTesto != 0)))
		return false;

	if (indiceAutore <= indiceTesto) {
		assert(indiceAutore == 0);
		*pautore = string(stringa, 7, indiceTesto-(7+1));
		*ptesto = string(stringa, indiceTesto+6, stringa.size()-(indiceTesto+6));
	} else {
		assert(indiceTesto == 0);
		*ptesto = string(stringa, 6, indiceAutore-(6+1));
		*pautore = string(stringa, indiceAutore+7, stringa.size()-(indiceAutore+7));
	}
	return true;
}

static void gestisci_invio(Richiesta *preq)
{
	// 1) Parsa testo e autore a partire dal contenuto del POST
	string testo, autore;
	if (parse_invio(preq, &testo, &autore) == false) {
		preq->rispondi_con_400();
		return;
	}
	// Ignora messaggi vuoti
	if (testo.size() == 0) {
		preq->rispondi_OK();
		return;
	}
	if (prossimo_indice >= MAX_INDICE)
		preq->rispondi_con_400();

	// 2) Verifica sicurezza input
	// Il testo sarà in una sezione CDATA: è sufficiente assicurarsi che non compaia il terminatore
	if (testo.find("]]>") != string::npos)
		preq->rispondi_con_400();
	// L'autore invece è in un attributo
	// TODO: verifica lista proibiti (es: UTF-8 crea problemi?)
	if ((autore.find('<') != string::npos) || (autore.find('>') != string::npos) || (autore.find('\\') != string::npos) ||
		(autore.find('"') != string::npos) || (autore.find('\'') != string::npos) || (autore.find('&') != string::npos))
		preq->rispondi_con_400();

	size_t spazio_disponibile = BUFSIZE - (buffer - inizio[prossimo_indice]);
	int len_xml = snprintf(inizio[prossimo_indice],
							  spazio_disponibile,
							  "<msg autore=\"%s\" numero=\"%d\"><![CDATA[%s]]></msg>\r\n",
							  autore.c_str(), prossimo_indice, testo.c_str());
	if (len_xml < 0)
		throw sys_error("snprintf");
	if (((unsigned) len_xml) >= spazio_disponibile)
		// Spazio nel buffer terminato
		preq->rispondi_con_400();


	inizio[prossimo_indice+1] = inizio[prossimo_indice] + len_xml;
	{
		HoldingMutex ml(&mutex_inserimento);
		prossimo_indice++;
	}

	preq->rispondi_OK();

	if (pthread_cond_broadcast(&nuovi_messaggi) != 0)
		throw sys_error("pthread_cond_broadcast");
}

static void *thread_gestione_invii(void *)
{
	Richiesta *preq;
	for (;;) {
		preq = messaggi_in_attesa.ricevi();
		gestisci_invio(preq);
		delete preq;
	}
	return 0;
}


int main()
{
	if (FCGX_Init() != 0)
		throw fcgi_error("Init");

	prossimo_indice = 1;
	inizio[prossimo_indice] = buffer;
#ifndef NDEBUG
	memset(buffer, 0xCC, sizeof(buffer));
#endif

	pthread_t gestore_invii;
	if (pthread_create(&gestore_invii, NULL, thread_gestione_invii, NULL) != 0)
		throw sys_error("pthread_create gestore invii");
	if (pthread_detach(gestore_invii) != 0)
		throw sys_error("pthread_detach gestore invii");

	for (;;) {
		Richiesta *preq = new Richiesta();
		preq->accetta_nuova();

		pthread_t newt;
		switch (preq->tipo()) {
		case Richiesta::INVIO:
			messaggi_in_attesa.accoda(preq);
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
