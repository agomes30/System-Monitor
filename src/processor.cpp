#include "processor.h"
#include "linux_parser.h"
#include<unistd.h>

#include <iostream>


// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  
  	idle = LinuxParser::IdleJiffies();
 	active = LinuxParser::ActiveJiffies();
  
  	long pTotal = pIdle + pActive;
  	long Total = idle + active;
 
  	long deltaTotal = Total - pTotal;
  	long deltaIdle = idle - pIdle;
  	pIdle = idle;
  	pActive = active;
  	return (float)(deltaTotal - deltaIdle) / deltaTotal; 
  
}