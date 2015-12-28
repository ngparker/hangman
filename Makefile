
PROGNAME = hangman
SOURCES = hangman.cc

CPPFLAGS = -std=c++11

$(PROGNAME) : $(SOURCES) Makefile
	$(CXX) -o $@ $(CPPFLAGS) $(LDFLAGS) $(SOURCES)
