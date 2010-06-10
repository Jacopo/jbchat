#include "richiesta.h"
#include <fcgiapp.h>
#include <string>
using namespace std;

void Richiesta::parse_query()
{
	// TODO
	FCGX_GetParam("REQUEST_METHOD", fcgi_request.envp);
	FCGX_GetParam("QUERY_STRING", fcgi_request.envp);

	m_tipo = RICEZIONE;
}
