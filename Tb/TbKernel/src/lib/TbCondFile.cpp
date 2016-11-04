#include "TbKernel/TbCondFile.h"

template <>
double TbCondFile::custom_cast(const std::string& ref) {
  return std::stod(ref);
}
template <>
int TbCondFile::custom_cast(const std::string& ref) {
  return std::stoi(ref);
}
template <>
std::string TbCondFile::custom_cast(const std::string& ref) {
  return ref;
}
template <>
unsigned int TbCondFile::custom_cast(const std::string& ref) {
  return std::stoi(ref);
}

template <>
float TbCondFile::custom_cast(const std::string& ref) {
  return std::stof(ref);
}
