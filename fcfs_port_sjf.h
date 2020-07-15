#ifndef __fcfsport_h
#define __fcfsport_h

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

// Hello

void fcfsport(std::ofstream& outfile, const std::vector<Process>& p, const int tcs);
void printqueueport(std::list<Process>& printer);
void printcpufinport(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinport(std::list<Process>& waiting, std::list<Process>& ready, int t);

void printqueueport(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
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

void printcpufinport(Process* incpu, int t, const int tcs, std::list<Process>& ready){
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueport(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueport(ready);
}

void printiofinport(std::list<Process>& waiting, std::list<Process>& ready, int t){
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue ";
    printqueueport(ready);
}

//arrival time, cpu start, cpu end(io start), io end

void fcfsport(std::ofstream& outfile, const std::vector<Process>& p, const int tcs){
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //time
    std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
    Process* incpu = NULL; //needs to be a pointer so we can dynammically call the process deep copy constructor
    auto compname = [](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();};
    auto comparrival = [](Process p1, Process p2) -> bool {if(p1.getarrivaltime() == p2.getarrivaltime()) return p1.getname() < p2.getname(); else return p1.getarrivaltime() < p2.getarrivaltime();};
    auto compcurwait = [](Process p1, Process p2) -> bool {if(p1.getcurrentwait() == p2.getcurrentwait()) return p1.getname() < p2.getname(); else return p1.getcurrentwait() < p2.getcurrentwait();};
    for(Process z : p){
        Process pusher(z);
        unarrived.push_back(pusher);
    }
    unarrived.sort(compname);

    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        if(z->getbursts() == 1)
            std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst " << std::endl;
        else std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU bursts " << std::endl;
    }
     std::cout << "time 0ms: Simulator started for FCFS ";
    printqueueport(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    if(t > 430000) break;
#endif
        if(waiting.size() > 1) waiting.sort(compcurwait);

        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))

        if(waitingtime <= -2){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueport(ready);
            if(ready.empty()) {
                t += tcs;
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
            } else {
                t += tcs /2;
            }
            waiting.pop_front();
            continue;
        }

        if(incpu == NULL && !ready.empty()){
            incpu = new Process(ready.front());
            ready.pop_front();
            t += tcs / 2; 
            if(waitingtime > -1){
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
            }
            std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
            printqueueport(ready);
            cputime = incpu->getcurrentruntime();
        }
#ifdef DEBUG_MODE
    std::cout << "DEBUG --> waitingtime: " << waitingtime << " arrivaltime: " << abs(arrivaltime - t) << " cputime: " << cputime << " ▼▼▼▼▼▼▼▼▼" << std::endl;
    if (incpu != NULL) std::cout << "DEBUG --> incpu: " << incpu->getname() << std::endl;
    std::cout << "DEBUG --> Ready: ";
    printqueueport(ready);
    std::cout << "DEBUG --> Waiting: ";
    printqueueport(waiting);
    std::cout << "DEBUG --> Unarrived: ";
    printqueueport(unarrived);
#endif
        if(arrivaltime == -1 && waitingtime == -1){ //finish cpu time
            t+= cputime;
            incpu->movenextruntime();
            incpu->decreaseburst();
            // waiting.push_back(Process(*incpu));
            // incpu->recalculateTau(alpha);
            if(incpu->getcurrentwait() <= -2) {
                waiting.push_back(Process(*incpu));
                delete incpu;
                incpu = NULL;
                continue;
            }
            printcpufinport(incpu,t,tcs,ready);
            waiting.push_back(Process(*incpu));
            t += tcs/2;
            delete incpu;
            incpu = NULL;
        } else if(arrivaltime == -1 && cputime == -1){ //finish waiting
            t += waitingtime;
            for(auto& w : waiting) w.decreasewaittime(waitingtime);
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            printiofinport(waiting,ready,t);
            waiting.pop_front();
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            t = arrivaltime;
            ready.push_back(Process(unarrived.front()));
            std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
            printqueueport(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                t += waitingtime;
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinport(waiting,ready,t);
                waiting.pop_front();
            } else {
                t+= cputime;
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                // waiting.push_back(Process(*incpu));
                // incpu->recalculateTau(alpha);
                 if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    if(waitingtime == -1) {
                        printiofinport(waiting, ready, t-1);
                    }
                    else {
                        printiofinport(waiting, ready, t);
                    }
                    waiting.pop_front();
                }
                delete incpu;
                incpu = NULL;
            }
        } else if(waitingtime == -1){ //no waiting, compare arrival/cpu
            if(abs(arrivaltime - t) < cputime){
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                incpu->decreaseruntime(gap);
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueport(ready);
                unarrived.pop_front();
            } else {
                t+= cputime;
                incpu->movenextruntime();
                incpu->decreaseburst();
                // waiting.push_back(Process(*incpu));
                // incpu->recalculateTau(alpha);
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0){
                    printiofinport(waiting, ready, t);
                    ready.push_back(Process(*incpu));
                }
                delete incpu;
                incpu = NULL;
            }
        } else if(cputime == -1){ //no cputime, compare arrival and waiting
            if(waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinport(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& w : waiting) w.decreasewaittime(gap);
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueport(ready);
                unarrived.pop_front();
            }
        } else { //triple check 
            if(cputime < waitingtime && cputime < abs(arrivaltime - t)){
                t+= cputime;
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                // waiting.push_back(Process(*incpu));
                // incpu->recalculateTau(alpha);
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0){
                    printiofinport(waiting, ready, t);
                    ready.push_back(Process(*incpu));
                }
                delete incpu;
                incpu = NULL;
            } else if(waitingtime < cputime && waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinport(waiting,ready,t);
                waiting.pop_front();
            } else if(abs(arrivaltime - t) < cputime && abs(arrivaltime - t) < waitingtime){
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& w : waiting) w.decreasewaittime(gap);
                incpu->decreaseruntime(gap);
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueport(ready);
                unarrived.pop_front();
            } else {
                std::cout << "bad vibes" << std::endl;
            }
        }
    }
    t -= tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for FCFS ";
    printqueueport(ready);
}
#endif
