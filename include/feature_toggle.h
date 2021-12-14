#ifndef FEATURE_TOGGLE_H
#define FEATURE_TOGGLE_H

/**
 * Returns true if a feature is enabled
 *
 * Features can be enabled/disables in a feature toggle file (feature_set.txt)
 * This file should be filled with lines of the form toggle_name=true or 
 * toggle_name=false. Spaces within the line are ignored.
 */
bool is_feature_enabled(std::string const& feature_name);

#endif // FEATURE_TOGGLE_H
