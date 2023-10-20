cmake_minimum_required(VERSION 3.12)

project(Hangman)

set(CMAKE_CXX_STANDARD 17)

configure_file(Wordlist.txt ${CMAKE_BINARY_DIR}/Wordlist.txt COPYONLY)

add_executable(hangman
    main.cpp
    hangman.cpp
    hangmanFrames.h
    hangman.h
)
