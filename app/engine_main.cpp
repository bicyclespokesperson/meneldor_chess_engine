#include "meneldor_engine.h"
#include "senjo/Output.h"
#include "senjo/UCIAdapter.h"

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
