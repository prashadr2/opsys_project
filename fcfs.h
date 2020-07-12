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

//arrival time, cpu start, cpu end(io start), io end

void fcfs(std::ofstream& outfile, const std::vector<Process>& p){
    outfile << "Algorithm FCFS\n"; //write to file test... working

    int t = 0; //time
  	std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
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
   	// t = unarrived.front().getarrivaltime();
  	// ready.push_back(unarrived.front());
    // unarrived.pop_front();
    // for(auto const& z : unarrived) if(z.getarrivaltime() == t) ready.push_back(z);
    // ready.sort(compname); //ready is forced to have the same arrival time at this point...
#ifdef DEBUG_MODE
    for(auto const& debugs : ready) std::cout << "DEBUG READYQUEUE --> Process " << debugs.getname() << " (arrival time " << debugs.getarrivaltime() << " ms)" << std::endl;
#endif
    while(!ready.empty() || !waiting.empty() || !unarrived.empty()){ //if any statement is true we have work to do still...
        if(waiting.size() > 1) waiting.sort(compcurwait); //update wait ordering...
        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime =  !ready.empty() ? ready.front().getcurrentruntime() : -1;
        if(waitingtime == -2){
            std::cout << "time " << t << ": Process " << waiting.front().getname() << " terminated "; 
            printqueue(ready);
            waiting.pop_front();
        }
        // std::cout << waitingtime << ' ' << arrivaltime << ' ' << cputime << std::endl;
        if(arrivaltime == -1){

        } else if(waitingtime == -1){
            //compare arrivaltime and cpu time...
            if(cputime == -1){ //if theres no cpu times to compare... we have an arrival
                t = arrivaltime;
                while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                    ready.push_back(unarrived.front());
                    std::cout << "time <" << t << ">: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    printqueue(ready);
                    unarrived.pop_front();
                }
            } else {//compare arrivaltime and cpu time...
                if(arrivaltime > cputime){
                    int gap = t - cputime;

                } else {

                }
            }

        } else if(cputime == -1){
        
        } else { //if we get here that means all 3 queues are occupied... god help us
            if(arrivaltime < waitingtime && arrivaltime < cputime){ //process arrival
                t = arrivaltime;
                ready.push_back(unarrived.front());
                std::cout << "time <" << t << ">: Process " << unarrived.front().getname() << " arrivedl added to ready queue ";
                printqueue(ready);
                unarrived.pop_front();
            }
            else if(waitingtime < arrivaltime && waitingtime < cputime){ //process finishes I/O

            }
            else{ //process completes CPU burst

            }
        }
    }
    // while(!ready.empty() || !waiting.empty() || !unarrived.empty()){ //if any statement is true we have work to do still...
    //     if(waiting.size() > 1) waiting.sort(compcurwait); //update wait ordering...
    //     if((!waiting.empty() && unarrived.front().getarrivaltime() < waiting.front().getcurrentwait()) //next "interesting" event is process arrival
    //         && unarrived.front().getarrivaltime() < ready.front().getarrivaltime()) {
    //         int gap = t - unarrived.front().getarrivaltime();
    //         t = unarrived.front().getarrivaltime();
    //         ready.push_back(unarrived.front());
    //         unarrived.pop_front();
    //         std::cout << "time <" << t << ">ms: Process \"" << ready.back().getname() << "\" arrived; added to ready queue ";
    //         printqueue(ready);
    //         //decrement all the other times by gap
    //   } else if(!waiting.empty() && waiting.front().getcurrentwait() < ready.front().getcurrentruntime()) { //next "interesting" event is finishing I/O
    //   	ready.push_back(waiting.front());
    //     waiting.pop_front();
    //   } else { //next "interesting" event is CPU burst completion/termination
    //     if (ready.front().getcurrentwait() == -1 ){ //process terminates
    //       std::cout << "time <" << t << ">ms: Process finishes using the CPU" << std::endl;
    //     	std::cout << "time <" << t << ">ms: Process Terminates" << std::endl;
    //     } else {
    //       waiting.push_back(ready.front()); //process in cpu moved to waiting state for I/O
    //       ready.pop_front();
    //       std::cout << "time <" << t << ">ms: Process finishes using the CPU" << std::endl;
    //        //t = t + context switch
    //       std::cout << "time <" << t << ">ms: Process starts performing I/O" << std::endl; //waiting.back()
    //       std::cout << "time <" << t << ">ms: Process starts using the CPU" << std::endl; //ready.front()
    //     }
    //   }
    // }
}
#endif