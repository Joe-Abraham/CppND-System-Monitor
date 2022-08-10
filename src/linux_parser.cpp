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

/**
 * @brief Helper function : Open the required file and extract the required
 * words
 *
 * @param file        File that needs to be opened
 * @param wanted_key  Key we are looking for
 * @param first_line  Whether the first line needs to be taken
 *                    to consideration or not!
 * @return array of words
 */
vector<string> OpenAndExtractWords(string file, string wanted_key,
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

/**
 * @brief Helper function : returns word correspoing to the
 *        wanted key.
 *
 * @param file
 * @param wanted_key
 * @return string
 */
string OpenAndExtractWord(string file, string wanted_key) {
  string key, line, value;
  std::ifstream filestream(file);

  while (std::getline(filestream, line)) {
    std::istringstream sstream(line);
    sstream >> key >> value;

    if (key == wanted_key) return value;
  }
  return "0";
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
  string file = kProcDirectory + kMeminfoFilename;
  float memTotal = std::stof(OpenAndExtractWord(file, "MemTotal:"));
  float memAvailable = std::stof(OpenAndExtractWord(file, "MemFree:"));

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
  vector<string> words = OpenAndExtractWords(filename, "", true);
  return std::stol(words[13]) + std::stol(words[14]) + std::stol(words[15]) +
         std::stol(words[16]);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  // ActiveJiffies = user + nice + system + irq + softirq + steal
  vector<string> jiffies =
      OpenAndExtractWords(kProcDirectory + kStatFilename, "cpu");
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
      OpenAndExtractWords(kProcDirectory + kStatFilename, "cpu");
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
  return std::stoi(
      OpenAndExtractWord(kProcDirectory + kStatFilename, "processes"));
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return std::stoi(
      OpenAndExtractWord(kProcDirectory + kStatFilename, "procs_running"));
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmd;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);

  if (filestream.is_open()) {
    std::getline(filestream, cmd);
    filestream.close();
  }
  return cmd;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string filename = kProcDirectory + std::to_string(pid) + kStatusFilename;
  long memKb = std::stol(OpenAndExtractWord(filename, "VmRSS:"));
  float memMb = memKb / 1024;
  long memTrunc = std::trunc(memMb);
  return std::to_string(memTrunc);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string filename = kProcDirectory + std::to_string(pid) + kStatusFilename;
  return OpenAndExtractWord(filename, "Uid:");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string user, line, pwd, userID;
  std::ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> pwd >> userID;
      if (userID == uid) break;
    }
    filestream.close();
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string pid_string = std::to_string(pid);
  string filename = kProcDirectory + pid_string + kStatFilename;
  long starttime =
      std::stol(OpenAndExtractWords(filename, "", true)[21]);  // in jiffies
  return LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
}
