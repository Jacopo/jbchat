#ifndef _RICHIESTA_H
#define _RICHIESTA_H

#include <fcgiapp.h>
#include "util.h"

class Richiesta {
public:
	enum TIPO_RICHIESTA { RICEZIONE, INVIO, IGNOTA };
	enum METODO {GET,POST,UNDEFINED};

	explicit Richiesta() : already_parsed(false) {
		if (FCGX_InitRequest(&fcgi_request, 0, 0) != 0)
			throw fcgi_error("InitRequest");
	}
	~Richiesta() { FCGX_Finish_r(&fcgi_request); FCGX_Free(&fcgi_request, 0); }

	FCGX_Stream* out() { return fcgi_request.out; }


	TIPO_RICHIESTA tipo() { if (!already_parsed) parse_query(); return m_tipo; }
	int da() { if (!already_parsed) parse_query(); return m_da; }
	std::string testo() { if (!already_parsed) parse_query(); return m_testo; }
	std::string autore() { if (!already_parsed) parse_query(); return m_autore; }



	// Risposte predefinite
	void rispondi_con_400() {
		if (FCGX_FPrintF(out(),
			"Status: 400 Bad Request\r\n"
		    "Content-Type: text/plain; charset=\"utf-8\"\r\n"
		    "\r\n"
			"400 Il client ha inviato una richiesta incomprensibile.\r\n") == -1)
			throw fcgi_error("FPrintF");
	}
	void rispondi_OK() {
		if (FCGX_FPrintF(out(),
			"Content-Type: text/plain; charset=\"utf-8\"\r\n"
			"\r\n"
			"OK") == -1)
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
	std::string m_testo;
	std::string m_autore;

	bool already_parsed;

	void processGET();
	void processPOST();
	void parse_query();
};

#endif
