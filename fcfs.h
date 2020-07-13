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
    if (incpu != NULL) std::cout << "incpu: " << incpu->getname() << std::endl;
    std::cout << "Ready: ";
    printqueue(ready);
    std::cout << "Waiting: ";
    printqueue(waiting);
    std::cout << "Unarrived: ";
    printqueue(unarrived);
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
            if(waitingtime == -1){//we just have to deal with cputime
                t += cputime;
                incpu->movenextruntime();//will be removed next run...
                incpu->decreaseburst(); 
                //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                printqueue(ready);
                waiting.push_back(Process(*incpu));
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                printqueue(ready);
                free(incpu);
                incpu = NULL;
            } else if (cputime == -1) { //we just have to deal with waitingtime
                t += waitingtime;
                std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue";
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                waiting.pop_front();
                printqueue(ready);

            } else { //need to compare waittime and cpu time
                if(waitingtime <= cputime){ //need to finish the waiting process and decrease cputime by waitingtime (handle equal case also)
                    if(waitingtime == cputime){ //edge case
                        std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue";
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        waiting.pop_front();
                        printqueue(ready);
                        //
                        incpu->movenextruntime();//will be removed next run...
                        incpu->decreaseburst(); 
                        //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                        printqueue(ready);
                        waiting.push_back(Process(*incpu));
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                        printqueue(ready);
                        free(incpu);
                        incpu = NULL;
                    } else {
                        incpu->decreaseruntime(waitingtime);
                        std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue";
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        waiting.pop_front();
                        printqueue(ready);
                    }
                } else { //decrease waiting time by cputime, move cpu out of processor
                    waiting.front().decreasewaittime(cputime);
                    incpu->movenextruntime();//will be removed next run...
                    incpu->decreaseburst(); 
                    //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                    printqueue(ready);
                    waiting.push_back(Process(*incpu));
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                    printqueue(ready);
                    free(incpu);
                    incpu = NULL;
                }
            }
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
                    //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                    printqueue(ready);
                    waiting.push_back(Process(*incpu));
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                    printqueue(ready);
                    free(incpu);
                    incpu = NULL;
                } else {
                    int gap = abs(arrivaltime - t);
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
            //if we get here, then waitngtime and arrival time are both populated, or else we would've gone into one of the above branches
            if(waitingtime <= arrivaltime){//decrease arrivaltime by waitingtime
                if(waitingtime == arrivaltime){
//TODO
                } else { //finish waiting and jump time
                    t += waitingtime;
                    // incpu->decreaseruntime(waitingtime);
                    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue";
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    waiting.pop_front();
                    printqueue(ready);
                }
            } else { //decrease waitingtime by arrivalgap, and handle the arrival
                int gap = abs(arrivaltime - t);
                t += gap;
                waiting.front().decreasewaittime(gap);
                while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                    ready.push_back(unarrived.front());
                    std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    printqueue(ready);
                    unarrived.pop_front();
                }
            }
        } else { //if we get here that means all 3 queues are occupied... god help us
            if(abs(arrivaltime - t) <= waitingtime && abs(arrivaltime - t) <= cputime){ //process arrival
                int gap = abs(arrivaltime - t);
                t += gap;
                incpu->decreaseruntime(gap); //need to handle the process currently running also
                while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                    ready.push_back(unarrived.front());
                    std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    printqueue(ready);
                    unarrived.pop_front();
                }
                //the gap MUST not go over the smallest waitingtime by the if statement...
                for(auto & w : waiting){
                    if(w.getcurrentwait() - gap == 0){ //if the remainder wait time is the same as arrival time difference
                        Process adder(w);
                        //TODO: remove w from waiting at this line!!!!!!
                        adder.movenextwait();
                        std::cout << "time " << t << "ms: Process " << adder.getname() << " completed I/O; added to ready queue";
                        ready.push_back(Process(adder));
                        printqueue(ready);

                    } else { //just decrease wait time
                        w.decreasewaittime(gap);
                    }
                }
                //need to edit cputime...
                if(cputime == gap) {//take cpu out and go into waiting
                    incpu->movenextruntime();//will be removed next run...
                    incpu->decreaseburst(); 
                    //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                    printqueue(ready);
                    waiting.push_back(Process(*incpu));
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                    printqueue(ready);
                    free(incpu);
                    incpu = NULL;
                } else {//just decrease time
                    incpu->decreaseruntime(gap);
                }
            } else if(waitingtime <= abs(arrivaltime - t) && waitingtime <= cputime){ //process finishes I/O
                t += waitingtime;
                std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue";
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                waiting.pop_front();
                printqueue(ready);
                if(arrivaltime == t){ //arrival at same time that waiting is over
                    while(!unarrived.empty() && unarrived.front().getarrivaltime() == t){
                        ready.push_back(unarrived.front());
                        std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                        printqueue(ready);
                        unarrived.pop_front();
                    }
                }
                //decrease cputime by gap or move it out if we finish n same time as wait
                if(cputime == waitingtime){ //move it out
                    incpu->movenextruntime();//will be removed next run...
                    incpu->decreaseburst(); 
                    //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                    printqueue(ready);
                    waiting.push_back(Process(*incpu));
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                    printqueue(ready);
                    free(incpu);
                    incpu = NULL;
                } else { //decreasee runtime
                    incpu->decreaseruntime(waitingtime);
                }

            } else if(cputime <= waitingtime && cputime <= abs(arrivaltime - t)){ //process completes CPU burst
                t += cputime;
                waiting.front().decreasewaittime(cputime);

                incpu->movenextruntime();//will be removed next run...
                incpu->decreaseburst(); 
                //incpu->movenextruntime(); //make sure we do movenextwaittime() on the front of waitingqueue if we are done waiting
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts() << " bursts to go ";
                printqueue(ready);
                waiting.push_back(Process(*incpu));
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait() << "ms ";
                printqueue(ready);
                free(incpu);
                incpu = NULL;
//                 if(cputime == waiting){
// //TODO
//                 } else {

//                 }
//             //
//                 if(cputime == abs(arrivaltime - t)){
// //TODO
//                 } else {

//                 }
            } else{
                std::cout << "what case even makes this run???????????????????????????????????" << std::endl;
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