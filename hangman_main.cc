/* Hangman algorithm tester.
   GPL Copyright 2016, Nathan Parker
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "hangman_maker.h"
#include "hangman_player.h"

using namespace std;

//const char* WORD_FILENAME = "/usr/share/dict/american-english";
const char* WORD_FILENAME = "./american-english";


// Read one word per line, and through out ones that we wouldn't
// use in the game of Hangman.  Expects something like 
// /usr/share/dict/american-english, and writes to *output_set.
// Returns True if it succeeds.
bool ReadWordList(const char* dict_filename, StringSet* output_set) {
  ifstream word_file;
  word_file.open(dict_filename);
  if (!word_file.is_open()) {
    perror("Cannot open word file");
    return false;
  }

  string word;
  while (!word_file.eof()) {
    // Read one word, strip it, lowercase it
    word_file >> word;
    word.erase(word.find_last_not_of(" \n\r")+1);
    if (word.empty())
      continue;
    transform(word.begin(), word.end(), word.begin(), ::tolower);

    // Drop things with dashes or apostophies or anything else.
    // Only accept words that use our allowed char set.
    if (word.find_first_not_of(HangmanMaker::AllChars()) != string::npos)
      continue;

    output_set->insert(word);
   }
  word_file.close();

  return output_set->size() != 0;
}


void Usage(const char* exec_name) {
  fprintf(stderr, "Usage: %s [-w word]\n"
      "\tWithout args, will play all words.\n",
       exec_name);
  exit(-1);
}

int main(int argc, char* argv[]) {
  string word_to_find;
  int opt;
  while ( (opt = getopt(argc, argv, "w:")) != -1 ) {
    switch ( opt ) {
      case 'w':
        word_to_find = optarg;
        break;
      defailt:
        Usage(argv[0]);
    }
  }


  StringSet words;
  if (!ReadWordList(WORD_FILENAME, &words))
    exit(-1);

  HPCPlayer player(words);
  HangmanMaker maker(&player);

  if (!word_to_find.empty())
    maker.PlayWord(word_to_find);
  else
    maker.PlayManyWords(words);

  return 0;
}
