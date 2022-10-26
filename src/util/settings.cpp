/**
 * @file settings.cpp
 * @author peng lei (plhitsz@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "settings.h"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
namespace base {
namespace util {

std::vector<std::string> split(const std::string& str,
                               const std::string& pattern) {
  std::vector<std::string> resStr;
  if ("" == str) {
    return resStr;
  }
  std::string strs = str + pattern;
  size_t pos = strs.find(pattern);
  size_t size = strs.size();

  while (pos != std::string::npos) {
    std::string x = strs.substr(0, pos);
    resStr.push_back(x);
    strs = strs.substr(pos + 1, size);
    pos = strs.find(pattern);
  }
  return resStr;
}

void Settings::read_ini(std::basic_istream<char>& stream,
                        std::map<std::string, std::string>& local) {
  typedef char Ch;
  typedef std::basic_string<Ch> Str;
  const Ch semicolon = stream.widen(';');
  const Ch hash = stream.widen('#');
  const Ch lbracket = stream.widen('[');
  const Ch rbracket = stream.widen(']');

  int64_t line_no = 0;
  std::string section;
  Str line;

  // For all lines
  while (stream.good()) {
    // Get line from stream
    ++line_no;
    std::getline(stream, line);
    if (!stream.good() && !stream.eof()) {
      break;
    }
    // If line is non-empty
    line = trim(line, stream.getloc());
    if (!line.empty()) {
      // Comment, section or key?
      if (line[0] == semicolon || line[0] == hash) {
        // Ignore comments
      } else if (line[0] == lbracket) {
        typename Str::size_type end = line.find(rbracket);
        if (end == Str::npos) {
          std::cout << " unmatched '[' " << std::endl;
          continue;
        }
        Str key = trim(line.substr(1, end - 1), stream.getloc());
        section = (key);
        if (local.find(section) != local.end()) {
          std::cout << " duplicate section name " << std::endl;
        }
      } else {
        if (section.empty()) {
          std::cout << " unmatched section " << std::endl;
          continue;
        }
        typename Str::size_type eqpos = line.find(Ch('='));
        if (eqpos == Str::npos) {
          std::cout << " unmatched '=' " << std::endl;
          continue;
        }
        if (eqpos == 0) {
          std::cout << " unmatched key " << std::endl;
          continue;
        }
        Str key = trim(line.substr(0, eqpos), stream.getloc());
        Str data = trim(line.substr(eqpos + 1, Str::npos), stream.getloc());
        std::string mixkey = section + std::string(".") + std::string(key);
        if (local.find(mixkey) != local.end()) {
          std::cout << " duplicate key name " << std::endl;
        }
        local.insert(std::make_pair(mixkey, (data)));
      }
    }
  }
}

void Settings::dump() {
  std::basic_ifstream<char> stream(SETTING_FILE_PATH);
  if (!stream) {
    throw std::runtime_error("File open failed");
  }
  sectionKv res;
  stream.imbue(std::locale());
  read_ini(stream, res);
  for (auto& m : res) {
    std::cout << m.first << "=" << m.second << std::endl;
  }
}

void Settings::write_ini(std::basic_ostream<char>& stream,
                         const std::map<std::string, std::string>& local) {
  typedef char Ch;
  std::set<std::string> print;
  for (auto& m : local) {
    auto res = split(m.first, ".");
    if (res.size() < 2) {
      // invalid data(no section or key)
      //  will not be wriiten in the file
      break;
    }
    if (print.find(res[0]) == print.end()) {
      if (print.empty()) {
        stream << Ch('[') << res[0] << Ch(']') << Ch('\n');
      } else {
        stream << Ch('\n') << Ch('[') << res[0] << Ch(']') << Ch('\n');
      }
      print.insert(res[0]);
    }
    std::string key;
    for (int i = 1; i < res.size(); i++) {
      if (!key.empty()) {
        key += ".";
      }
      key += res[i];
    }
    stream << key << Ch('=') << m.second << Ch('\n');
  }
}

}  // namespace util
}  // namespace base
