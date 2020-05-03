#include "hangman.h"
#include "hangmanFrames.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <set>
#include <sstream>

int Hangman::play(std::filesystem::path const& wordList) {
        auto const size = getValidWordSize(1, 50);

        if (size <= 0) // failed to read size
                return -1;

        auto possibleWords = readAllWordsWithSize(wordList, size);

        std::string guess(size, '\0');
        std::size_t wrongGuessCount{}, correctGuessCount{};

        std::array<bool, std::numeric_limits<char>::max()> alreadyGuessed{}; // chars we've already guessed

        if (std::cin.peek() == '\n') // prepare for unformatted input
                std::cin.ignore();

        while (true) {
                clearScreen();
                printWord(guess);
                std::puts(hangmanFrames[wrongGuessCount]);

                if (checkGameState(possibleWords, guess, wrongGuessCount,
                        correctGuessCount) == ReturnCode::gameOver)
                        break;

                displayThinking(possibleWords);
                auto const chGuess =
                        getMostLikelyLetter(possibleWords, alreadyGuessed, guess);

                std::printf("My guess: %c\n"
                        "If I guessed right, enter indexes: (ex. 1 2 3)\n"
                        "Else, just press Enter\n",
                        std::toupper(static_cast<unsigned char>(chGuess)));

                std::set<int> pos;
                switch (readIndexes(guess, pos)) {
                case ReturnCode::success:
                        correctGuessCount += pos.size();
                        std::for_each(pos.cbegin(), pos.cend(),
                                [&guess, chGuess](auto const i) { guess[i] = chGuess; });

                        filterGuessed(possibleWords, chGuess, pos);
                        alreadyGuessed[alphabeticIndex(chGuess)] = true;
                        break;

                case ReturnCode::noInput:
                        ++wrongGuessCount;
                        filterFailedToGuess(possibleWords, chGuess);
                        break;

                case ReturnCode::illegalInput:
                        std::printf("Illegal Index\nPress Enter to continue... ");
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        break;

                default:
                        break;
                }
        }

        return 0;
}

int Hangman::alphabeticIndex(char const ch) noexcept {
        return std::tolower(static_cast<unsigned char>(ch)) - 'a';
}

char Hangman::alphabeticIndexToChar(int const i) noexcept {
        return static_cast<char>('a' + i);
}

Hangman::ReturnCode Hangman::readIndexes(std::string_view const word,
        std::set<int>& pos) {
        std::string line;
        std::getline(std::cin, line);
        std::istringstream is{ line };

        for (int i; is >> i; pos.insert(i - 1)) {
                if (static_cast<size_t>(i) - 1 >= word.size() || word[i - 1] != '\0') {
                        return ReturnCode::illegalInput;
                }
        }
        if (pos.empty())
                return ReturnCode::noInput;
        return ReturnCode::success;
}

Hangman::ReturnCode
Hangman::checkGameState(std::list<Word> const& possibleWords,
        std::string_view guess, std::size_t wrongGuessCount,
        std::size_t correctGuessCount) {
        std::size_t constexpr maxGuess =
                sizeof(hangmanFrames) / sizeof(*hangmanFrames) - 1;

        if (wrongGuessCount == maxGuess) {
                std::puts("You win!");
                displayThinking(possibleWords, -1);
                return ReturnCode::gameOver;
        }
        if (correctGuessCount == guess.size()) {
                std::puts("I win!");
                return ReturnCode::gameOver;
        }
        if (possibleWords.empty()) {
                std::printf("Out of guesses\n"
                        "Looks like my word list does not have that word\n");
                return ReturnCode::gameOver;
        }

        return ReturnCode::success;
}

char Hangman::getMostLikelyLetter(std::list<Word> const& list,
        std::array<bool, std::numeric_limits<char>::max()> const& alreadyGuessed,
        std::string_view const current) noexcept {

        std::array<double, std::numeric_limits<char>::max()> words{};

        for (auto const& word : list) {

                // no need to over-emphasize multiple use in the same word
                std::array<bool, std::numeric_limits<char>::max()> charsInThisWord{};

                for (std::size_t i = 0; i != current.size(); ++i) {
                        if (current[i] == '\0') {
                                auto const ch = word.str[i];
                                if (auto const charIndex = alphabeticIndex(ch);
                                        !alreadyGuessed[charIndex] && 
                                        !charsInThisWord[charIndex])
                                {

                                        // frequency
                                        words[charIndex] += 1.0 / word.rank;

                                        charsInThisWord[charIndex] = true;
                                }
                        }
                }
        }
        auto const max = std::max_element(words.cbegin(), words.cend());

        return alphabeticIndexToChar(max - words.cbegin());
}

void Hangman::filterGuessed(std::list<Word>& list, char const latest,
        std::set<int> const& pos) noexcept {
        list.remove_if([latest, &pos](Word const& word) {
                for (std::size_t i = 0; i != word.str.size(); ++i) {
                        if (pos.find(i) == pos.end()) {
                                if (word.str[i] == latest)
                                        return true;
                        }
                        else {
                                if (word.str[i] != latest)
                                        return true;
                        }
                }
                return false;
                });
}

void Hangman::filterFailedToGuess(std::list<Word>& list,
        const char guess) noexcept {
        list.remove_if([guess](Word const& word) {
                return word.str.find(guess) != std::string::npos;
                });
}

void Hangman::printWord(std::string_view const word) noexcept {
        for (size_t i = 0; i != word.size(); ++i) {
                if (word[i] == '\0')
                        std::printf("_  ");
                else
                        std::printf("%c  ", word[i]);
        }

        std::putchar('\n');

        for (size_t i = 0; i != word.size(); ++i) {
                std::printf("%-2d ", i + 1);
        }

        std::putchar('\n');

}

void Hangman::displayThinking(std::list<Word> const& possibleWords,
        int max) noexcept {
        if (auto const sz = static_cast<int>(possibleWords.size()); sz) {
                if (max < 0)
                        max = sz;

                auto const loopCount = std::min(sz, max);
                auto i = 0;

                std::printf("Thinking of: ");
                for (auto const& word : possibleWords) {
                        if (i++ >= loopCount)
                                break;

                        std::printf("%s, ", word.str.c_str());
                }

                if (loopCount < sz)
                        std::printf("...");

                std::printf("\n\n");
        }
}

void Hangman::clearScreen() noexcept {
#if defined(_WIN32)
        std::system("cls");
#elif defined(__linux__ )
        std::system("clear");
#endif // os test
}

int Hangman::getValidWordSize(int const min, int const max) noexcept {
        int size;
        do {
                std::printf("Length: ");
                if (!(std::cin >> size)) {
                        std::puts("Failed to read length");
                        return -1;
                }
                if (size >= min && size <= max) {
                        return size;
                }
                std::puts("Illegal Size");
        } while (true);
}

std::list<Hangman::Word>
Hangman::readAllWordsWithSize(std::filesystem::path const& wordList,
        int const size) noexcept {
        std::list<Word> possibleWords;
        std::ifstream inputFile(wordList);
        auto rank = 1;
        for (Word word; inputFile >> word.str; ++rank) {
                if (static_cast<int>(word.str.size()) == size) {
                        word.rank = rank;
                        possibleWords.push_back(std::move(word));
                }
        }
        return possibleWords;
}
