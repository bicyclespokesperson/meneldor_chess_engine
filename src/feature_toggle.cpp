#include "feature_toggle.h"

namespace rs = std::ranges;

namespace
{
std::string const c_feature_toggle_file_path{"feature_set.txt"};

std::unordered_set<std::string> parse_features(std::string const& filename)
{
  std::unordered_set<std::string> enabled_features;
  std::ifstream infile{filename};
  if (!infile.good())
  {
    std::cerr << "Could not open feature toggle file: " << std::filesystem::current_path() / c_feature_toggle_file_path
              << "\n";
    return enabled_features;
  }

  std::string line;
  line.reserve(256);
  while (std::getline(infile, line))
  {
    line.erase(rs::remove_if(line, isspace).begin(), line.end());
    rs::transform(line, line.begin(),
                  [](char c)
                  {
                    return std::tolower(c, std::locale());
                  });

    if (line.empty())
    {
      continue;
    }
    auto const eq_location = line.find('=');
    if (eq_location == std::string::npos)
    {
      std::cerr << "Unexpected line in toggle file: " << line << "\n";
      return enabled_features;
    }

    auto const toggle_name = line.substr(0, eq_location);
    auto const toggle_val = line.substr(eq_location + 1, std::string::npos) == "true";
    if (toggle_val)
    {
      enabled_features.insert(toggle_name);
    }
  }

  return enabled_features;
}
} // namespace

namespace Meneldor
{
bool is_feature_enabled(std::string const& feature_name)
{
  static auto const features = parse_features(c_feature_toggle_file_path);

  return features.contains(feature_name);
}
} // namespace Meneldor
