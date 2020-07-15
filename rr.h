#ifndef __roundrobin_h
#define __roundrobin_h

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

void rr(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, int tslice, std::string rradd);
void printcpufinRR(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinRR(std::list<Process>& waiting, std::list<Process>& ready, int t);
void printqueueRR(std::list<Process>& printer);
bool preemption_ioRR(Process*& incpu, std::list<Process>& waiting, std::list<Process>& ready, int& t, int tcs, int tslice);
bool preemption_willRR(Process*& incpu, std::list<Process>& waiting, std::list<Process>& ready, int& t, int tcs, int tslice);

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
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueRR(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueRR(ready);
}

void printiofinRR(std::list<Process>& waiting, std::list<Process>& ready, int t){
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " completed I/O; added to ready queue ";
    printqueueRR(ready);
}

bool preemption_ioRR(Process*& incpu, std::list<Process>& ready, std::list<Process>& waiting, int& t, int tcs, int tslice){ //call when finish IO and ready.push_back
    //ready.back() is the process to preempt...
    // if(ready.back().getTau() - (ready.back().getPreviousBurst() - ready.back().getcurrentruntime()) >= incpu->getTau() - (incpu->getPreviousBurst() - incpu->getcurrentruntime())) return false;
    if(tslice >= incpu->getcurrentruntime()) return false;
    incpu->setp(true);
    t += tslice;
    for(auto& w : waiting) w.decreasewaittime(tslice);
    incpu->decreaseruntime(tslice);
    std::cout << "time " << t << "ms: Time slice expired; process " << incpu->getname() << " preempted with " << incpu->getcurrentruntime() << "ms to go ";
    printqueueRR(ready);
    // std::cout << "time " << t << "ms: Process " << ready.back().getname() << " completed I/O; preempting " << incpu->getname() << ' ';
    // printqueueRR(ready);
    // waiting.pop_front();
#ifdef DEBUG_MODE
    if(!waiting.empty()) std::cout << "preempt_waittime: " << waiting.front().getcurrentwait() << std::endl;
#endif
    bool pushed = false;
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
    t += tcs/2;
    if(!waiting.empty() && waiting.front().getcurrentwait() == 2) {ready.push_back(*incpu); pushed = true;}
    for(auto& w : waiting) w.decreasewaittime(tcs/2);
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
    if(!pushed) ready.push_back(*incpu);
    delete incpu;
    incpu = NULL;
    return true;
}

bool preemption_willRR(Process*& incpu, std::list<Process>& ready, std::list<Process>& waiting, int& t, int tcs, int tslice){
    //ready.back() is the process to preempt...
    if(ready.back().getTau() - (ready.back().getPreviousBurst() - ready.back().getcurrentruntime()) >= incpu->getTau() - (incpu->getPreviousBurst() - incpu->getcurrentruntime())) {t += 1; return false;}
    incpu->setp(true);
    std::cout << "time " << t << "ms: Process " << ready.front().getname() << " completed I/O; added to ready queue ";
    printqueueRR(ready);
    waiting.pop_front();
    t += tcs/2;
    std::cout << "time " << t-1 << "ms: Process " << incpu->getname() << " started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
    printqueueRR(ready);
    std::cout << "time " << t-1 << "ms: Process " << ready.front().getname() << " will preempt " << incpu->getname() << ' ';
    printqueueRR(ready);
    for(auto& w : waiting) w.decreasewaittime(tcs/2);
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
    ready.push_back(*incpu);
    delete incpu;
    incpu = NULL;
    t+=1;
    return true;
}

//For a preemption during the RR algorithm, the process always goes to the end of the queue.
void rr(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, int tslice, std::string rradd){
    outfile << "Algorithm RR\n"; //write to file test... working
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
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst";
        if(z->getbursts() == 1) std::cout << std::endl; else std::cout << "s" << std::endl;
    }
    std::cout << "time 0ms: Simulator started for RR ";
    printqueueRR(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    if(t > 34600) break;
#endif
        if(waiting.size() > 1) waiting.sort(compcurwait);

        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))
        int remainings = (incpu != NULL) ? incpu->getslice() : -1;

        if(waitingtime <= -2){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueRR(ready);
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
            // std::cout << "triggered @ t = " << t << ", waitt: " << waitingtime << std::endl;
            if(arrivaltime != -1 && arrivaltime < t){ //recently added
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << arrivaltime << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueRR(ready);
                unarrived.pop_front();
            }
            incpu = new Process(ready.front());
            ready.pop_front();
            t += tcs / 2; 
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
                        // std::cout << "switch1" << std::endl;
                        ready.push_back(Process(waiting.front()));
                        if(ready.back().getTau() > incpu->getTau()){
                            if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                        } else {
                            t-=1;
                            if(preemption_willRR(incpu,ready,waiting,t,tcs,tslice)) {continue;}
                        }
                        printiofinRR(waiting, ready, t-1);
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                        if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";
                        printqueueRR(ready);
                    } else {
                        // std::cout << "switch2" << std::endl;
                        std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                        if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";
                        printqueueRR(ready);
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                        printiofinRR(waiting, ready, t);
                        
                    }
                    waiting.pop_front();
                } else {
                    // std::cout << "switch3" << std::endl;
                    std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                    if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";
                    printqueueRR(ready);
                }
            } else {
                // std::cout << "switch4" << std::endl;
                std::cout << "time " << t << "ms: Process " << incpu->getname() << " started using the CPU ";
                if(incpu->getp()) std::cout << "with " << incpu->getcurrentruntime() << "ms burst remaining "; else std::cout << "for " << incpu->getcurrentruntime() << "ms burst ";
                printqueueRR(ready);
            }
            waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1;
        }
