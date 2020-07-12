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
        // int cputime =  !ready.empty() ? ready.front().getcurrentruntime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))
#ifdef DEBUG_MODE
    std::cout << "waitingtime: " << waitingtime << " arrivaltime: " << arrivaltime << " cputime: " << cputime << "▼▼▼▼▼▼▼▼▼" << std::endl;
#endif
        if(waitingtime == -2){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueue(ready);
            waiting.pop_front();
        }
        if(incpu == NULL && !ready.empty()){
            incpu = new Process(ready.front());
            ready.pop_front();
            t += tcs / 2; 
            std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the cpu for " << incpu->getcurrentruntime() << "ms burst ";
            printqueue(ready);
            cputime = incpu->getcurrentruntime();
        } 

        if(arrivaltime == -1){

        } else if(waitingtime == -1){
            //compare arrivaltime and cpu time...
            if(cputime == -1){ //if theres no cpu times to compare... we have an arrival
                t = arrivaltime;
                while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                    ready.push_back(unarrived.front());
                    std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    printqueue(ready);
                    unarrived.pop_front();
                }
            } else {//compare arrivaltime and cpu time...
                if(arrivaltime > cputime){
                    //just increment t by cputime..
                    t += cputime;
                    incpu->movenextruntime();//will be removed next run...
                    incpu->decreaseburst(); 
                    incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                    printqueue(ready);
                    waiting.push_back(Process(*incpu));
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                    printqueue(ready);
                    free(incpu);
                    incpu = NULL;
                } else {
                    int gap = arrivaltime - t;
                    t += gap;
                    incpu->decreaseruntime(gap); //need to handle the process currently running also
                    while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                        ready.push_back(unarrived.front());
                        std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                        printqueue(ready);
                        unarrived.pop_front();
                    }
                }
            }

        } else if(cputime == -1){
        
        } else { //if we get here that means all 3 queues are occupied... god help us
            if(arrivaltime < waitingtime && arrivaltime < cputime){ //process arrival
                t = arrivaltime;
                ready.push_back(unarrived.front());
                std::cout << "time <" << t << ">: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueue(ready);
                unarrived.pop_front();
            }
            else if(waitingtime < arrivaltime && waitingtime < cputime){ //process finishes I/O

            }
            else{ //process completes CPU burst

            }
        }
    }


                    //we need to use to commented code in sections where the wait queue is full!!!!!!
                    // if(!waiting.empty()){ //need to decrease all the waiting times by cputime...
                    //     for(auto const& w : waiting){
                    //         int cmp = w.getcurrentwait() - cputime;
                    //         if(cmp < 0 || cmp == 0){ //if the time will be under OR done

                    //         } else{

                    //         }
                    //     }
                    // }
}
#endif