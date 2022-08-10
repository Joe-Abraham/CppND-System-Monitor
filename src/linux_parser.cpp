#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

vector<string> OpenAndExtractWord(string file, string wanted_key,
                               bool first_line = false) {
  string key, line, word;
  const int word_limit = 25;  // word_limit
  vector<string> words(word_limit, "0");
  std::ifstream filestream(file);

  if (filestream.is_open()) {
    // only read first line if first_line is true
    if (first_line) {
      std::getline(filestream, line);
      std::istringstream stream(line);
      int i = 0;
      while (stream >> word && i < word_limit) {
        words[i] = word;
        i++;
      }
      return words;
    }
    // else find the desired key
    while (std::getline(filestream, line)) {
      std::istringstream sstream(line);
      sstream >> key;
      if (key == wanted_key) break;
    }
    std::istringstream stream(line);
    int j = 0;
    while (stream >> word && j < word_limit) {
      words[j] = word;
      j++;
    }
    filestream.close();
  }
  return words;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  string title, count, unit;
  string line;
  float memTotal, memAvailable;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> title >> count >> unit;
      if (title == "MemTotal:") {
        memTotal = std::stof(count);
      }
      if (title == "MemFree:") {
        memAvailable = std::stof(count);
      }
    }
  }

  return (memTotal - memAvailable) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  string uptime;
  string line;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> uptime;
    }
  }

  return std::stol(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string filename = kProcDirectory + std::to_string(pid) + kStatFilename;
  vector<string> words = OpenAndExtractWord(filename, "", true);
  return std::stol(words[13]) + std::stol(words[14]) + std::stol(words[15]) +
         std::stol(words[16]);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  // ActiveJiffies = user + nice + system + irq + softirq + steal
  vector<string> jiffies =
      OpenAndExtractWord(kProcDirectory + kStatFilename, "cpu");
  long user = std::stol(jiffies[1]);
  long nice = std::stol(jiffies[2]);
  long system = std::stol(jiffies[3]);
  long irq = std::stol(jiffies[6]);
  long softirq = std::stol(jiffies[7]);
  long steal = std::stol(jiffies[8]);
  return user + nice + system + irq + softirq + steal;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> idleJiffies =
      OpenAndExtractWord(kProcDirectory + kStatFilename, "cpu");
  long idle = std::stol(idleJiffies[4]);
  long iowait = std::stol(idleJiffies[5]);
  return idle + iowait;
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization() {
  // The utilisation of all CPU is summed up.
  long jiffies = LinuxParser::Jiffies();
  float activeJiffies = 1.0 * LinuxParser::ActiveJiffies();

  if (jiffies != 0) {
    return activeJiffies / jiffies;
  }
  return 0;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream stream(line);
      while (stream >> key >> value) {
        if (key == "processes") {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream stream(line);
      while (stream >> key >> value) {
        if (key == "procs_running") {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }
