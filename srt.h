#ifndef __srt_h
#define __srt_h

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

void srt(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, const double alpha, const int lambda);
void printcpufinSRT(Process* incpu, int t, const int tcs, std::list<Process>& ready);
void printiofinSRT(std::list<Process>& waiting, std::list<Process>& ready, int t);
void printqueueSRT(std::list<Process>& printer);
bool preemption_ioSRT(Process*& incpu, std::list<Process>& waiting, std::list<Process>& ready, int& t, int tcs);
bool preemption_willSRT(Process*& incpu, std::list<Process>& waiting, std::list<Process>& ready, int& t, int tcs);

void printqueueSRT(std::list<Process>& printer){ //THIS FUNCTION PRINTS A NEWLINE CHAR!!!
  std::cout << "[Q";
  if(printer.size() == 0){
      std::cout << " <empty>]" << std::endl;
      return;
  }
  for(std::list<Process>::iterator z = printer.begin(); z != printer.end(); z++) {
      std::cout << ' ' << z->getname();
#ifdef DEBUG_MODE
    std::cout << "-->(waittime: " << z->getcurrentwait() << ", tau: " << z->getTau() << "), ";
#endif
  }
  std::cout << "]" << std::endl;
}

void printcpufinSRT(Process* incpu, int t, const int tcs, std::list<Process>& ready, const double alpha){
    if (t > 999) {incpu->recalculateTau(alpha); return;}
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) completed a CPU burst; " << incpu->getbursts();
    if(incpu->getbursts() == 1) std::cout << " burst to go "; else std::cout << " bursts to go ";
    printqueueSRT(ready);
    incpu->recalculateTau(alpha);
    std::cout << "time " << t << "ms: Recalculated tau = " << incpu->getTau() << "ms for process " << incpu->getname() << ' ';
    printqueueSRT(ready);
    std::cout << "time " << t << "ms: Process " << incpu->getname() << " switching out of CPU; will block on I/O until time " << t+incpu->getcurrentwait()+(tcs/2) << "ms ";
    printqueueSRT(ready);
}

void printiofinSRT(std::list<Process>& waiting, std::list<Process>& ready, int t){
    if (t > 999) return;
    std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " (tau " << waiting.front().getTau() << "ms) completed I/O; added to ready queue ";
    printqueueSRT(ready);
}

bool preemption_ioSRT(Process*& incpu, std::list<Process>& ready, std::list<Process>& waiting, int& t, int tcs, int& preemptions){ //call when finish IO and ready.push_back
    //ready.back() is the process to preempt...
    if(ready.back().getTau() - (ready.back().getPreviousBurst() - ready.back().getcurrentruntime()) >= incpu->getTau() - (incpu->getPreviousBurst() - incpu->getcurrentruntime())) return false;
    if(ready.size() > 1) ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
    if (t <= 999) std::cout << "time " << t << "ms: Process " << ready.front().getname() << " (tau " << ready.front().getTau() << "ms) completed I/O; preempting " << incpu->getname() << ' ';
    if (t <= 999) printqueueSRT(ready);
    waiting.pop_front();
#ifdef DEBUG_MODE
    if(!waiting.empty()) std::cout << "preempt_waittime: " << waiting.front().getcurrentwait() << std::endl;
#endif
    bool pushed = false;
    if(!waiting.empty()){
        int waitingtime = waiting.front().getcurrentwait();
        if(waitingtime <= 0) {
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
            if(waitingtime == -1){
                printiofinSRT(waiting, ready, t-1);
            } else {
                printiofinSRT(waiting, ready, t);
            }
            waiting.pop_front();
        }
    }
    t += tcs/2;
    preemptions++;
    for(auto& r : ready) r.addwaittime(tcs/2);
    if(!waiting.empty() && waiting.front().getcurrentwait() == 2) {ready.push_back(*incpu); pushed = true;}
    for(auto& w : waiting) w.decreasewaittime(tcs/2);
    if(!waiting.empty()){
        int waitingtime = waiting.front().getcurrentwait();
        if(waitingtime <= 0) {
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
            if(waitingtime == -1){
                printiofinSRT(waiting, ready, t-1);
            } else {
                printiofinSRT(waiting, ready, t);
            }
            waiting.pop_front();
        }
    }
    if(!pushed) ready.push_back(*incpu);
    if(ready.size() > 1) ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
    delete incpu;
    incpu = NULL;
    return true;
}

