.POSIX:

TARGET = ../wwwroot/jbchat.fcgi
CPPFILES = jbchat.cpp
GLOBAL_HEADERS = 
# Library dependencies: -pthread (or -lrt), -lm for math
DEPCFLAGS = -pthread
DEPLIBS = -lm -lfcgi++
FCGIDEBUG = ../fcgi-debug/build/fcgi-debug

# Default values for user-overridable flags:
CXXFLAGS = -Wall -Wextra -ansi -pedantic -Wundef -Wshadow -Wunsafe-loop-optimizations -Wcast-qual -Wwrite-strings -Wconversion -Winline -Wreorder -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo
#-Weffc++
#LDFLAGS = 
BUILD_FLAGS = -ggdb
#BUILD_FLAGS = -O3 -march=native -pipe -combine -fwhole-program


all: $(TARGET)

$(TARGET): $(CPPFILES) $(GLOBAL_HEADERS)
	$(CXX) $(BUILD_FLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DEPCFLAGS) $(DEPLIBS) $(CPPFILES) -o $@

.PHONY: clean all spawn spawn-debug
clean:
	-rm -f $(TARGET)

spawn: all
	spawn-fcgi -a 127.0.0.1 -p 9000 -- $(TARGET)

spawn-debug: all
	spawn-fcgi -a 127.0.0.1 -p 9000 -n -- $(FCGIDEBUG) $(TARGET)
