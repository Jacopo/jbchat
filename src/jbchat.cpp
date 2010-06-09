#include <fcgiapp.h>

int main() {
	FCGX_Stream *in, *out, *err;
	FCGX_ParamArray envp;

	while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
	}

	return 0;
}

