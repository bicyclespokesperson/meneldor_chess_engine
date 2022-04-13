#include "meneldor_engine.h"
#include "senjo/Output.h"
#include "senjo/UCIAdapter.h"


constexpr bool c_log_uci_commands{true};

void log_uci_command(std::string const& cmd)
{
  constexpr auto command_log_path = "./command_log.uci";

  static bool first_call{true};

  // Open and close the file on each call so we have a command log in case of a crash
  std::ofstream command_log{command_log_path, first_call ? std::ofstream::out : std::ofstream::app};
  if (!command_log.good())
  {
      std::cerr << "Failed to open " << command_log_path << " for writing\n";
      exit(1);
  }

  if (first_call)
  {
    auto const t = std::time(nullptr);
    auto const tm = *std::localtime(&t);
    command_log << "Meneldor command log (" << std::put_time(&tm, "%m/%d/%Y %H:%M") << ")\n";
  }
  first_call = false;

  command_log << cmd << std::endl;
  command_log.close();
}

int main(int argc, char* argv[])
{
  try
  {
    Meneldor_engine engine;
    std::cout << engine.getEngineName() << " v" << engine.getEngineVersion() << " by " << engine.getAuthorName()
              << "\n";
    engine.setDebug(false);
    senjo::UCIAdapter adapter(engine);

    std::string line;
    line.reserve(16384);

    while (std::getline(std::cin, line))
    {
      if constexpr (c_log_uci_commands)
      {
        log_uci_command(line);
      }

      if (!adapter.doCommand(line))
      {
        break;
      }
    }

    return 0;
  }
  catch (const std::exception& e)
  {
    senjo::Output() << "ERROR: " << e.what();
    return 1;
  }

  return 0;
}
