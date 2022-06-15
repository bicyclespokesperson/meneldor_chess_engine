#include "meneldor_engine.h"
#include "senjo/Output.h"
#include "senjo/UCIAdapter.h"

constexpr bool c_log_uci_commands{true};

void log_uci_command(std::string const& cmd)
{
  std::filesystem::path const command_log_path{"./command_log.uci"};

  static bool first_call{true};

  // Open and close the file on each call so we have a command log in case of a crash
  std::ofstream command_log{command_log_path, first_call ? std::ofstream::out : std::ofstream::app};
  if (!command_log.good())
  {
    std::cerr << "Failed to open " << std::filesystem::absolute(command_log_path) << " for writing\n";
    exit(1);
  }

  if (first_call)
  {
    auto const t = std::time(nullptr);
    auto const tm = *std::localtime(&t);
    command_log << "# Meneldor command log (" << std::put_time(&tm, "%m/%d/%Y %H:%M") << ")\n";
  }
  first_call = false;

  command_log << cmd << std::endl;
  command_log.close();
}

int main(int argc, char* argv[])
{
  std::ifstream infile;

  if (argc == 2)
  {
    if (!strcmp(argv[1], "-h"))
    {
      std::cerr << "Usage: " << argv[0] << " [opt: uci command file]\n";
      return 2;
    }
    else
    {
      std::filesystem::path commands_path{argv[1]};
      infile.open(commands_path);
      if (!infile.good())
      {
        std::cerr << "Failed to open file: " << std::filesystem::absolute(commands_path) << "\n";
        return 1;
      }
    }
  }

  std::istream* stream = infile.is_open() ? &infile : &std::cin;

  try
  {
    Meneldor_engine engine;
    std::cout << engine.getEngineName() << " v" << engine.getEngineVersion() << " by " << engine.getAuthorName()
              << "\n";
    engine.setDebug(false);
    senjo::UCIAdapter adapter{engine};

    std::string line;
    line.reserve(16384);

    while (std::getline(*stream, line))
    {
      std::transform(line.begin(), line.end(), line.begin(),
                     [](char c)
                     {
                       return std::tolower(c, std::locale());
                     });

      if constexpr (c_log_uci_commands)
      {
        log_uci_command(line);
      }

      // Ignore lines beginning with '#'
      if (auto index = line.find_first_not_of(" \t"); index != std::string::npos && line[index] != '#')
      {
        if (!adapter.doCommand(line))
        {
          break;
        }
      }
    }

    return 0;
  }
  catch (std::exception const& e)
  {
    senjo::Output() << "ERROR: " << e.what();
    return 1;
  }

  return 0;
}
