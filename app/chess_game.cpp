/*
 * File:   chess.cpp
 * Author: 1697518
 *
 * This is the main method for the chess game.
 * Created on March 24, 2013, 5:02 PM
 */

#include "bitboard.h"
#include "board.h"
#include "game.h"
#include "meneldor_engine.h"
#include "move_generator.h"
#include "my_assert.h"
#include "player.h"
#include "transposition_table.h"
#include "utils.h"
#include "zobrist_hash.h"

void print_bitboard_with_squares(std::vector<std::string> const& squares)
{
  Bitboard bb;
  for (auto const& str : squares)
  {
    if (auto sq = Coordinates::from_str(str))
    {
      bb.set_square(*sq);
    }
    else
    {
      std::cout << "Invalid square: " << str << "\n";
    }
  }
  std::cout << bb << "\nhex: " << bb.hex_str() << "\n";
}

//NOLINTNEXTLINE // Match signature of main function
void run_chess_game(int argc, char* argv[])
{
  if (argc > 3)
  {
    // h -> human, c -> computer
    std::cerr << "Usage: chess_game [hh/hc/ch/cc] [optional: fen]\n";
    exit(-1);
  }

  Game game;
  if (argc == 3)
  {
    game.set_starting_position(argv[2]);
  }

  // First player is white, second is black
  std::string param{"hc"};
  if (argc >= 2)
  {
    param = argv[1];
  }

  if (param == "hh")
  {
    game.player_vs_player();
  }
  else if (param == "hc")
  {
    game.player_vs_computer(Color::white);
  }
  else if (param == "ch")
  {
    game.player_vs_computer(Color::black);
  }
  else if (param == "cc")
  {
    game.computer_vs_computer();
  }
  else
  {
    // h -> human, c -> computer
    std::cerr << "Usage: chess [hh/hc/ch/cc]\n";
    exit(-1);
  }
}

void benchmark()
{
  uint64_t val = 0x8100000000000081;

  volatile size_t total{0};
  size_t const iterations = 1'000'000'000;
  {
    auto const start = std::chrono::system_clock::now();
    for (size_t i{0}; i < iterations; ++i)
    {
      Bitboard b{val};
      total = total + b.occupancy();
    }
    auto const end = std::chrono::system_clock::now();
    std::chrono::duration<double> const elapsed_time = end - start;
    std::cout << "occupancy time: " << elapsed_time.count() << "\n";
  }

  {
    auto const start = std::chrono::system_clock::now();
    for (size_t i{0}; i < iterations; ++i)
    {
      Bitboard b{val};
      total = total + b.occupancy();
    }
    auto const end = std::chrono::system_clock::now();
    std::chrono::duration<double> const elapsed_time = end - start;
    std::cout << "Occupancy builtin time: " << elapsed_time.count() << "\n";
    std::cout << total << std::endl;
  }
}

/**
 * Play the chess game.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
#if 1
  run_chess_game(argc, argv);
#else
  benchmark();
#endif

  return 0;
}
