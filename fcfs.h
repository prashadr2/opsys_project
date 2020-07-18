#ifndef __fcfs_h
#define __fcfs_h

//cpp includes
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <list>
//c includes
#include <stdlib.h>
#include <math.h>
//custom includes
#include "process.h"

void fcfs(std::ofstream& outfile, const std::vector<Process>& p, const int tcs);
void printqueue(std::list<Process>& printer);
void printcpufin(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinFCFS(std::list<Process>& waiting, std::list<Process>& ready, int t);

void printqueue(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
  std::cout << "[Q";
  if(printer.size() == 0){
      std::cout << " <empty>]" << std::endl;
      return;
  }
  for(std::list<Process>::iterator z = printer.begin(); z != printer.end(); z++) {
      std::cout << ' ' << z->getname();
#ifdef DEBUG_MODE
    std::cout << "-->waittime: " << z->getcurrentwait() << ", ";
#endif
  }
  std::cout << "]" << std::endl;
}

void printcpufin(Process* incpu, int t, const int tcs, std::list<Process>& ready){
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueue(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueue(ready);
}

void printiofinFCFS(std::list<Process>& waiting, std::list<Process>& ready, int t){
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue ";
    waiting.front().movenextwait();
    ready.push_back(Process(waiting.front()));
    waiting.pop_front();
    printqueue(ready);
}

//arrival time, cpu start, cpu end(io start), io end

void fcfs(std::ofstream& outfile, const std::vector<Process>& p, const int tcs){
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //time
  	std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
    Process* incpu = NULL; //needs to be a pointer so we can dynammically call the process deep copy constructor
    auto compname = [](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();};
    auto comparrival = [](Process p1, Process p2) -> bool {if(p1.getarrivaltime() == p2.getarrivaltime()) return p1.getname() < p2.getname(); else return p1.getarrivaltime() < p2.getarrivaltime();};
    auto compcurwait = [](Process p1, Process p2) -> bool {if(p1.getcurrentwait() == p2.getcurrentwait()) return p1.getname() < p2.getname(); else return p1.getcurrentwait() < p2.getcurrentwait();};
    std::vector<int> burstavg; //push back to here each burst time
    std::vector<int> waitavg; //push back to here each wait time (io block)
    for(Process z : p) unarrived.push_back(Process(z)); //use deep copy to not modify the input
    unarrived.sort(compname); //sort by names for printing first
    
    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU bursts" << std::endl;
    }
    std::cout << "time 0ms: Simulator started for FCFS ";
    printqueue(ready);

  	unarrived.sort(comparrival); //sorting for actual program
#ifdef DEBUG_MODE
    for(auto const& debugs : ready) std::cout << "DEBUG READYQUEUE --> Process " << debugs.getname() << " (arrival time " << debugs.getarrivaltime() << " ms)" << std::endl;
#endif
    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){ //if any statement is true we have work to do still...
        if(waiting.size() > 1) waiting.sort(compcurwait);
        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -10000, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))

         if(waitingtime <= -10000){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueport(ready);
            if(ready.empty()) {
                t += tcs;
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
            } else {
                t += tcs /2;
                for(auto& r : ready) r.addwaittime(tcs/2);
            }
            garbage.push_back(Process(waiting.front()));
            waiting.pop_front();
            continue;
        }


        if(arrivaltime == t) {

        }

        t++;
    }
    t-=tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for FCFS ";
    printqueue(ready);
}
#endif