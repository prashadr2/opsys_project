#ifndef __fcfs_h
#define __fcfs_h

//cpp includes
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <queue>
#include <list>
//c includes
#include <stdlib.h>
#include <math.h>
//custom includes
#include "process.h"

void fcfs(std::ofstream& outfile, std::vector<Process>& p);
void printqueue(std::list<Process>& printer);

void printqueue(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
  std::cout << "[Q";
  for(std::list<Process>::iterator z = printer.begin(); z != printer.end(); z++) std::cout << ' ' << z->getname();
  std::cout << "]" << std::endl;
}

void fcfs(std::ofstream& outfile, std::vector<Process>& p){
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //time
  	
  	std::list<Process> unarrived;
    for(int z = 0; z < p.size(); z++) unarrived.push_back(p[z]);
 	std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
  
  	unarrived.sort([](Process p1, Process p2) -> bool {return p1.getarrivaltime() < p2.getarrivaltime();});
   	t = unarrived.front().getarrivaltime();
  	ready.push_back(unarrived.front());
    unarrived.pop_front();
  	for(std::list<Process>::iterator zit = unarrived.begin(); zit != unarrived.end(); zit++){
     	if(zit->getarrivaltime() == t) ready.push_back(*zit);
    }
    ready.sort([](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();});
    
    std::cout << "time <0>ms: Start of simulation [Q ]" << std::endl;
  	
  	while(!ready.empty() && !waiting.empty() && !unarrived.empty()){
      if(unarrived.front().getarrivaltime() < waiting.front().getcurrentwait() //next "interesting" event is process arrival
          && unarrived.front().getarrivaltime() < ready.front().getarrivaltime()) {
        
        int gap = t - unarrived.front().getarrivaltime();
        t = unarrived.front().getarrivaltime();
    	ready.push_back(unarrived.front());
        unarrived.pop_front();
        std::cout << "time <" << t << ">ms: Process \"" << ready.back().getname() << "\" arrived ";
        printqueue(ready);
        
        //decrement all the other times by gap
      } else if(waiting.front().getcurrentwait() < ready.front().getcurrentruntime()) { //next "interesting" event is finishing I/O
      	ready.push_back(waiting.front());
        waiting.pop_front();
      } else { //next "interesting" event is CPU burst completion/termination
        if (ready.front().getcurrentwait() == -1 ){ //process terminates
          std::cout << "time <" << t << ">ms: Process finishes using the CPU" << std::endl;
        	std::cout << "time <" << t << ">ms: Process Terminates" << std::endl;
        } else {
          waiting.push_back(ready.front()); //process in cpu moved to waiting state for I/O
          ready.pop_front();
          std::cout << "time <" << t << ">ms: Process finishes using the CPU" << std::endl;
           //t = t + context switch
          std::cout << "time <" << t << ">ms: Process starts performing I/O" << std::endl; //waiting.back()
          std::cout << "time <" << t << ">ms: Process starts using the CPU" << std::endl; //ready.front()
        }
      }
    }
}

//rm -rf .git/refs/remotes/origin/
//./a.o 5 231313123213123123 0.001343232423 545 5 5 5
//g++ -D DEBUG_MODE -o a.o driver.cpp
#endif