#include "richiesta.h"
#include <fcgiapp.h>
#include <cassert>
#include <cstdlib>
#include <string>
#include <cstring>

using namespace std;

void Richiesta::parse_query()
{
    char* str;
    METODO metodo;
    str=FCGX_GetParam("REQUEST_METHOD", fcgi_request.envp);
    if (strcmp ( str, "GET")==0){
        metodo=GET;
    }else if (strcmp ( str, "POST")==0){
        metodo=POST;
    }else {
        metodo=UNDEFINED;
    }
    switch (metodo){
        case GET:
            processGET();
            break;
        case POST:
            processPOST();
            break;
        default:
            m_tipo=IGNOTA;
    };

    already_parsed = true;
}

void Richiesta::processGET()
{
	m_tipo = IGNOTA;

	char* str = FCGX_GetParam("QUERY_STRING", fcgi_request.envp);
	if ((strlen(str) > 9) && (strncmp(str,"ricevi_da=",9)==0)) {
		errno = 0;
		unsigned long t = strtoul(str + 10, NULL, 10);
		if ((t == 0) || (errno != 0))
			return;
		m_tipo = RICEZIONE;
		m_da = (int) t;
	}
}

void Richiesta::processPOST()
{
	m_tipo = IGNOTA;

	char str[2000];
	size_t indiceAutore;        //posizione in cui si trova il nome dell'autore
								//dovrebbe essere zero
	size_t indiceTesto;         //posizione in cui si trova il testo

	int byte_letti = FCGX_GetStr(str, sizeof(str)-1, fcgi_request.in);
	str[byte_letti] = '\0';
	if (strlen(str) != ((size_t) byte_letti)) {
		// Rifiutiamo stringhe contenenti byte nulli
		return;
	}

	string stringa(str);
	indiceTesto=stringa.find("testo=");
	indiceAutore=stringa.find("autore=");
	if ((indiceTesto == string::npos) || (indiceTesto == string::npos) || ((indiceAutore != 0) && (indiceTesto != 0)))
		return;

	if (indiceAutore <= indiceTesto) {
		assert(indiceAutore == 0);
		m_autore = string(str+7, indiceTesto-(7+1));
		m_testo = string(str+indiceTesto+6, strlen(str)-(indiceTesto+6));
	} else {
		assert(indiceTesto == 0);
		m_testo = string(str+6, indiceAutore-(6+1));
		m_autore = string(str+indiceAutore+7, strlen(str)-(indiceAutore+7));
	}

	m_tipo = INVIO;
}

