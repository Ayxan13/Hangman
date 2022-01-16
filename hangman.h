/*
 * Written by Ayxan Haqverdili
 * May 4 2020
 */

#ifndef HANGMAN_HPP
#define HANGMAN_HPP
#pragma once
#include <filesystem>
#include <list>
#include <set>

class Hangman {
public:
        /*
         * Call this function with a frequency-sorted wordlist to play a game
         */
        static int play(std::filesystem::path const& wordList);

private:
        /*
         * Class that represents a word and its ranking as in how common it's used.
         *
         *      str  : the word itself
         *      rank : ranking of the word
         */
        struct Word {
                std::string str;
                int rank;
        };

        // Generic return code indicating function state
        enum class ReturnCode { success, noInput, illegalInput, gameOver };

        // returns character that is believed to be likely given the list of possible words
        // and already guessed letters.
        static char getMostLikelyLetter(std::list<Word> const& list,
                std::array<bool, std::numeric_limits<char>::max()> const& alreadyGuessed,
                std::string_view current) noexcept;

        /*
         * This function filters and removes words in `list` when we guess a character `latest` in
         * positions `pos`
         */
        static void filterGuessed(std::list<Word>& list, char latest,
                std::set<int> const& pos) noexcept;

        /*
         * This function filters and removes words in `list` when we know a certain character is not in the word we're
         * trying to guess.
         */
        static void filterFailedToGuess(std::list<Word>& list, char guess) noexcept;

        /* Prints the word and indexes below it. Skips empty characters */
        static void printWord(std::string_view word) noexcept;

        /* Display currently considered words, up to `max` words. If max < 0, prints all words. */
        static void displayThinking(std::list<Word> const& possibleWords,
                int max = 10) noexcept;

        /* Attempts to clear console */
        static void clearScreen() noexcept;

        /* Prompts the user for a size between `min` and `max`. Returns -1 if fails. */
        static int getValidWordSize(int min, int max) noexcept;

        /* Reads all words with the size `size` from the file `wordList` and returns a list of them */
        static std::list<Word> readAllWordsWithSize(std::filesystem::path const& wordList, int size) noexcept;

        /* Returns alphabetic index of the character. ex. 'a' -> 0, 'b' -> 1, 'c' -> 2 */
        static int alphabeticIndex(char ch) noexcept;

        /* Converts alphabetic index to character. ex. 0 -> 'a', 1 -> 'b', 2 -> 'c' */
        static char alphabeticIndexToChar(int i) noexcept;

        /*
         * Prompts user to enter all indexes the guessed letter appears in.
         * returns:
         *      ReturnCode::success -> successfully read
         *      ReturnCode::noInput -> empty list of indexes, simply enter was pressed.
         *      ReturnCode::illegalInput -> out of index input
         */
        static ReturnCode readIndexes(std::string_view const word, std::set<int>& pos);

        /*
         * Checks is the game is over and prints the result.
         *      returns:
         *              ReturnCode::gameOver -> if the game is over.
         *              ReturnCode::success  -> if the game is not over.
         */
        static ReturnCode checkGameState(
                std::list<Word> const& possibleWords,
                std::string_view guess,
                std::size_t wrongGuessCount, 
                std::size_t correctGuessCount);
};

#endif
