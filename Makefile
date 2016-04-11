
PROGNAME = hangman
SOURCES = hangman_main.cc hangman_maker.cc hangman_player.cc
HEADERS = hangman_maker.h hangman_player.cc

CPPFLAGS = -std=c++11

$(PROGNAME) : $(SOURCES) $(HEADERS) Makefile
	$(CXX) -o $@ $(CPPFLAGS) $(LDFLAGS) $(SOURCES)
