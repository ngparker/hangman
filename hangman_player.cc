/* Hangman player, HPC implementation. 
   HPC stands for "highest probability character".
   GPL Copyright 2016, Nathan Parker
*/

#include "hangman_player.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "hangman_maker.h"

// ------- HPCGame class and methods ---------

typedef std::set<char> CharSet;

class HPCGame : public HangmanPlayer::Game {
  public:
    // Constructor w/ precomputed data. Ptr not owned.
    HPCGame(size_t word_len, const HPCData* data);

    // HangmanPlayer::Game methods
    char GuessNextChar(const std::string& pattern, std::string* word_guess)
      override;

  private:
  void UpdateStateWithPattern(const std::string& new_pattern);

   const HPCData* data_;

   CharSet tried_;  // Characters already tried.
   StringSet active_set_;  // Words remaining

   std::string last_pattern_;  // The pattern we had last time. 
   char last_guessed_char_;  // The char we guessed last time.
};


HPCGame::HPCGame(size_t word_len, const HPCData* data)
  : data_(data), last_guessed_char_('!') {
  auto itr = data->words_by_len.find(word_len);
  if (itr != data->words_by_len.end()) {
    // Deep copy.
    active_set_ = itr->second;
  } else {
    printf("We have no %zu-char words!\n", word_len);
    exit(1);
  }

}
  
// We call this after we get an updated pattern in response to our guess.
void HPCGame::UpdateStateWithPattern(const std::string& new_pattern) {
  bool matched = (new_pattern != last_pattern_);

  // Shorten our active_set_
  StringSet new_set;
  if (matched) {
    // Filter out words that don't match the new pattern.
    for (const auto& w : active_set_) {
      if (HangmanMaker::PatternMatches(new_pattern, w))
        new_set.insert(w);
    }
  } else {
    // Remove words that don't have the last_guess char.
    for (const auto& w : active_set_) {
      if (w.find(last_guessed_char_) == std::string::npos)
        new_set.insert(w);
    }
  }
  printf("%s guess of '%c' reduced domain from %zu to %zu (by %.1f%%).\n",
      (matched ? "Successful" : "Failed"),
      last_guessed_char_, active_set_.size(), new_set.size(),
      (100.0 * (active_set_.size() - new_set.size())) / active_set_.size());

  active_set_.swap(new_set);
}

char HPCGame::GuessNextChar(const std::string& new_pattern,
    std::string* word_guess) {
  // Was there a previous guess?
  if (!last_pattern_.empty())
    UpdateStateWithPattern(new_pattern);
  last_pattern_= new_pattern;

  const std::string& all_chars = HangmanMaker::AllChars();

  // Number of active words with a given char, indexed by position
  // in all_chars.
  std::vector<size_t> active_counts;
  active_counts.resize(all_chars.size());

  size_t char_pos = 0;
  size_t biggest_count = 0;
  char biggest_char = '!';
  const std::string* potential_word_guess = NULL;
  bool multiple_potential = false;
  for (size_t char_pos = 0; char_pos < all_chars.size(); char_pos++) {
    char c = all_chars[char_pos];
    if (tried_.count(c) != 0)
      continue;

    // Count how many words have this char.
    for (const std::string& w : active_set_) {
      if (w.find(c) == std::string::npos)
        continue;

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
  if (biggest_count == 1 and !multiple_potential) {
    // We think we've got it -- guess the word!
    *word_guess = *potential_word_guess;
    return '!';  // not used.
  }

  printf("  Picking '%c' since there are %zu possible matches (%0.2f%%)\n",
      biggest_char, biggest_count, 100.0*biggest_count / active_set_.size());

  if (tried_.count(biggest_char)) {
    printf("Ack! trying one I've tried before\n");
    exit(-1);
  } else if (biggest_count == 0) {
    printf("Failed to find a new char!\n");
    exit(-1);
  }

  // Set state for next time.
  tried_.insert(biggest_char);
  last_guessed_char_ = biggest_char;

  return biggest_char;
}

/*

// Play against ourselves with a given word.
void RunWordMatch(const std::string& word_to_find) {
  printf("Going to play against word '%s'.  %zu chars.\n", word_to_find.c_str(),
      word_to_find.size());

  StringSet possible_set = words_by_len_[word_to_find.size()];
  // Pattern is all _'s.
  std::string pattern;
  for (auto z : word_to_find) {
    pattern += "_";
  }

  // While we still have some unmatched chars in the pattern,
  // keep guessing.
  CharSet tried;
  size_t failed_guess_count = 0;
  while (pattern.find('_') != std::string::npos) {
    std::string word_guess;
    char choice = PickNextChar(tried, possible_set, &word_guess);

    if (!word_guess.empty()) {
      printf("Guessing word '%s'\n", word_guess.c_str());
      if (word_guess != word_to_find) {
        printf("WRONG!\n");
      } else {
        printf("Found it in %zu guesses, %zu were wrong\n",
            tried.size() + 1, failed_guess_count);
      }
      return;
    }

    // See if we got a match.
    size_t found = 0;
    std::string old_pattern = pattern;
    for (size_t i=0; i < word_to_find.size(); i++) {
      if (word_to_find[i] == choice) {
        pattern[i] = choice;
        found++;
      }
    }
    // Remember that we've tried this char.
    tried.insert(choice);

    // Shorten our possible_set
    StringSet new_set;
    if (found) {
      // Compare against the new pattern
      for (const auto& w : possible_set) {
        if (HangmanMaker::PatternMatches(pattern, w)) {
          new_set.insert(w);
        }
      }

    } else {
      failed_guess_count++;
      // Remove words that have letter not present.
      for (const auto& w : possible_set) {
        if (w.find(choice) == std::string::npos) {
          new_set.insert(w);
        }
      }
    }
    printf("Guess #%zu (%zu): '%c'. Pattern '%s'. Possibilities: %zu -> %zu\n",
        tried.size(), failed_guess_count, choice, pattern.c_str(),
        possible_set.size(), new_set.size());
    possible_set.swap(new_set);
  }
  printf("Found it in %zu guesses, %zu were wrong\n",
      tried.size(), failed_guess_count);
}
*/

// ------- HPCPlayer methods ---------
HPCPlayer::HPCPlayer(const StringSet& dictionary) {
  // Pre-compute stuff
  for (auto w : dictionary)
    data_.words_by_len[w.size()].insert(w);
}

std::unique_ptr<HangmanPlayer::Game> HPCPlayer::MakeNewGame(size_t word_len) {
  return std::unique_ptr<HangmanPlayer::Game>(new HPCGame(word_len, &data_));
}

