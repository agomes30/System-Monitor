#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <sys/time.h>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  	
  	string line, key, value;
  	float memTotal = 0, memFree = 0, memUtilization = 0;
	std::ifstream filestream(kProcDirectory + kMeminfoFilename); // /proc/meminfo
	if (filestream.is_open()){
      while (std::getline(filestream, line)){ //iterating every line
        std::istringstream linestream(line); 
        linestream >> key >> value; //getting the fitst two tokens per line
        if (key == "MemTotal:"){ 
       		memTotal = stof(value); // string to float
        }
        if (key == "MemFree:"){
        	memFree = stof(value); // string to float 
        }
      }
    memUtilization = (memTotal - memFree) / memTotal; // in decimal, multiply by 100 for percentage
    }
  	return memUtilization;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
	string line, uptime;
  	std::ifstream filestream(kProcDirectory + kUptimeFilename);
  	if (filestream.is_open()){
    	std::getline(filestream, line);
      	std::istringstream linestream(line);
      	linestream >> uptime; // interested in the first token of the only line
      	return stol(uptime);
    }
  	return 0;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  	long totalJif = 0;
  	vector<string> cpuVal = LinuxParser::CpuUtilization();
  	for (int i = kUser_; i < kSteal_; i++){
    	totalJif += stol(cpuVal[i]);
    }
  
  	//totalJif *= sysconf(_SC_CLK_TCK); // number of clock ticks per second * nmber of seconds
  	return totalJif;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  	string line, overlook;
	std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename); //path
  	if (filestream.is_open()){
      std::getline(filestream, line);
      std::istringstream linestream(line);
      long utime;
      long stime;
      long cutime;
      long cstime;
      long starttime;
      for(int i = 0; i < 13; i++){
        linestream >> overlook;
      }
      linestream >> utime >> stime >> cutime >> cstime ;
      for(int i = 0; i < 4; i++){
        linestream >> overlook;
      }
      linestream >> starttime;
      return utime + stime + cutime + cstime + starttime; 
    }
  return 0;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
	return LinuxParser::Jiffies() - LinuxParser::IdleJiffies(); // total - idle = active
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
	long idleJiffies = 0;
  	vector<string> cpuVal = LinuxParser::CpuUtilization();
  	for (int i = kIdle_; i <= kIOwait_; i++){
    	idleJiffies += stol(cpuVal[i]);
    }
  	return idleJiffies;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string line, key;
  vector<string> cpuVal; 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()){
  	std::getline(filestream, line); // here's the first line for total utilization
   	std::istringstream linestream(line); // we break down into tokens 
    // as long as there is token in the line
    while(linestream >> key){
    	if (key != "cpu"){
        	cpuVal.push_back(key); // all the 10 values pushed as strings 
        }
    }    
  }  
  return cpuVal; 
}

string LinuxParser::Stats(string keyword){
	string line, key, value;
	std::ifstream filestream(kProcDirectory + kStatFilename);
  	if (filestream.is_open()){
      while(std::getline(filestream, line)){
      	std::istringstream linestream(line);
      	linestream >> key >> value;
      	if (key == keyword){
        	return value;
        }
     }
    }
	return value;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
	string stat = LinuxParser::Stats("processes");
  	if (stat != ""){
    	return stoi(stat);
    }
  	return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
	string processes = LinuxParser::Stats("procs_running");
  	if (processes != ""){
    	return stoi(processes);
    }
  	return 0;
  }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()){
  	std::getline(filestream, line);
    return line;
  }
  return string();
}


string LinuxParser::Status(string keyword, int pid){
	string line, key, value;
  	std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  	if (filestream.is_open()){
    	while(std::getline(filestream, line)){
          	std::replace(line.begin(), line.end(), ':', ' ');
        	std::istringstream linestream(line);
          	linestream >> key >> value;
        	if (key == keyword){
              	if (keyword == "VmSize"){
                	break;
                }
            	return value;
            }
        }
      std::stringstream ram;
      ram << stof(value) / 1024;
      return ram.str();
    }
  return "0"; 

}


// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  	return LinuxParser::Status("VmSize", pid);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
	return LinuxParser::Status("Uid", pid);
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string key, x, value, line;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()){
  	while(std::getline(filestream, line)){
    	std::replace(line.begin(), line.end(), ':', ' ');
      	std::istringstream linestream(line);
      	linestream >> key >> x >> value;
      	if (value == uid){
        	return key;
        }
    }
  }
  return string(); 
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  string line, value;
  long startTime = 0;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()){
  	std::getline(filestream, line);
    std::istringstream linestream(line);
    std::string overlook;
    for (int i = 0; i < kStartTime_; i++){
    	linestream >> overlook;
    }
    linestream >> startTime;
    struct timeval tVal;
    gettimeofday(&tVal, 0);
    //std::time_t start = std::time(0);
    std::time_t elapsed = LinuxParser::UpTime() - (startTime/sysconf(_SC_CLK_TCK));
    return elapsed;
  }
  return startTime; 


}
