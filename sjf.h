#ifndef __sjf_h
#define __sjf_h

//cpp includes
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <list>
#include <iomanip>
//c includes
#include <stdlib.h>
#include <math.h>
//custom includes
#include "process.h"

void sjf(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, const double alpha, const int lambda);
void printcpufinSJF(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinSJF(std::list<Process>& waiting, std::list<Process>& ready, int t);
void printqueueSJF(std::list<Process>& printer);
void printqueueSJF(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
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

void printcpufinSJF(Process* incpu, int t, const int tcs, std::list<Process>& ready, const double alpha){
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueSJF(ready);
    incpu->recalculateTau(alpha);
    std::cout << "time " << t << "ms: Recalculated tau = " << incpu->getTau() << "ms for process " << incpu->getname() << ' ';
    printqueueSJF(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueSJF(ready);
}

void printiofinSJF(std::list<Process>& waiting, std::list<Process>& ready, int t){
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " (tau " << waiting.front().getTau() << "ms) completed I/O; added to ready queue ";
    printqueueSJF(ready);
}

void sjf(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, const double alpha, const double lambda){
    outfile << "Algorithm SJF\n"; //write to file test... working
    int t = 0; //time
    int tau = (int)ceil(1/lambda);
    std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
    std::vector<Process> garbage;
    Process* incpu = NULL; //needs to be a pointer so we can dynammically call the process deep copy constructor
    auto compname = [](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();};
    auto comparrival = [](Process p1, Process p2) -> bool {if(p1.getarrivaltime() == p2.getarrivaltime()) return p1.getname() < p2.getname(); else return p1.getarrivaltime() < p2.getarrivaltime();};
    auto compcurwait = [](Process p1, Process p2) -> bool {if(p1.getcurrentwait() == p2.getcurrentwait()) return p1.getname() < p2.getname(); else return p1.getcurrentwait() < p2.getcurrentwait();};
    auto comptau = [](Process p1, Process p2) -> bool {if(p1.getTau() == p2.getTau()) return p1.getname() < p2.getname(); else return p1.getTau() < p2.getTau();};
    for(Process z : p){
        Process pusher(z);
        pusher.setTau(tau);
        pusher.setPreviousBurst(pusher.getcurrentruntime());
        unarrived.push_back(pusher);
    }
    unarrived.sort(compname);

    int burstcount = 0, bursttotal = 0;
    for(auto const& pp : p) for(int z : pp.getcputime()) {burstcount++; bursttotal += z;}
    double cpuavg = (double)bursttotal / (double)burstcount;
    outfile << "-- average CPU burst time: " << std::setprecision(3) << std::fixed << cpuavg <<  " ms\n";

    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst";
        if(z->getbursts() == 1) std::cout << " (tau " << z-> getTau() << "ms)" << std::endl; else std::cout << "s (tau " << z-> getTau() << "ms)" << std::endl;
    }
     std::cout << "time 0ms: Simulator started for SJF ";
    printqueueSJF(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    if(t > 11800) break;
#endif
        if(waiting.size() > 1) waiting.sort(compcurwait);
        if(ready.size() > 1) ready.sort(comptau);

        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))

        if(waitingtime <= -2){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueSJF(ready);
            if(ready.empty()) {
                t += tcs;
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
            } else {
                t += tcs /2;
                for(auto& r : ready) r.addwaittime(tcs/2);
            }
            garbage.push_back(Process(waiting.front()));
            waiting.pop_front();
            // for(auto& w: waiting) w.decreasewaittime(tcs/2);
            continue;
        }

        if(incpu == NULL && !ready.empty()){
            if(arrivaltime != -1 && arrivaltime < t){ //recently added
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << arrivaltime << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueSJF(ready);
                unarrived.pop_front();
            }
            incpu = new Process(ready.front());
            ready.pop_front();
            t += tcs / 2; 
            for(auto& r : ready) r.addwaittime(tcs/2);
            if(waitingtime > -1){
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
                if(waiting.size() > 1) waiting.sort(compcurwait);
                waitingtime = waiting.front().getcurrentwait();
            }
            if(ready.size() > 1) ready.sort(comptau);
            cputime = incpu->getcurrentruntime();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    if(waitingtime == -1){
                        ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,comptau)) continue;
                        ready.sort(comptau);
                        printiofinSJF(waiting, ready, t-1);
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                        printqueueSJF(ready);
                    } else {
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                        printqueueSJF(ready);
                        ready.push_back(Process(waiting.front()));
                        // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,comptau)) continue;
                        ready.sort(comptau);
                        printiofinSJF(waiting, ready, t);
                        
                    }
                    waiting.pop_front();
                } else {
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                    printqueueSJF(ready);
                }
            } else {
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                printqueueSJF(ready);
            }
            waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1;
        }
