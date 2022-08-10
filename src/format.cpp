#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>
using std::string;
using std::to_string;

string PadZeros(long n, int k) {
  std::ostringstream ss;
  ss << std::setw(k) << std::setfill('0') << n;
  return ss.str();
}

string Format::ElapsedTime(long seconds) {
  long h, m, s;

  h = seconds / 3600;
  m = (seconds - h * 3600) / 60;
  s = (seconds - h * 3600 - m * 60);

  return PadZeros(h, 2) + ":" + PadZeros(m, 2) + ":" + PadZeros(s, 2);
}