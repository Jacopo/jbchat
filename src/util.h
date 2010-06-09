#include <stdexcept>

struct fcgi_error : std::runtime_error {
	explicit fcgi_error(const char *msg) : runtime_error(msg) { }
};