bool preemption_willSRT(Process*& incpu, std::list<Process>& ready, std::list<Process>& waiting, int& t, int tcs, int& preemptions){
    //ready.back() is the process to preempt...
    if(ready.back().getTau() - (ready.back().getPreviousBurst() - ready.back().getcurrentruntime()) >= incpu->getTau() - (incpu->getPreviousBurst() - incpu->getcurrentruntime())) {t += 1; return false;}
    if(ready.size() > 1) ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
    if (t <= 999) std::cout << "time " << t << "ms: Process " << ready.front().getname() << " (tau " << ready.front().getTau() << "ms) completed I/O; added to ready queue ";
    if (t <= 999) printqueueSRT(ready);
    waiting.pop_front();
    t += tcs/2;
    preemptions++;
    for(auto& r : ready) r.addwaittime(tcs/2);
    if (t <= 999) std::cout << "time " << t-1 << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
    if (t <= 999) printqueueSRT(ready);
    if (t <= 999) std::cout << "time " << t-1 << "ms: Process " << ready.front().getname() << " (tau " << ready.front().getTau() << "ms) will preempt " << incpu->getname() << ' ';
    if (t <= 999) printqueueSRT(ready);
    for(auto& w : waiting) w.decreasewaittime(tcs/2);
    if(!waiting.empty()){
        int waitingtime = waiting.front().getcurrentwait();
        if(waitingtime <= 0) {
            waiting.front().movenextwait();
            ready.push_back(Process(waiting.front()));
            ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
            if(waitingtime == -1){
                printiofinSRT(waiting, ready, t-1);
            } else {
                printiofinSRT(waiting, ready, t);
            }
            waiting.pop_front();
        }
    }
    ready.push_back(*incpu);
    if(ready.size() > 1) ready.sort([](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());});
    delete incpu;
    incpu = NULL;
    t+=1;
    return true;
}

