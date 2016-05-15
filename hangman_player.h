/* Hangman game maker, and scorer.
   GPL Copyright 2016, Nathan Parker
*/

#ifndef HANGMAN_PLAYER_H_
#define HANGMAN_PLAYER_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>

// Interface for a player to implement.
class HangmanPlayer {
 public:
   virtual ~HangmanPlayer() {};

   // Interface for a single game.
   class Game {
    public:
      // The Maker calls this repeatedly with an updated pattern.
      // Player should fill in word_guess only if it has the answer.
      // Otherwise, it returns a char to guess.
      virtual char GuessNextChar(const std::string& pattern,
          std::string* word_guess) = 0;

      virtual ~Game() {};
    };
   virtual std::unique_ptr<Game> MakeNewGame(size_t word_len) = 0;
};


///
// The "Highest-Probability Character Choice" (HPC) algorithm impl.

typedef std::unordered_set<std::string> StringSet;

// Precomputed data
struct HPCData {
  std::unordered_map<size_t, StringSet> words_by_len;
};

class HPCPlayer : public HangmanPlayer {
  public:
    // This pre-computes things.
   HPCPlayer(const StringSet& dictionary);
   virtual ~HPCPlayer() override {};

   virtual std::unique_ptr<HangmanPlayer::Game> MakeNewGame(size_t word_len)
     override;

  private:
   HPCData data_;
};

#endif // #ifndef HANGMAN_PLAYER_H_
