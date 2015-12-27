#include "stdio.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

const char* WORD_FILENAME = "/usr/share/dict/american-english";
const char* ALPHA_LOWER = "abcdefghijklmnopqrstuvwxyz";

class Hangman {
public:
  typedef set<string> StringSet;
  StringSet all_words_;
  map<size_t, StringSet> words_by_len_;

bool ReadWordList() {

  ifstream word_file;
  word_file.open(WORD_FILENAME);
  if (!word_file.is_open()) {
    perror("Cannot open word file");
    return false;
  }
  string word;
  while (!word_file.eof()) {
    // Read one word, strip, lowercase it
    word_file >> word;
    word.erase(word.find_last_not_of(" \n\r")+1);
    if (word.empty())
      continue;
    transform(word.begin(), word.end(), word.begin(), ::tolower);

    // Drop things with dashes or apostophies or anything else.
    if (word.find_first_not_of(ALPHA_LOWER) != string::npos)
      continue;

    AddOneWord(word);
   }
  word_file.close();
  printf("Found %d words.\n", all_words_.size());
  return true;
}
void AddOneWord(const string& word) {
  all_words_.insert(word);
  words_by_len_[word.size()].insert(word);
}


};

int main(int argc, char* argv[]) {

  Hangman h;

  if (!h.ReadWordList())
    return -1;

  return 0;
}
