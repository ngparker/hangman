/* Hangman solver.
   GPL Copyright 12015, Nathan Parker
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const char* WORD_FILENAME = "/usr/share/dict/american-english";
const string ALPHA_LOWER = "abcdefghijklmnopqrstuvwxyz";

class Hangman {
public:
  typedef set<string> StringSet;
  typedef set<char> CharSet;
  StringSet all_words_;
  map<size_t, StringSet> words_by_len_;

// Read WORD_FILENAME filter+store words.
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
  printf("Found %d words:\n", all_words_.size());

  for (auto& itr : words_by_len_) {
    printf("  %2d chars: %5d words\n", itr.first, itr.second.size());
  }
  printf("\n");

  return true;
}

// Insert a word in our master lists
void AddOneWord(const string& word) {
  all_words_.insert(word);
  words_by_len_[word.size()].insert(word);
}

// The meat of the algorithm.  Pick the next best guess.
// Can optioanlly set word_guess if we want to guess the full word.
char PickNextChar(const CharSet& tried,
    const StringSet& active_set, string* word_guess) {
  // Number of active words with a given char, indexed by position
  // in ALPHA_LOWER.
  vector<size_t> active_counts;
  active_counts.resize(ALPHA_LOWER.size());

  size_t char_pos = 0;
  size_t biggest_count = 0;
  char biggest_char = '!';
  const string* potential_word_guess = NULL;
  bool multiple_potential = false;
  for (size_t char_pos = 0; char_pos < ALPHA_LOWER.size(); char_pos++) {
    char c = ALPHA_LOWER[char_pos];
    if (tried.count(c) != 0)
      continue;
    // Count how mant words have this char.
    for (const string& w : active_set) {
      if (w.find(c) != string::npos) {
        active_counts[char_pos]++;
        if (active_counts[char_pos] >= biggest_count) {
          biggest_count = active_counts[char_pos];
          biggest_char = c;
          // Save this word as a possible guess.  If there is ever
          // another possible guess, we won't guess.
          if (!potential_word_guess) {
            potential_word_guess = &w;
          } else {
            if (potential_word_guess != &w) {
              multiple_potential = true;
            }
          }
        }
      }
    }
  }
  printf("  Picking '%c' with %zu possible matches (%0.2f%%)\n",
      biggest_char, biggest_count, 100.0*biggest_count /
        active_set.size());
  if (biggest_count == 1 and !multiple_potential) {
    *word_guess = *potential_word_guess;
  }
  if (tried.count(biggest_char)) {
    printf("Ack! trying one I've tried before\n");
  }
  if (biggest_count == 0) {
    printf("Failed to find a new char!\n");
    exit(-1);
  }
  return biggest_char;
}

// Returns true iff the word matches the pattern.  _'s are wild.
// Assumes they are the same length.
bool PatternMatches(const string& pattern, const string& word) {
  for (size_t i = 0; i < pattern.size(); ++i) {
    if (pattern[i] != '_' && pattern[i] != word[i])
      return false;
  }
  return true;
}

// Play against ourselves with a given word.
void RunWordMatch(const string& word_to_find) {
  printf("Going to play against word '%s'.  %d chars.\n", word_to_find.c_str(),
      word_to_find.size());

  StringSet possible_set = words_by_len_[word_to_find.size()];
  // Pattern is all _'s.
  string pattern;
  for (auto z : word_to_find) {
    pattern += "_";
  }

  // While we still have some unmatched chars in the pattern,
  // keep guessing.
  CharSet tried;
  while (pattern.find('_') != string::npos) {
    string word_guess;
    char choice = PickNextChar(tried, possible_set, &word_guess);

    if (!word_guess.empty()) {
      printf("Guessing word '%s'\n", word_guess.c_str());
      if (word_guess != word_to_find) {
        printf("WRONG!\n");
      } else {
        printf("Found it in %d guesses\n", tried.size() + 1);
      }
      return;
    }

    // See if we got a match.
    size_t found = 0;
    string old_pattern = pattern;
    for (size_t i=0; i < word_to_find.size(); i++) {
      if (word_to_find[i] == choice) {
        pattern[i] = choice;
        found++;
      }
    }
    // Remember that we've tried this char.
    tried.insert(choice);
    printf("Tried '%c' against pattern '%s' and %zu possibilities:"
        " Found %zu chars\n",
        choice, pattern.c_str(), possible_set.size(), found);


    // Shorten our possible_set
    StringSet new_set;
    if (found) {
      // Compare against the new pattern
      for (const auto& w : possible_set) {
        if (PatternMatches(pattern, w)) {
          new_set.insert(w);
        }
      }
    } else {
      // Remove words that have letter not present.
      for (const auto& w : possible_set) {
        if (w.find(choice) == string::npos) {
          new_set.insert(w);
        }
      }
    }
    possible_set.swap(new_set);
  }
  printf("Found solution after %d attempts.\n", tried.size());
}


};



void Usage(const char* exec_name) {
  fprintf(stderr, "Usage: %s -w [word]\n", exec_name);
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

  if (word_to_find.empty())
    Usage(argv[0]);

  Hangman h;

  if (!h.ReadWordList())
    exit(-1);

  h.RunWordMatch(word_to_find);

  return 0;
}
