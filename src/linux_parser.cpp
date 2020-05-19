#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
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

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, desc, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> desc >> kernel;
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
  std::ifstream meminfo(kProcDirectory + kMeminfoFilename);
  string line, key;
  int memtotal, memfree, memavail, buffers;
  if (meminfo.is_open()) {
    for (int i = 0; i < 4; i++) {
      std::getline(meminfo, line);
      std::istringstream linestream(line);
      switch(i) {
        case 0: linestream >> key >> memtotal; break;
        case 1: linestream >> key >> memfree; break;
        case 2: linestream >> key >> memavail; break;
        case 3: linestream >> key >> buffers; break;
      }
    }
  }
  float utilization = 1.0f * (memtotal - memfree) / memtotal;
  return utilization;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  float uptime, idle;
  std::ifstream memoryFile(kProcDirectory + kUptimeFilename);
  string line;
  if (memoryFile.is_open())
  {
    std::getline(memoryFile, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idle;
  }
  return uptime;
}

// Read and return CPU utilization
float LinuxParser::CpuUtilization() {
  std::ifstream stat (kProcDirectory + kStatFilename);

  string line, token;
  long usertime, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  if (stat.is_open()) {
    getline(stat, line);
    std::istringstream ss (line);
    ss >> token >> usertime >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    usertime = usertime - guest;
    long nicetime = nicetime - guest_nice;
    long idlealltime = idle + iowait;
    long systemalltime = system + irq + softirq;
    long virtalltime = guest + guest_nice;
    long totaltime = usertime + nicetime + systemalltime + idlealltime + steal + virtalltime;
    float usage = (1.0f * totaltime - idlealltime) / totaltime;
    return usage;
  }
  return 0;
}

float LinuxParser::CpuUtilization(int pid) {
  std::ifstream file(kProcDirectory + to_string(pid) + kStatFilename);
  string line;
  long int utime, stime, cutime, cstime, starttime;

  if (file.is_open()) {
    getline(file, line);
    std::istringstream ss (line);

    for (int i = 0; i < 13; i++) {
      string token;
      ss >> token;
    }

    ss >> utime;
    ss >> stime;
    ss >> cutime;
    ss >> cstime;

    for (int i = 0; i < 4; i++) {
      string token;
      ss >> token;
    }

    ss >> starttime;
  }

  long Hertz = sysconf(_SC_CLK_TCK);
  long total_time = utime + stime;
  long seconds = UpTime() - (starttime / Hertz);
  float cpu_usage = 100.0f * ((1.0f * total_time / Hertz) / seconds);
  return cpu_usage;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stat (kProcDirectory + kStatFilename);
  int total = 0;
  string line, key;
  if (stat.is_open()) {
    while (std::getline(stat, line)) {
      std::istringstream sline (line);
      sline >> key;
      if (key == "processes") {
        sline >> total;
      }
    }
  }
  return total;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stat (kProcDirectory + kStatFilename);
  int total = 0;
  string line, key;
  if (stat.is_open()) {
    while (std::getline(stat, line)) {
      std::istringstream sline (line);
      sline >> key;
      if (key == "procs_running") {
        sline >> total;
      }
    }
  }
  return total;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream file (kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  if (file.is_open()) {
    getline(file, line);
  }
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  std::ifstream file (kProcDirectory + to_string(pid) + kStatusFilename);
  string line, key;
  long memoryUsage = - 1;
  
  if (file.is_open()) {
    while (getline(file, line)) {
      std::istringstream ss (line);
      ss >> key;
      if (key == "VmSize:") {
        ss >> memoryUsage;
        break;
      }
    }
  }
  long memoryMegaBytes = memoryUsage / 1024;
  return to_string(memoryUsage >= 0 ? memoryMegaBytes : 0);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::ifstream file (kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line, key, uid;
  if (file.is_open()) {
    while (getline(file, line)) {
      std::istringstream sline (line);
      sline >> key;
      if (key == "Uid:") {
        sline >> uid;
      }
    }
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  std::ifstream file (kPasswordPath);

  string line, user;
  if (file.is_open()) {
    while (getline(file, line)) {
      if (line.find(uid + ":") != string::npos) {
        std::istringstream ss(line);
        getline(ss, user, ':');
      }
    }
  }

  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream file(kProcDirectory + to_string(pid) + kStatFilename);
  string line;
  long uptimeClockTicks = 0;

  if (file.is_open()) {
    std::istringstream ss (line);

    for (int i = 0; i < 21; i++) {
      string token;
      ss >> token;
    }

    ss >> uptimeClockTicks;
  }

  long uptime = uptimeClockTicks / sysconf(_SC_CLK_TCK);
  return uptime;
}