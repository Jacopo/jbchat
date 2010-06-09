.POSIX:

TARGET = jbchat
CPPFILES = jbchat.cpp
GLOBAL_HEADERS = 
# Library dependencies: -pthread (or -lrt), -lm for math
DEPCFLAGS = -pthread
DEPLIBS = -lm -lfcgi++

# Default values for user-overridable flags:
CXXFLAGS = -Wall -Wextra -ansi -pedantic -Wundef -Wshadow -Wunsafe-loop-optimizations -Wcast-qual -Wwrite-strings -Wconversion -Winline -Wreorder -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo
#-Weffc++
#LDFLAGS = 
BUILD_FLAGS = -ggdb
#BUILD_FLAGS = -O3 -march=native -pipe -combine -fwhole-program


all: $(TARGET)

$(TARGET): $(CPPFILES) $(GLOBAL_HEADERS)
	$(CXX) $(BUILD_FLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DEPCFLAGS) $(DEPLIBS) $(CPPFILES) -o $@

.PHONY: clean all install check
clean:
	-rm -f $(TARGET)

install: all
	install -d $(DESTDIR)$(bindir)
	install $(DESTDIR)$(bindir)/$(TARGET)

check: all
	./test.sh
	@echo Test passed

