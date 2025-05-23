#include "game.h"
#include "board.h"
#include "engine_player.h"
#include "meneldor_engine.h"
#include "uci_engine_player.h"
#include "user_player.h"

namespace Meneldor
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) Can't be a member variable because we want to use it in a handler
std::atomic_flag is_cancelled{};

void my_handler(int /* s */)
{
  is_cancelled.test_and_set();
}

Game::Game()
{
  init_handler_();
}

void Game::init_handler_()
{
#ifndef _WIN32
  // We still want to print out moves if the game is cancelled, so
  // set a handler for ctrl-c that sets a flag that is tested in the loop
  struct sigaction sigIntHandler{};

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);
#endif
}

bool Game::set_starting_position(std::string fen)
{
  if (!Board::from_fen(fen))
  {
    return false;
  }

  m_starting_position_fen = std::move(fen);
  return true;
}

void Game::player_vs_player()
{
  User_player white_player("White player");
  User_player black_player("Black player");
  play_game(white_player, black_player);
}

void Game::player_vs_computer(Color player_color)
{
  if (player_color == Color::white)
  {
    User_player white_player("White player");
    Engine_player black_player("Black engine");
    // auto black_player = Uci_engine_player::create("stockfish", 10);
    //auto black_player = std::make_unique<Uci_engine_player>("Meneldor", "/Users/jeremysigrist/Desktop/code_projects/chess_engine/bin/meneldor", 8);
    play_game(white_player, black_player);
  }
  else
  {
    Engine_player white_player("White engine");
    User_player black_player("Black player");
    play_game(white_player, black_player);
  }
}

void Game::computer_vs_computer()
{
  auto white_player = std::make_unique<Engine_player>("Meneldor_w");
  //auto black_player = std::make_unique<Engine_player>("Meneldor_b");
  //auto white_player = Uci_engine_player::create("laser", 10);
  //auto white_player = Uci_engine_player::create("Defenchess", 10);
  //auto white_player = Uci_engine_player::create("stockfish", 10);
  //auto white_player = Uci_engine_player::create("shallowblue", 8);
  //auto white_player = std::make_unique<Uci_engine_player>("Meneldor_white", "/Users/jeremysigrist/Desktop/meneldor_chess_engine/bin/meneldor", 8);
  auto black_player = std::make_unique<Uci_engine_player>("stockfish", "stockfish", 10);

  //auto black_player = Uci_engine_player::create("stockfish", 8);

  play_game(*white_player, *black_player);
}

bool is_drawn(Board const& board, Threefold_repetition_detector const& detector)
{
  auto const color = board.get_active_color();

  if (!(board.has_sufficient_material(color) || board.has_sufficient_material(opposite_color(color))))
  {
    return true;
  }

  if (board.get_halfmove_clock() >= 100)
  {
    return true;
  }

  return detector.is_drawn();
}

void Game::play_game(Player& white_player, Player& black_player)
{
  auto const start_time = std::chrono::system_clock::now();

  auto board = *Board::from_fen(m_starting_position_fen);
  Board::set_use_unicode_output(true);

  white_player.set_position(m_starting_position_fen);
  black_player.set_position(m_starting_position_fen);

  Threefold_repetition_detector detector;
  std::vector<std::string> move_list;
  bool white_to_move{true};
  Game_state state{Game_state::in_progress};

  std::cout << board;
  while (state == Game_state::in_progress)
  {
    std::optional<std::string> move;
    if (white_to_move)
    {
      move = white_player.get_next_move(std::cin, std::cout);
    }
    else
    {
      move = black_player.get_next_move(std::cin, std::cout);
    }

    if (move)
    {
      if (auto result = board.try_move_uci(*move))
      {
        white_player.notify(*move);
        black_player.notify(*move);
        move_list.push_back(*move);
        white_to_move = !white_to_move;
        detector.add_fen(board.to_fen());

        std::cout << "Board state after " << move_list.size() << " ply\n";
        std::cout << board;
        state = board.calc_game_state();
        if (state == Game_state::in_progress && is_drawn(board, detector))
        {
          state = Game_state::draw;
        }
      }
      else
      {
        std::cout << "\n ------------ Invalid Move: " << *move << " ------------ \n\n";
      }
    }
    else
    {
      state = (white_to_move) ? Game_state::black_victory : Game_state::white_victory;
    }

    if (is_cancelled.test())
    {
      state = Game_state::draw;
    }
  }

  auto const end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed = end_time - start_time;

  std::cout << "\n";
  if (state == Game_state::draw)
  {
    std::cout << "Game is a draw\n";
  }
  else
  {
    std::string winning_player = ((state == Game_state::black_victory) ? (black_player.get_name() + " (as black)") :
                                                                         (white_player.get_name() + " (as white)"));
    std::cout << winning_player << " is victorious!\n";
  }

  std::cout << "Game took " << elapsed.count() << " seconds\n";

  std::cout << "Starting position: " << m_starting_position_fen << "\n";
  std::cout << "Moves: ";
  for (auto const& move : move_list)
  {
    std::cout << move << " ";
  }
  std::cout << "\n";
}
} // namespace Meneldor
