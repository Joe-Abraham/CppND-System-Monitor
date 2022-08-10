#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  cmd_ = LinuxParser::Command(pid);
  user_ = LinuxParser::User(pid);
  cpu_ = LinuxParser::CpuUtilization();
  ram_ = LinuxParser::Ram(pid);
  uptime_ = LinuxParser::UpTime(pid);
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_; }

// Return the command that generated this process
string Process::Command() { return cmd_; }

// Return this process's memory utilization
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const &a) const {
  return this->CpuUtilization() < a.CpuUtilization();
}