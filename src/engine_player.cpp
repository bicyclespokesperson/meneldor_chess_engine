#include "engine_player.h"
#include "utils.h"

Engine_player::Engine_player(std::string name) : Player(std::move(name))
{
  m_engine.initialize();
}

std::optional<std::string> Engine_player::get_next_move(std::istream& /* in */, std::ostream& out)
{
  out << get_name() << " thinking\n";
  senjo::GoParams params;
  params.depth = 6;
  auto const start = std::chrono::system_clock::now();
  auto const engine_move = m_engine.go(params, nullptr);
  auto const end = std::chrono::system_clock::now();
  std::chrono::duration<double> const elapsed_time = end - start;
  auto search_stats = m_engine.getSearchStats();

  out << "Engine played " << engine_move << " after thinking for " << std::fixed << std::setprecision(2)
      << format_with_commas(elapsed_time.count()) << " seconds and searching " << format_with_commas(search_stats.nodes)
      << " nodes (" << format_with_commas(search_stats.nodes / elapsed_time.count()) << " nodes/sec)\n";

  return engine_move;
}

void Engine_player::notify(std::string const& move)
{
  m_engine.makeMove(move);
}

bool Engine_player::set_position(std::string_view fen)
{
  return m_engine.setPosition(std::string{fen});
}

void Engine_player::reset()
{
  m_engine.resetEngineStats();
  m_engine.initialize();
}