#ifdef DEBUG_MODE
    std::cout << "DEBUG -->waitingtime: " << waitingtime << " arrivaltime: " << abs(arrivaltime - t) << " cputime: " << cputime << " ▼▼▼▼▼▼▼▼▼" << std::endl;
    std::cout << "DEBUG -->cpu_address: " << incpu << std::endl;
    if (incpu != NULL) std::cout << "DEBUG --> incpu: " << incpu->getname() << std::endl;
    std::cout << "DEBUG --> Ready: ";
    printqueueRR(ready);
    std::cout << "DEBUG --> Waiting: ";
    printqueueRR(waiting);
    std::cout << "DEBUG --> Unarrived: ";
    printqueueRR(unarrived);
#endif
        if(arrivaltime == -1 && waitingtime == -1){ //finish cpu time
            if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
            t+= cputime;
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
            delete incpu;
            incpu = NULL;
        } else if(arrivaltime == -1 && cputime == -1){ //finish waiting
            t += waitingtime;
            for(auto& w : waiting) w.decreasewaittime(waitingtime);
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            printiofinRR(waiting,ready,t);
            waiting.pop_front();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                    if(waitingtime == -1){
                        printiofinRR(waiting, ready, t-1);
                    } else {
                        printiofinRR(waiting, ready, t);
                    }
                    waiting.pop_front();
                }
            }
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            t = arrivaltime;
            ready.push_back(Process(unarrived.front()));
            std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
            printqueueRR(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                if(incpu->getslice() < waitingtime) if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                // std::cout << "i hate switches" << std::endl;
                t += waitingtime;
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else if(waitingtime > cputime){
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                t += cputime;
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
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
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
                delete incpu;
                incpu = NULL;
            } else { //equals case im cry :(
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                t+= cputime;
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
                ready.push_back(Process(waiting.front()));
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                        if(waitingtime == -1){
                            printiofinRR(waiting, ready, t-1);
                        } else {
                            printiofinRR(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                t += tcs/2;
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
                printqueueRR(ready);
                unarrived.pop_front();
            } else {
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                t+= cputime;
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
                waitingtime = waiting.front().getcurrentwait();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
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
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& w : waiting) w.decreasewaittime(gap);
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueRR(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
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
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                t+= cputime;
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
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
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
                if(incpu->getslice() < waitingtime) if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                t += waitingtime;
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
                printiofinRR(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
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
                for(auto& w : waiting) w.decreasewaittime(gap);
                incpu->decreaseruntime(gap);
                ready.push_back(Process(unarrived.front()));
                std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " arrived; added to ready queue ";
                printqueueRR(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioRR(incpu,ready,waiting,t,tcs,tslice)) continue;
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
}


#endif