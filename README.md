# Hangman Algorithm Experimentation

## TL;DR
This is some experimental code to test implementations of algorithms for solving the game of Hangman. 

## The Code
The code is split into a `HangmanMaker` which picks the words and controls the game, and the `HangmanPlayer` which
guesses letters.  Both are given the dictionary of possible words. Via the command line you can give the Maker a word to
play against the Player, to see how it does.  You can also run all the words in the dicitonary, and look at stats
of how well it did. The code it not optimized for speed.

## Fitness Function
In the [classic game](https://en.wikipedia.org/wiki/Hangman_(game)), the player gets a win if they can guess the word with
less than six incorrect guesses. Total guesses is irrelevant. The overall score is based on wins and losses on multiple games.

My hunch (not proven) is that the most optimal algorithm for for this game is merely to guess the letter that is present in
the most words that match the current pattern.  I've implemented that in `HPCPlayer` class ("High Probability Character"
Player), and it does fairly well.  It's possible that this may optimize for fewest-wrong-choices, but not necessarily
for most wins with less than six wrong choices.  An algorithm that always had five wrong choices would be better than one that
averaged three but sometimes got nine.

A more interesting problem is if we choose to optimize for the fewest guesses overall (minimize the sum of guesses over
multiple games
wins).  In this case we need to chose the letter that not only reduces the search space the most, but also provides
likely future options that will reduce the search space the most.  I haven't developed a algorithm for this.
