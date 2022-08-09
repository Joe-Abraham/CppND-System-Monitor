#include "format.h"

#include <string>

using std::string;
using std::to_string;

string Format::ElapsedTime(long seconds) {
  const int hour = seconds / 3600;
  const int minute = (seconds / 60) % 60;
  const int second = seconds % 60;

  return string(to_string(hour) + ":" + to_string(minute) + ":" +
                to_string(second));
}