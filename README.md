# Meneldor Chess Engine

## Overview

Meneldor chess engine is a UCI Compilant chess engine written in C++

## Features

  - Supports full chess ruleset, including threefold repetition draws and 50 move rule
  - Magic bitboard hashing
  - Negamax search
  - Alpha beta pruning
  - Iterative deepening
  - Null move pruning
  - Quiescense search
  - Principal variation search
  - MVV/LVA move ordering
  - Zobrist hashing
  - Transposition table
  - UCI Compliance using the [Senjo UCI Adapter](https://github.com/zd3nik/SenjoUCIAdapter) library

## Building

Meneldor engine uses CMake (version 3.17+). 

#### Mac/Linux

CMake can be used directly, or a `create_build.sh` script is provided for convenience. The path to the compiler
can be set in the `create_build.sh` script. Build files are placed in the `_build` directory. The engine has 
been built with Clang 15, though other C++20 compliant compilers may work as well.

The project can be built with the `build.sh` script.

```
./create_build.sh -r # -r for release, -d for debug
./build.sh
```

#### Windows

Use CMake to generate a Visual Studio solution and build the project in Visual Studio.

## Running

There are four executables in the `bin` directory

  - `meneldor` The UCI complient executable for the engine
  - `chess_game` Plays a game of chess using standard input and output. Supports human and computer players.
  - `perft` A perft script used for correctness and performance testing
  - `tests` Runs the catch2 test executable

## Testing

Meneldor uses [Catch2](https://github.com/catchorg/Catch2) for testing via the `bin/tests` executable

```
./tests # Run all tests except for perft tests and engine performance tests (these take longer to run)
./tests [Move_generator] # Run move generator perft tests
./tests [Meneldor_engine] # Run engine performance tests
```

## License

MIT License

Copyright (c) 2021-2022 Jeremy Sigrist

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

