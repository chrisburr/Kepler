#ifndef TBCONDFILE_H
#define TBCONDFILE_H

#include <fstream>
#include <sstream>
#include <iostream>

#include "TbKernel/TbBufferedFile.h"

/* @class TbCondFile TbCondFile.h
 *
 * Interface classes for dealing with conditions files,
 * both locally and via more general TbBufferedFiles
 *
 */

class TbCondFile : public TbBufferedFile<100000, char> {
 public:
  /// Constructor
  TbCondFile(const std::string& filename) : TbBufferedFile(filename) {}

  bool split(std::stringstream& ss, const char delim) {
    std::string item;
    std::getline(ss, item, delim);
    return item.empty();
  }

  bool getLine(std::string& line) {
    line = "";
    char tmp('a');
    while (tmp != '\n' && !TbBufferedFile::eof()) {
      tmp = getNext();
      if (tmp != '\n') line += tmp;
    }
    const size_t p = line.find_first_not_of(' ');
    return (line.empty() || line[0] == '#' || p == std::string::npos) ? false
                                                                      : true;
  }

  template <typename Type>
  Type custom_cast(const std::string& ref);
  template <typename Type, typename... Types>
  bool split(std::stringstream& ss, const char delim, Type& val,
             Types&... rest);
  template <typename... Types>
  bool split(const std::string& s, const char delim, Types&... rest);
};

template <typename... Types>
bool TbCondFile::split(const std::string& s, const char delim, Types&... rest) {
  std::stringstream ss(s);
  return split(ss, delim, rest...);
}

template <typename Type, typename... Types>
bool TbCondFile::split(std::stringstream& ss, const char delim, Type& val,
                       Types&... rest) {
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (item == "") continue;
    val = custom_cast<Type>(item);
    return split(ss, delim, rest...);
  }
  return true;
}

#endif
