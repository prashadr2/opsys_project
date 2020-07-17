#ifndef __roundrobin_h
#define __roundrobin_h

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

void rr(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, int tslice, std::string rradd);
void printcpufinRR(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinRR(std::list<Process>& waiting, std::list<Process>& ready, int t);
void printqueueRR(std::list<Process>& printer);
bool preemption_ioRR(Process*& incpu, std::list<Process>& waiting, std::list<Process>& ready, int& t, int tcs, int tslice, std::string rradd);

void printqueueRR(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
  std::cout << "[Q";
  if(printer.size() == 0){
      std::cout << " <empty>]" << std::endl;
      return;
  }
  for(std::list<Process>::iterator z = printer.begin(); z != printer.end(); z++) {
      std::cout << ' ' << z->getname();
#ifdef DEBUG_MODE
    std::cout << "-->(waittime: " << z->getcurrentwait() << "), ";
#endif
  }
  std::cout << "]" << std::endl;
}

void printcpufinRR(Process* incpu, int t, const int tcs, std::list<Process>& ready){
    if (t > 999) return;
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueRR(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueRR(ready);
}

void printiofinRR(std::list<Process>& waiting, std::list<Process>& ready, int t){
    if (t > 999) return;
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue ";
    printqueueRR(ready);
}

bool preemption_ioRR(Process*& incpu, std::list<Process>& ready, std::list<Process>& waiting, int& t, int tcs, int tslice, std::string rradd, int& preemptions){ //call when finish IO and ready.push_back
    //ready.back() is the process to preempt...
    if(incpu->getslice() >= incpu->getcurrentruntime()) return false;
    if(ready.empty()) {
        t += incpu->getslice();
        for(auto& r : ready) r.addwaittime(incpu->getslice());
        for(auto& w : waiting) w.decreasewaittime(incpu->getslice());
        incpu->decreaseruntime(incpu->getslice());
        if (t <= 999) std::cout << "time " << t << "ms: Time slice expired; no preemption because ready queue is empty ";
        if (t <= 999) printqueueRR(ready);
         if(!waiting.empty()){
            int waitingtime = waiting.front().getcurrentwait();
            if(waitingtime <= 0) {
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(waitingtime == -1){
                    printiofinRR(waiting, ready, t-1);
                } else {
                    printiofinRR(waiting, ready, t);
                }
                waiting.pop_front();
            }
        }
        incpu->setslice(tslice);
    } else {
        preemptions++;
        incpu->setp(true);
        t += incpu->getslice();
        for(auto& r : ready) r.addwaittime(incpu->getslice());
        for(auto& w : waiting) w.decreasewaittime(incpu->getslice());
        incpu->decreaseruntime(incpu->getslice());
        if (t <=  999) std::cout << "time " << t << "ms: Time slice expired; process " << incpu->getname() << " preempted with " << incpu->getcurrentruntime() << "ms to go ";
        if (t <= 999) printqueueRR(ready);
    #ifdef DEBUG_MODE
        if(!waiting.empty()) std::cout << "preempt_waittime: " << waiting.front().getcurrentwait() << std::endl;
    #endif
        bool pushed = false;
        if(!waiting.empty()){
            int waitingtime = waiting.front().getcurrentwait();
            if(waitingtime <= 0) {
                waiting.front().movenextwait();
               if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                if(waitingtime == -1){
                    printiofinRR(waiting, ready, t-1);
                } else {
                    printiofinRR(waiting, ready, t);
                }
                waiting.pop_front();
            }
        }
        // incpu->movenextruntime();
        incpu->setslice(tslice);
        t += tcs/2;
        for(auto& r : ready) r.addwaittime(tcs/2);
        if(!waiting.empty() && waiting.front().getcurrentwait() == 2) {ready.push_back(*incpu); pushed = true;}
        for(auto& w : waiting) w.decreasewaittime(tcs/2);
        if(!waiting.empty()){
            int waitingtime = waiting.front().getcurrentwait();
            if(waitingtime <= 0) {
                waiting.front().movenextwait();
                if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                if(waitingtime == -1){
                    printiofinRR(waiting, ready, t-1);
                } else {
                    printiofinRR(waiting, ready, t);
                }
                waiting.pop_front();
            }
        }
        if(!pushed) ready.push_back(*incpu);
        delete incpu;
        incpu = NULL;
    }
    return true;
}

//For a preemption during the RR algorithm, the process always goes to the end of the queue.
void rr(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, int tslice, std::string rradd){
    outfile << "Algorithm RR\n"; //write to file test... working
    int t = 0; //time
    std::list<Process> unarrived;
    std::list<Process> ready; //first positon in ready queue IS THE PROCESS CURRENTLY IN THE CPU
    std::list<Process> waiting;
    std::vector<Process> garbage;
    Process* incpu = NULL; //needs to be a pointer so we can dynammically call the process deep copy constructor
    auto compname = [](Process p1, Process p2) -> bool {return p1.getname() < p2.getname();};
    auto comparrival = [](Process p1, Process p2) -> bool {if(p1.getarrivaltime() == p2.getarrivaltime()) return p1.getname() < p2.getname(); else return p1.getarrivaltime() < p2.getarrivaltime();};
    auto compcurwait = [](Process p1, Process p2) -> bool {if(p1.getcurrentwait() == p2.getcurrentwait()) return p1.getname() < p2.getname(); else return p1.getcurrentwait() < p2.getcurrentwait();};
    for(Process z : p){
        Process pusher(z);
        unarrived.push_back(pusher);
    }
    unarrived.sort(compname);

     int burstcount = 0, bursttotal = 0, preemptions = 0;
    for(auto const& pp : p) for(int z : pp.getcputime()) {burstcount++; bursttotal += z;}
    double cpuavg = (double)bursttotal / (double)burstcount;
    outfile << "-- average CPU burst time: " << std::setprecision(3) << std::fixed << cpuavg <<  " ms\n";

    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst";
        if(z->getbursts() == 1) std::cout << std::endl; else std::cout << "s" << std::endl;
    }
    std::cout << "time 0ms: Simulator started for RR ";
    printqueueRR(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    if(t > 17000) break;
#endif
        if(waiting.size() > 1) waiting.sort(compcurwait);

        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))
        
        if(waitingtime <= -2){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueRR(ready);
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
                if(rradd == "END") ready.push_back(Process(unarrived.front())); else ready.push_front(Process(unarrived.front()));
                if (t <= 999) std::cout << "time " << arrivaltime << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueRR(ready);
                unarrived.pop_front();
            }
            incpu = new Process(ready.front());
            if(!incpu->getp()) incpu->setslice(tslice);
            ready.pop_front();
            t += tcs / 2; 
            for(auto& r : ready) r.addwaittime(tcs/2);
            if(waitingtime > -1){
                for(auto& w: waiting) w.decreasewaittime(tcs/2);
                if(waiting.size() > 1) waiting.sort(compcurwait);
                waitingtime = waiting.front().getcurrentwait();
            }
            cputime = incpu->getcurrentruntime();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    if(waitingtime == -1){
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                        printiofinRR(waiting, ready, t-1);
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                        if (t <= 999) {if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";}
                        if (t <= 999) printqueueRR(ready);
                    } else {
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                        if (t <= 999) {if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";}
                        if (t <= 999) printqueueRR(ready);
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                        printiofinRR(waiting, ready, t);
                        
                    }
                    waiting.pop_front();
                } else {
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                    if (t <= 999) {if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";}
                    if (t <= 999) printqueueRR(ready);
                }

            } else {
                if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                if (t <= 999) {if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";}
                if (t <= 999) printqueueRR(ready);
            }
            waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1;
        }
#ifdef DEBUG_MODE
    std::cout << "DEBUG -->waitingtime: " << waitingtime << " arrivaltime: " << abs(arrivaltime - t) << " cputime: " << cputime << " ▼▼▼▼▼▼▼▼▼" << std::endl;
    std::cout << "DEBUG -->cpu_address: " << incpu << std::endl;
    if (incpu != NULL) std::cout << "DEBUG --> incpu: " << incpu->getname() << " || timeslice: " << incpu->getslice() << " || preempted = " << incpu->getp() << std::endl;
    std::cout << "DEBUG --> Ready: ";
    printqueueRR(ready);
    std::cout << "DEBUG --> Waiting: ";
    printqueueRR(waiting);
    std::cout << "DEBUG --> Unarrived: ";
    printqueueRR(unarrived);
#endif
        if(arrivaltime == -1 && waitingtime == -1){ //finish cpu time
            if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions))continue;
            t+= cputime;
            for(auto& r : ready) r.addwaittime(cputime);
            incpu->movenextruntime();
            incpu->setslice(tslice);
            incpu->decreaseburst();
            if(incpu->getcurrentwait() <= -2) {
                waiting.push_back(Process(*incpu));
                delete incpu;
                incpu = NULL;
                continue;
            }
            printcpufinRR(incpu,t,tcs,ready);
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
            if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
            printiofinRR(waiting,ready,t);
            waiting.pop_front();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                    if(waitingtime == -1){
                        printiofinRR(waiting, ready, t-1);
                    } else {
                        printiofinRR(waiting, ready, t);
                    }
                    waiting.pop_front();
                }
            }
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
            t = arrivaltime;
            if(rradd == "END") ready.push_back(Process(unarrived.front())); else ready.push_front(Process(unarrived.front()));
            if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
            if (t <= 999) printqueueRR(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                if(incpu->getslice() < waitingtime) if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else if(waitingtime > cputime){
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t += cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->setslice(tslice);
                incpu->decreaseburst();
                 if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinRR(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                delete incpu;
                incpu = NULL;
            } else { //equals case im cry :(
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->setslice(tslice);
                incpu->decreaseburst();
                 if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinRR(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));

                // 

                waiting.front().movenextwait();
                if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                delete incpu;
                incpu = NULL;
            }
        } else if(waitingtime == -1){ //no waiting, compare arrival/cpu
            if(abs(arrivaltime - t) < cputime){
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                incpu->decreaseruntime(gap);
                if(rradd == "END") ready.push_back(Process(unarrived.front())); else ready.push_front(Process(unarrived.front()));;

                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueRR(ready);
                unarrived.pop_front();
            } else {
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                incpu->movenextruntime();
                incpu->setslice(tslice);
                incpu->decreaseburst();
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinRR(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                delete incpu;
                incpu = NULL;
            }
        } else if(cputime == -1){ //no cputime, compare arrival and waiting
            if(waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                waiting.front().movenextwait();
                if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                if(rradd == "END") ready.push_back(Process(unarrived.front())); else ready.push_front(Process(unarrived.front()));;
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueRR(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            }
        } else { //triple check 
            if(cputime < waitingtime && cputime < abs(arrivaltime - t)){
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->setslice(tslice);
                incpu->decreaseburst();
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinRR(incpu,t,tcs,ready);
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                delete incpu;
                incpu = NULL;
            } else if(waitingtime < cputime && waitingtime < abs(arrivaltime - t)){
                if(incpu->getslice() < waitingtime) if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice,rradd,preemptions)) continue;
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else if(abs(arrivaltime - t) < cputime && abs(arrivaltime - t) < waitingtime){
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                incpu->decreaseruntime(gap);
                if(rradd == "END") ready.push_back(Process(unarrived.front())); else ready.push_front(Process(unarrived.front()));;
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                if (t <= 999) printqueueRR(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        if(rradd == "END") ready.push_back(Process(waiting.front())); else ready.push_front(Process(waiting.front()));;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else {
                std::cout << "bad vibes" << std::endl;
            }
        }
    }
    t -= tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for RR ";
    printqueueRR(ready);

    int waitn = 0;
    for(auto& g : garbage) {
        for(int ttg : g.getwaittime()) waitn += ttg;
    }
    double waitavg = (double)waitn / (double)burstcount;
    outfile << "-- average wait time: " << std::setprecision(3) << std::fixed << waitavg <<  " ms\n";
    outfile << "-- average turnaround time: " << std::setprecision(3) << std::fixed << (double)((burstcount+preemptions)*4 + bursttotal + waitn) / (double) burstcount << " ms\n";
    outfile << "-- total number of context switches: " << burstcount + preemptions << '\n';
    outfile << "-- total number of preemptions: " << preemptions << '\n';
}


#endif