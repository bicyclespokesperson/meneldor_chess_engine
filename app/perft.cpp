#include "board.h"
#include "move_generator.h"
#include "utils.h"

int main(int argc, char* argv[])
{
  if (argc != 2 && argc != 3)
  {
    std::cerr << "Usage: perft [depth] [fen]\n";
    exit(-1);
  }

  int depth{0};
  try
  {
    depth = std::stoi(argv[1]);
  }
  catch (std::invalid_argument const& err)
  {
    std::cerr << err.what();
    exit(-1);
  }

  std::string starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  if (argc == 3)
  {
    starting_position = argv[2];
  }

  auto board = Board::from_fen(starting_position);
  if (!board)
  {
    std::cerr << "Invalid fen string\n";
  }

  auto const start = std::chrono::system_clock::now();
  std::atomic_flag is_cancelled{false};
  auto result = Move_generator::perft(depth, *board, is_cancelled);
  auto const end = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed = end - start;
  auto const elapsed_seconds = elapsed.count();

  std::cout << "perft(" << std::to_string(depth) << ") = " << std::to_string(result) << "\n";
  std::cout << "Elapsed time: " << std::to_string(elapsed_seconds) << " seconds\n";
  std::cout << "Nodes/sec: " << format_with_commas(result / elapsed_seconds) << "\n";

  return 0;
}
