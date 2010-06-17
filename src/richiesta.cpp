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
	int byte_letti = FCGX_GetStr(str, sizeof(str)-1, fcgi_request.in);
	str[byte_letti] = '\0';
	if (strlen(str) != ((size_t) byte_letti)) {
		// Rifiutiamo stringhe contenenti byte nulli
		return;
	}

	// Per non sprecare risorse del main thread, il parsing vero e proprio
	// verrà fatto in seguito
	m_contenuto_post = str;
	m_tipo = INVIO;
}

