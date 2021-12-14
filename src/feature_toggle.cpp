#include "feature_toggle.h"

namespace
{
std::string c_feature_toggle_file_path{"./feature_set.txt"};

std::unordered_set<std::string> parse_features(std::string_view filename)
{
  std::unordered_set<std::string> enabled_features;
  std::ifstream infile{filename};
  if (!infile.good())
  {
    std::cerr << "Could not open feature toggle file: " << c_feature_toggle_file_path << "\n";
    return enabled_features;
  }

  std::string line;
  line.reserve(256);
  while (std::getline(infile, line))
  {
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
    std::transform(line.begin(), line.end(), line.begin(), tolower);
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

bool is_feature_enabled(std::string const& feature_name)
{
  static const auto features = parse_features(c_feature_toggle_file_path);

  return features.contains(feature_name);
}
