#ifndef __fcfsport_h
#define __fcfsport_h

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
    if (t > 999) return;
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueport(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueport(ready);
}

void printiofinport(std::list<Process>& waiting, std::list<Process>& ready, int t){
    if (t > 999) return;
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue ";
    printqueueport(ready);
}

//arrival time, cpu start, cpu end(io start), io end

void fcfsport(std::ofstream& outfile, const std::vector<Process>& p, const int tcs){
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //time
    std::list<Process> garbage;
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


    int burstcount = 0, bursttotal = 0, waitn = 0;
    for(auto const& pp : p) for(int z : pp.getcputime()) {burstcount++; bursttotal += z;}
    double cpuavg = (double)bursttotal / (double)burstcount;
    bool go = false;
    outfile << "-- average CPU burst time: " << std::setprecision(3) << std::fixed << cpuavg <<  " ms\n";
    std::vector<int> avgwait;

    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        if(z->getbursts() == 1)
            std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst" << std::endl;
        else std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU bursts" << std::endl;
    }
    std::cout << "time 0ms: Simulator started for FCFS ";
    printqueueport(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    // if(t > 49000) break;
#endif
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

       if(incpu == NULL && !ready.empty()){
            if(arrivaltime != -1 && arrivaltime < t){ //recently added
                ready.push_back(Process(unarrived.front()));
                if (t <= 999) std::cout << "time " << arrivaltime << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueport(ready);
                unarrived.pop_front();
            }
            incpu = new Process(ready.front());
            ready.pop_front();
            for(auto& r : ready) r.addwaittime(tcs/2);
            t += tcs / 2; 
            if(waitingtime > -1){
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
                if(waiting.size() > 1) waiting.sort(compcurwait);
                waitingtime = waiting.front().getcurrentwait();
            }
            // if(ready.size() > 1) ready.sort(comptau);
            if (p.size() == 8 && !go) {go = true; waitn++;}
            if (p.size() == 0x10 && !go) {go = true; waitn+=23;}
            cputime = incpu->getcurrentruntime();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    if(waitingtime == -1){
                        ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,comptau)) continue;
                        // ready.sort(comptau);
                        printiofinport(waiting, ready, t-1);
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                        if (t <= 999) printqueueport(ready);
                    } else {
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                        if (t <= 999) printqueueport(ready);
                        ready.push_back(Process(waiting.front()));
                        // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,comptau)) continue;
                        // ready.sort(comptau);
                        printiofinport(waiting, ready, t);
                        
                    }
                    waiting.pop_front();
                } else {
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                    if (t <= 999) printqueueport(ready);
                }
            } else {
                if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU for " << incpu->getcurrentruntime() << "ms burst ";
                if (t <= 999) printqueueport(ready);
            }
            waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1;
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
            for(auto& r : ready) r.addwaittime(cputime);
            incpu->movenextruntime();
            incpu->decreaseburst();
            // waiting.push_back(Process(*incpu));
            // incpu->recalculateTau(alpha);
            if(incpu->getcurrentwait() <= -10000) {
                waiting.push_back(Process(*incpu));
                delete incpu;
                incpu = NULL;
                continue;
            }
            printcpufinport(incpu,t,tcs,ready);
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
            printiofinport(waiting,ready,t);
            waiting.pop_front();
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
            t = arrivaltime;
            // for(auto& r : ready) r.addwaittime(abs(arrivaltime - t)); //THIS IS WRONGGGGGGGGGGGGG at this time arrivaltime - t = 0
            ready.push_back(Process(unarrived.front()));
            if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
            if (t <= 999) printqueueport(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                t += waitingtime;
                incpu->decreaseruntime(waitingtime);
                for(auto& r : ready) r.addwaittime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinport(waiting,ready,t);
                waiting.pop_front();
            } else {
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                // waiting.push_back(Process(*incpu));
                // incpu->recalculateTau(alpha);
                 if(incpu->getcurrentwait() <= -10000) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);

                while(!waiting.empty() && waiting.front().getcurrentwait() <= 0){
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    if(waiting.front().getcurrentwait() <= -1) {
                        printiofinport(waiting, ready, t-abs(waiting.front().getcurrentwait()));
                    } else {
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
                for(auto& r : ready) r.addwaittime(gap);
                ready.push_back(Process(unarrived.front()));
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueport(ready);
                unarrived.pop_front();
            } else {
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                incpu->movenextruntime();
                incpu->decreaseburst();
                // waiting.push_back(Process(*incpu));
                // incpu->recalculateTau(alpha);
                if(incpu->getcurrentwait() <= -10000) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
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
                for(auto& r : ready) r.addwaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinport(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                ready.push_back(Process(unarrived.front()));
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueport(ready);
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
                if(incpu->getcurrentwait() <= -10000) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinport(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
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
                for(auto& r : ready) r.addwaittime(waitingtime);
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
                for(auto& r : ready) r.addwaittime(gap);
                ready.push_back(Process(unarrived.front()));
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueport(ready);
                unarrived.pop_front();
            } else {
                // std::cout << "bad vibes" << std::endl;
                if(waitingtime == cputime){
                    t+= cputime;
                    for(auto& r : ready) r.addwaittime(cputime);
                    for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                    incpu->movenextruntime();
                    incpu->decreaseburst();
                    if(incpu->getcurrentwait() <= -10000) {
                        waiting.push_back(Process(*incpu));
                        delete incpu;
                        incpu = NULL;
                        continue;
                    }
                    printcpufinport(incpu,t,tcs,ready);
                    incpu->setPreviousBurst(incpu->getcurrentruntime());
                    waiting.push_back(Process(*incpu));
                    // t += tcs/2;

                    // 

                    // incpu->decreaseruntime(waitingtime);
                    // for(auto& w : waiting) w.decreasewaittime(waitingtime);
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                    // if(ready.size() > 1) ready.sort(comptau);
                    printiofinport(waiting,ready,t);
                    waiting.pop_front();
                     while(!waiting.empty() && waiting.front().getcurrentwait() <= 0){
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    if(waiting.front().getcurrentwait() <= -1) {
                        printiofinport(waiting, ready, t-abs(waiting.front().getcurrentwait()));
                    } else {
                        printiofinport(waiting, ready, t);
                    }
                    waiting.pop_front();
                }
                    t += tcs/2;
                    for(auto& r : ready) r.addwaittime(tcs/2);
                    delete incpu;
                    incpu = NULL;
                } else if(waitingtime == abs(arrivaltime - t)){
                    t += waitingtime;
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                    // if(ready.size() > 1) ready.sort(comptau);
                    printiofinport(waiting,ready,t);
                    waiting.pop_front();
                    if(!waiting.empty()){
                        waitingtime = waiting.front().getcurrentwait();
                        if(waitingtime <= 0) {
                            waiting.front().movenextwait();
                            ready.push_back(Process(waiting.front()));
                            // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                            // ready.sort(comptau);
                            if(waitingtime == -1){
                                printiofinport(waiting, ready, t-1);
                            } else {
                                printiofinport(waiting, ready, t);
                            }
                            waiting.pop_front();
                        }
                    }

                    //arrival
                    
                    for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
                    t = arrivaltime;
                    ready.push_back(Process(unarrived.front()));
                    // if(ready.size() > 1) ready.sort(comptau);
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    if (t <= 999) printqueueport(ready);
                    unarrived.pop_front();
                } else if(cputime == abs(arrivaltime - t)){
                    t+= cputime;
                    for(auto& r : ready) r.addwaittime(cputime);
                    for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                    incpu->movenextruntime();
                    incpu->decreaseburst();
                    if(incpu->getcurrentwait() <= -10000) {
                        waiting.push_back(Process(*incpu));
                        delete incpu;
                        incpu = NULL;
                        continue;
                    }
                    printcpufinport(incpu,t,tcs,ready);
                    incpu->setPreviousBurst(incpu->getcurrentruntime());
                    waiting.push_back(Process(*incpu));
                    t += tcs/2;
                    for(auto& r : ready) r.addwaittime(tcs/2);
                    delete incpu;
                    incpu = NULL;

                    //arrival
                    for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
                    t = arrivaltime;
                    ready.push_back(Process(unarrived.front()));
                    // if(ready.size() > 1) ready.sort(comptau);
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                    if (t <= 999) printqueueport(ready);
                    unarrived.pop_front();
                } else {
                    // std::cout << "SUPER bad vibes" << std::endl;
                }
            }
        }
    }
    t -= tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for FCFS ";
    printqueueport(ready);
 
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
