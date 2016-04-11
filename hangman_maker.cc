/* Hangman game maker, and scorer.
   GPL Copyright 2016, Nathan Parker
*/

#include "hangman_maker.h"
#include "hangman_player.h"

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

std::string HangmanMaker::all_chars_ =  "abcdefghijklmnopqrstuvwxyz";


// Returns true iff the word matches the pattern.  _'s are wild.
// Assumes they are the same length.
bool HangmanMaker::PatternMatches(
    const std::string& pattern, const std::string& word) {
  for (size_t i = 0; i < pattern.size(); ++i) {
    if (pattern[i] != '_' && pattern[i] != word[i])
      return false;
  }
  return true;
}

void HangmanMaker::PlayManyWords(const StringSet& word_set) {
  for (const auto& w : word_set) {
    PlayWord(w);
  }
}

// Have player_ try to guess the word_to_find.  Collect stats.
void HangmanMaker::PlayWord(const std::string& word_to_find) {
  totals.words_tried++;
  printf("Going to play against word '%s'.  %zu chars.\n", word_to_find.c_str(),
      word_to_find.size());

  // Pattern is all _'s.
  std::string pattern;
  for (auto c : word_to_find) {
    pattern += "_";
  }

  // Start a new game
  std::unique_ptr<HangmanPlayer::Game> game =
    player_->MakeNewGame(pattern.length());

  size_t guess_count = 0;
  size_t failed_guess_count = 0;
  std::string word_guess;
  do {
    char guessed_char = game->GuessNextChar(pattern, &word_guess);
    guess_count++;

    if (!word_guess.empty()) {
      printf("Guessing word '%s'\n", word_guess.c_str());
      totals.word_guesses++;
      if (word_guess != word_to_find) {
        // TODO: log via function so we can turn it off for repeated runs.
        printf("WRONG!  Failure.\n");
        totals.wrong_word_guesses++;
        return;
      } 
    } else {
      // See if we got a letter match.
      size_t found = 0;
      std::string old_pattern = pattern;
      for (size_t i=0; i < word_to_find.size(); i++) {
        if (word_to_find[i] == guessed_char) {
        pattern[i] = guessed_char;
        found++;
        }
      }
      totals.guesses++;
      if (!found)
        totals.wrong_guesses++;
    }

  } while (pattern != word_to_find && word_guess != word_to_find);


  printf("Found it in %zu guesses, %zu were wrong\n",
      guess_count, failed_guess_count);
}

void HangmanMaker::PrintStats() const {
  printf("Totals:\n");
  printf("  Words tried: %zu\n", totals.words_tried);
  printf("  Letter Guesses: %zu, %.2f per word\n", totals.guesses, 
      1.0 * totals.guesses / totals.words_tried);
  printf("  Wrong letter guesses: %zu, %.2f per word\n",
      totals.wrong_guesses, 
      1.0 * totals.wrong_guesses / totals.words_tried);
  printf("  Word guesses: %zu, %.2f per word\n", totals.word_guesses, 
      1.0 * totals.word_guesses / totals.words_tried);
  printf("  Wrong word guesses: %zu\n", totals.wrong_word_guesses);
}
