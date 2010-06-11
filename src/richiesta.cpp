#include "richiesta.h"
#include <fcgiapp.h>
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

void Richiesta::processPOST(){
    m_tipo=INVIO;
}

