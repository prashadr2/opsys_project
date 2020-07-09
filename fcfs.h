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

void fcfs(std::ofstream& outfile, const std::vector<Process>& p);
void printqueue(std::list<Process>& printer);

void printqueue(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
  std::cout << "[Q";
  if(printer.size() == 0){
      std::cout << " <empty>]" << std::endl;
      return;
  }
  for(std::list<Process>::iterator z = printer.begin(); z != printer.end(); z++) std::cout << ' ' << z->getname();
  std::cout << "]" << std::endl;
}

void fcfs(std::ofstream& outfile, const std::vector<Process>& p){
    outfile << "Algorithm FCFS\n"; //write to file test... working

    int t = 0; //time
  	std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;

    for(Process z : p) unarrived.push_back(Process(z)); //use deep copy to not modify the input
    unarrived.sort([](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();}); //sort by names for printing first
   
    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU bursts" << std::endl;
    }
    std::cout << "time <0>ms: Simulator started for FCFS ";
    printqueue(ready);

  	unarrived.sort([](Process p1, Process p2) -> bool {return p1.getarrivaltime() < p2.getarrivaltime();}); //sorting for actual program.
   	t = unarrived.front().getarrivaltime();
  	ready.push_back(unarrived.front());
    unarrived.pop_front();
    for(std::list<Process>::iterator zit = unarrived.begin(); zit != unarrived.end(); zit++){
     	if(zit->getarrivaltime() == t) ready.push_back(*zit);
    }
    ready.sort([](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();}); //ready is forced to have the same arrival time at this point...
  	
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
#endif