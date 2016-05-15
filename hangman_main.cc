/* Hangman algorithm tester.
   GPL Copyright 2016, Nathan Parker
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
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
  fprintf(stderr, "Usage:\n"
      " %s -w word\n"
      " %s -n count_of_words_to_try [-l word_len]\n",
      exec_name, exec_name);
  exit(-1);
}

// Pick out |num| words that are |word_len| long (or all if word_len==0);
void FilterWords(const StringSet& words, StringSet* new_words, int num,
    int word_len) {

  auto itr = words.begin();
  while (itr != words.end() && new_words->size() < num) {
    if (word_len == itr->size() || word_len == 0)
      new_words->insert(*itr);
    itr++;
  }
}

int main(int argc, char* argv[]) {
  string word_to_find;
  int word_count_to_find = 0;
  int word_len = 0;
  int opt;
  while ( (opt = getopt(argc, argv, "w:n:l:")) != -1 ) {
    switch ( opt ) {
      case 'w':
        word_to_find = optarg;
        if (word_to_find.empty())
          Usage(argv[0]);
        break;
      case 'n':
        word_count_to_find = strtol(optarg, NULL, 10);
        if (word_count_to_find <= 0)
          Usage(argv[0]);
        break;
      case 'l':
        word_len = strtol(optarg, NULL, 10);
        if (word_len <= 0)
          Usage(argv[0]);
        break;
      default:
        Usage(argv[0]);
    }
  }

  if (!((word_count_to_find == 0) ^ word_to_find.empty()))
    Usage(argv[0]);

  StringSet words;
  if (!ReadWordList(WORD_FILENAME, &words))
    exit(-1);

  HPCPlayer player(words);
  HangmanMaker maker(&player);

  if (!word_to_find.empty()) {
    maker.PlayWord(word_to_find);
  } else {
    StringSet new_words;
    FilterWords(words, &new_words, word_count_to_find, word_len);
    maker.PlayManyWords(new_words);
    maker.PrintStats();
  }

  return 0;
}