#ifdef DEBUG_MODE
    std::cout << "DEBUG --> waitingtime: " << waitingtime << " arrivaltime: " << abs(arrivaltime - t) << " cputime: " << cputime << " ▼▼▼▼▼▼▼▼▼" << std::endl;
    if (incpu != NULL) std::cout << "DEBUG --> incpu: " << incpu->getname() << std::endl;
    std::cout << "DEBUG --> Ready: ";
    printqueueSJF(ready);
    std::cout << "DEBUG --> Waiting: ";
    printqueueSJF(waiting);
    std::cout << "DEBUG --> Unarrived: ";
    printqueueSJF(unarrived);
#endif
        if(arrivaltime == -1 && waitingtime == -1){ //finish cpu time
            t+= cputime;
            for(auto& r : ready) r.addwaittime(cputime);
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
            printcpufinSJF(incpu,t,tcs,ready,alpha);
            incpu->setPreviousBurst(incpu->getcurrentruntime());
            waiting.push_back(Process(*incpu));
            t += tcs/2;
            for(auto& r : ready) r.addwaittime(tcs/2);
            delete incpu;
            incpu = NULL;
        } else if(arrivaltime == -1 && cputime == -1){ //finish waiting
            t += waitingtime;
            for(auto& r : ready) r.addwaittime(waitingtime);
            for(auto& w : waiting) w.decreasewaittime(waitingtime);
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            if(ready.size() > 1) ready.sort(comptau);
            printiofinSJF(waiting,ready,t);
            waiting.pop_front();
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
            t = arrivaltime;
            ready.push_back(Process(unarrived.front()));
            if(ready.size() > 1) ready.sort(comptau);
            std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
            printqueueSJF(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSJF(waiting,ready,t);
                waiting.pop_front();
            } else {
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
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
                printcpufinSJF(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    ready.sort(comptau);
                    if(waitingtime == -1){
                        printiofinSJF(waiting, ready, t-1);
                    } else {
                        printiofinSJF(waiting, ready, t);
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
                for(auto& r : ready) r.addwaittime(gap);
                incpu->decreaseruntime(gap);
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                printqueueSJF(ready);
                unarrived.pop_front();
            } else {
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
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
                printcpufinSJF(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) printiofin(waiting, ready, t);
                delete incpu;
                incpu = NULL;
            }
        } else if(cputime == -1){ //no cputime, compare arrival and waiting
            if(waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSJF(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                printqueueSJF(ready);
                unarrived.pop_front();
            }
        } else { //triple check 
            if(cputime < waitingtime && cputime < abs(arrivaltime - t)){
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
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
                printcpufinSJF(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) printiofin(waiting, ready, t);
                delete incpu;
                incpu = NULL;
            } else if(waitingtime < cputime && waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSJF(waiting,ready,t);
                waiting.pop_front();
            } else if(abs(arrivaltime - t) < cputime && abs(arrivaltime - t) < waitingtime){
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                incpu->decreaseruntime(gap);
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                printqueueSJF(ready);
                unarrived.pop_front();
            } else {
                std::cout << "bad vibes" << std::endl;
            }
        }
    }
    t -= tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for SJF ";
    printqueueSJF(ready);

    int waitn = 0;
    for(auto& g : garbage) {
        for(int ttg : g.getwaittime()) waitn += ttg;
    }
    double waitavg = (double)waitn / (double)burstcount;
    outfile << "-- average wait time: " << std::setprecision(3) << std::fixed << waitavg <<  " ms\n";
    outfile << "-- average turnaround time: " << std::setprecision(3) << std::fixed << (double)(burstcount*4 + bursttotal + waitn) / (double) burstcount << " ms\n";
    outfile << "-- total number of context switches: " << burstcount << '\n';
    outfile << "-- total number of preemptions: 0\n";
}


#endif