void srt(std::ofstream& outfile, const std::vector<Process>& p, const int tcs, const double alpha, const double lambda){
    outfile << "Algorithm SRT\n"; //write to file test... working
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
    auto comptau = [](Process p1, Process p2) -> bool {if(p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) == p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime())) return p1.getname() < p2.getname(); else return p1.getTau() - (p1.getPreviousBurst() - p1.getcurrentruntime()) < p2.getTau() - (p2.getPreviousBurst() - p2.getcurrentruntime());};
    for(Process z : p){
        Process pusher(z);
        pusher.setTau(tau);
        pusher.setPreviousBurst(pusher.getcurrentruntime());
        unarrived.push_back(pusher);
    }
    unarrived.sort(compname);

    int burstcount = 0, bursttotal = 0, preemptions = 0;
    for(auto const& pp : p) for(int z : pp.getcputime()) {burstcount++; bursttotal += z;}
    double cpuavg = (double)bursttotal / (double)burstcount;
    outfile << "-- average CPU burst time: " << std::setprecision(3) << std::fixed << cpuavg <<  " ms\n";

    for(std::list<Process>::iterator z = unarrived.begin(); z != unarrived.end(); z++){
        std::cout << "Process " << z->getname() << " [NEW] (arrival time " << z->getarrivaltime() << " ms) " << z->getbursts() << " CPU burst";
        if(z->getbursts() == 1) std::cout << " (tau " << z-> getTau() << "ms)" << std::endl; else std::cout << "s (tau " << z-> getTau() << "ms)" << std::endl;
    }
    std::cout << "time 0ms: Simulator started for SRT ";
    printqueueSRT(ready);
    unarrived.sort(comparrival); //sorting for actual program

    while(!ready.empty() || !waiting.empty() || !unarrived.empty() || incpu != NULL){
#ifdef DEBUG_MODE
    if(t > 34600) break;
#endif
        if(waiting.size() > 1) waiting.sort(compcurwait);
        if(ready.size() > 1) ready.sort(comptau);

        int waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1; //if this val is -1, wait queue is empty, if it is -2, TERMINATE THE PROCESS!!!!
        int arrivaltime = !unarrived.empty() ? unarrived.front().getarrivaltime() : -1;
        int cputime = (incpu != NULL) ? incpu->getcurrentruntime() : -1; //MAKE SURE TO FREE INCPU AND SET TO NULL AFTER WE COPY IT INTO WAITING QUEUE (waitingqueue.push_back(Process(*incpu)))
        
        if(waitingtime <= -99999){
            std::cout << "time " << t << "ms: Process " << waiting.front().getname() << " terminated "; 
            printqueueSRT(ready);
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
            // std::cout << "triggered @ t = " << t << ", waitt: " << waitingtime << std::endl;
            if(arrivaltime != -1 && arrivaltime < t){ //recently added
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                if (t <= 999) std::cout << "time " << arrivaltime << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                if (t <= 999) printqueueSRT(ready);
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
                        // std::cout << "switch1" << std::endl;
                        ready.push_back(Process(waiting.front()));
                        if(ready.back().getTau() > incpu->getTau()){
                            if(preemption_ioSRT(incpu,ready,waiting,t,tcs, preemptions)) continue;
                        } else {
                            t-=1;
                            if(preemption_willSRT(incpu,ready,waiting,t,tcs, preemptions)) {continue;}
                        }
                        // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                        ready.sort(comptau);
                        printiofinSRT(waiting, ready, t-1);
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
                        if (t <= 999) printqueueSRT(ready);
                    } else {
                        // std::cout << "switch2" << std::endl;
                        if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
                        if (t <= 999) printqueueSRT(ready);
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs, preemptions)) continue;
                        ready.sort(comptau);
                        printiofinSRT(waiting, ready, t);
                        
                    }
                    waiting.pop_front();
                } else {
                    // std::cout << "switch3" << std::endl;
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
                    if (t <= 999) printqueueSRT(ready);
                }
            } else {
                // std::cout << "switch4" << std::endl;
                if (t <= 999) std::cout << "time " << t << "ms: Process " << incpu->getname() << " (tau " << incpu->getTau() << "ms) started using the CPU with " << incpu->getcurrentruntime() << "ms burst remaining ";
                if (t <= 999) printqueueSRT(ready);
            }
            waitingtime = !waiting.empty() ? waiting.front().getcurrentwait() : -1;
        }
#ifdef DEBUG_MODE
    std::cout << "DEBUG -->waitingtime: " << waitingtime << " arrivaltime: " << abs(arrivaltime - t) << " cputime: " << cputime << " ▼▼▼▼▼▼▼▼▼" << std::endl;
    std::cout << "DEBUG -->cpu_address: " << incpu << std::endl;
    if (incpu != NULL) std::cout << "DEBUG --> incpu: " << incpu->getname() << std::endl;
    std::cout << "DEBUG --> Ready: ";
    printqueueSRT(ready);
    std::cout << "DEBUG --> Waiting: ";
    printqueueSRT(waiting);
    std::cout << "DEBUG --> Unarrived: ";
    printqueueSRT(unarrived);
