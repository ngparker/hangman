/* Hangman game maker, and scorer.
   GPL Copyright 2016, Nathan Parker
*/

#ifndef HANGMAN_MAKER_H_
#define HANGMAN_MAKER_H_

#include "hangman_player.h"

#include <string>
#include <set>


class HangmanPlayer;


// The game-maker.
class HangmanMaker {
public:

  // Constructor. Does not own player.
  HangmanMaker(HangmanPlayer* player) : player_(player) { }

  // Returns true iff the word matches the pattern.  _'s are wild.
  // Assumes they are the same length.
  static bool PatternMatches(const std::string& pattern, const std::string& word);

  // All valid chars that could be in a word.
  static const std::string& AllChars() {return all_chars_;}

  // Play all the words in word_set. Collects stats
  void PlayManyWords(const std::set<std::string>& word_set);

  // Have player_ try to guess the word_to_find.  Collect stats.
  void PlayWord(const std::string& word_to_find);

  // Print stats collected so far for this player.
  void PrintStats() const;

private:
  HangmanPlayer* player_;

  static std::string all_chars_;

  // Total stats for this player
  struct TotalsStats {
    size_t words_tried;
    size_t guesses;
    size_t wrong_guesses;

    size_t word_guesses;
    size_t wrong_word_guesses;
  } totals;
};


#endif // #ifndef HANGMAN_MAKER_H_
