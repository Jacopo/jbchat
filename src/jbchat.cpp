#include <fcgiapp.h>
#include "util.h"

int main()
{
	if (FCGX_Init() != 0)
		throw fcgi_error("Init");

	for (;;) {
		FCGX_Request request;
		if (FCGX_InitRequest(&request, 0, 0) != 0)
			throw fcgi_error("InitRequest");

		if (FCGX_Accept_r(&request) != 0)
			break;

		if (FCGX_FPrintF(request.out,
			"Content-Type: text/xml; charset=\"utf-8\"\r\n"
			"\r\n"
			"<keepalive></keepalive>\r\n") == -1)
			throw fcgi_error("FPrintF");

		FCGX_Finish_r(&request);
		FCGX_Free(&request, 0);

		sleep(10);
	}
	return 0;
}