#endif
        if(arrivaltime == -1 && waitingtime == -1){ //finish cpu time
            t+= cputime;
            for(auto& r : ready) r.addwaittime(cputime);
            incpu->movenextruntime();
            incpu->decreaseburst();
            if(incpu->getcurrentwait() <= -2) {
                waiting.push_back(Process(*incpu));
                delete incpu;
                incpu = NULL;
                continue;
            }
            printcpufinSRT(incpu,t,tcs,ready,alpha);
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
            printiofinSRT(waiting,ready,t);
            waiting.pop_front();
            if(!waiting.empty()){
                waitingtime = waiting.front().getcurrentwait();
                if(waitingtime <= 0) {
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                    ready.sort(comptau);
                    if(waitingtime == -1){
                        printiofinSRT(waiting, ready, t-1);
                    } else {
                        printiofinSRT(waiting, ready, t);
                    }
                    waiting.pop_front();
                }
            }
        } else if(waitingtime == -1 && cputime == -1){ //we have an arrival
            for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
            t = arrivaltime;
            ready.push_back(Process(unarrived.front()));
            if(ready.size() > 1) ready.sort(comptau);
            if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
            if (t <= 999) printqueueSRT(ready);
            unarrived.pop_front();
        } else if(arrivaltime == -1){ //no arrival, compare waiting/cpu
            if(waitingtime < cputime){
                // std::cout << "i hate switches" << std::endl;
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSRT(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else if(waitingtime > cputime){
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                 if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinSRT(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                delete incpu;
                incpu = NULL;
            } else { //equals case im cry :(
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                 if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinSRT(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                // t += tcs/2;

                // 

                // incpu->decreaseruntime(waitingtime);
                // for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSRT(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
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
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                if (t <= 999) printqueueSRT(ready);
                unarrived.pop_front();
            } else {
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                incpu->movenextruntime();
                incpu->decreaseburst();
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinSRT(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                waitingtime = waiting.front().getcurrentwait();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
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
                ready.push_back(Process(waiting.front()));
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSRT(waiting,ready,t);
                waiting.pop_front();
            } else {
                int gap = abs(arrivaltime - t);
                t = arrivaltime;
                for(auto& r : ready) r.addwaittime(gap);
                for(auto& w : waiting) w.decreasewaittime(gap);
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                if (t <= 999) printqueueSRT(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        // if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            }
        } else { //triple check 
            if(cputime < waitingtime && cputime < abs(arrivaltime - t)){
                t+= cputime;
                for(auto& r : ready) r.addwaittime(cputime);
                for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                incpu->movenextruntime();
                incpu->decreaseburst();
                if(incpu->getcurrentwait() <= -2) {
                    waiting.push_back(Process(*incpu));
                    delete incpu;
                    incpu = NULL;
                    continue;
                }
                printcpufinSRT(incpu,t,tcs,ready,alpha);
                incpu->setPreviousBurst(incpu->getcurrentruntime());
                waiting.push_back(Process(*incpu));
                t += tcs/2;
                for(auto& r : ready) r.addwaittime(tcs/2);
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
                delete incpu;
                incpu = NULL;
            } else if(waitingtime < cputime && waitingtime < abs(arrivaltime - t)){
                t += waitingtime;
                for(auto& r : ready) r.addwaittime(waitingtime);
                incpu->decreaseruntime(waitingtime);
                for(auto& w : waiting) w.decreasewaittime(waitingtime);
                waiting.front().movenextwait();
                ready.push_back(Process(waiting.front()));
                if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                if(ready.size() > 1) ready.sort(comptau);
                printiofinSRT(waiting,ready,t);
                waiting.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
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
                ready.push_back(Process(unarrived.front()));
                if(ready.size() > 1) ready.sort(comptau);
                if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                if (t <= 999) printqueueSRT(ready);
                unarrived.pop_front();
                if(!waiting.empty()){
                    waitingtime = waiting.front().getcurrentwait();
                    if(waitingtime <= 0) {
                        waiting.front().movenextwait();
                        ready.push_back(Process(waiting.front()));
                        if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                        ready.sort(comptau);
                        if(waitingtime == -1){
                            printiofinSRT(waiting, ready, t-1);
                        } else {
                            printiofinSRT(waiting, ready, t);
                        }
                        waiting.pop_front();
                    }
                }
            } else {
                // std::cout << "bad vibes" << std::endl;
                if(waitingtime == cputime){
                    t+= cputime;
                    for(auto& r : ready) r.addwaittime(cputime);
                    for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                    incpu->movenextruntime();
                    incpu->decreaseburst();
                    if(incpu->getcurrentwait() <= -2) {
                        waiting.push_back(Process(*incpu));
                        delete incpu;
                        incpu = NULL;
                        continue;
                    }
                    printcpufinSRT(incpu,t,tcs,ready,alpha);
                    incpu->setPreviousBurst(incpu->getcurrentruntime());
                    waiting.push_back(Process(*incpu));
                    // t += tcs/2;

                    // 

                    // incpu->decreaseruntime(waitingtime);
                    // for(auto& w : waiting) w.decreasewaittime(waitingtime);
                    waiting.front().movenextwait();
                    ready.push_back(Process(waiting.front()));
                    // if(preemption_ioSRT(incpu,ready,waiting,t,tcs)) continue;
                    if(ready.size() > 1) ready.sort(comptau);
                    printiofinSRT(waiting,ready,t);
                    waiting.pop_front();
                    if(!waiting.empty()){
                        waitingtime = waiting.front().getcurrentwait();
                        if(waitingtime <= 0) {
                            waiting.front().movenextwait();
                            ready.push_back(Process(waiting.front()));
                            if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                            ready.sort(comptau);
                            if(waitingtime == -1){
                                printiofinSRT(waiting, ready, t-1);
                            } else {
                                printiofinSRT(waiting, ready, t);
                            }
                            waiting.pop_front();
                        }
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
                    if(ready.size() > 1) ready.sort(comptau);
                    printiofinSRT(waiting,ready,t);
                    waiting.pop_front();
                    if(!waiting.empty()){
                        waitingtime = waiting.front().getcurrentwait();
                        if(waitingtime <= 0) {
                            waiting.front().movenextwait();
                            ready.push_back(Process(waiting.front()));
                            if(preemption_ioSRT(incpu,ready,waiting,t,tcs,preemptions)) continue;
                            ready.sort(comptau);
                            if(waitingtime == -1){
                                printiofinSRT(waiting, ready, t-1);
                            } else {
                                printiofinSRT(waiting, ready, t);
                            }
                            waiting.pop_front();
                        }
                    }

                    //arrival
                    
                    for(auto& r : ready) r.addwaittime(abs(arrivaltime - t));
                    t = arrivaltime;
                    ready.push_back(Process(unarrived.front()));
                    if(ready.size() > 1) ready.sort(comptau);
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                    if (t <= 999) printqueueSRT(ready);
                    unarrived.pop_front();
                } else if(cputime == abs(arrivaltime - t)){
                    t+= cputime;
                    for(auto& r : ready) r.addwaittime(cputime);
                    for(auto& w : waiting) w.decreasewaittime(cputime + (tcs/2));
                    incpu->movenextruntime();
                    incpu->decreaseburst();
                    if(incpu->getcurrentwait() <= -2) {
                        waiting.push_back(Process(*incpu));
                        delete incpu;
                        incpu = NULL;
                        continue;
                    }
                    printcpufinSRT(incpu,t,tcs,ready,alpha);
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
                    if(ready.size() > 1) ready.sort(comptau);
                    if (t <= 999) std::cout << "time " << t << "ms: Process " << unarrived.front().getname() << " (tau " << unarrived.front().getTau() << "ms) arrived; added to ready queue ";
                    if (t <= 999) printqueueSRT(ready);
                    unarrived.pop_front();

                } else {
                    // std::cout << "SUPER bad vibes" << std::endl;
                }
            }
        }
    }
    t -= tcs/2;
    std::cout << "time " << t << "ms: Simulator ended for SRT ";
    printqueueSRT(ready);

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