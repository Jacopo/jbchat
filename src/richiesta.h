#ifndef _RICHIESTA_H
#define _RICHIESTA_H

#include <fcgiapp.h>
#include "util.h"

class Richiesta {
public:
	enum TIPO_RICHIESTA { RICEZIONE, INVIO, IGNOTA };

	explicit Richiesta() : already_parsed(false) {
		if (FCGX_InitRequest(&fcgi_request, 0, 0) != 0)
			throw fcgi_error("InitRequest");
	}
	~Richiesta() { FCGX_Finish_r(&fcgi_request); FCGX_Free(&fcgi_request, 0); }

	FCGX_Stream* out() { return fcgi_request.out; }


	TIPO_RICHIESTA tipo() { if (!already_parsed) parse_query(); return m_tipo; }
	int da() { if (!already_parsed) parse_query(); return m_da; }


	// Risposte predefinite
	void rispondi_con_400() {
		if (FCGX_FPrintF(out(),
		    "Status: 400 Bad Request"
		    "Content-Type: text/plain; charset=\"utf-8\"\r\n"
		    "\r\n"
		    "Il client ha inviato una richiesta incomprensibile.\r\n") == -1)
			throw fcgi_error("FPrintF");
	}



	void accetta_nuova() {
		// Nota: va chiamato _solo_ dal thread principale
		if (FCGX_Accept_r(&fcgi_request) != 0)
			throw fcgi_error("Accept_r");
	}

private:
	FCGX_Request fcgi_request;

	TIPO_RICHIESTA m_tipo;
	int m_da;

	bool already_parsed;

	void parse_query();
};

#endif
