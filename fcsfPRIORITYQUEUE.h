#ifndef __fcfs_h
#define __fcfs_h

//cpp includes
#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
//c includes
#include <stdlib.h>
#include <math.h>
//custom includes
#include "process.h"

void fcfs(std::ofstream& outfile, std::vector<Process>& p);

void fcfs(std::ofstream& outfile, std::vector<Process>& p){
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //time
    auto comparator = [](Process p1, Process p2) {return p1.getarrivaltime() > p2.getarrivaltime();}; //compare arrival times
    std::priority_queue<Process, std::vector<Process>, decltype(comparator)> qprocess(comparator); 
    //comparator lambda should sort from LEAST arrival time to GREATEST beacuse the LEAST time is first time aka top of heap

    // Process a("A", 1,1,std::vector<int>(), std::vector<int>());
    // Process b("B", 2,1,std::vector<int>(), std::vector<int>());
    // Process c("C", 3,1,std::vector<int>(), std::vector<int>());
    // Process d("D", 4,1,std::vector<int>(), std::vector<int>());
    // std::vector<Process> tester;
    // tester.push_back(a);
    // tester.push_back(b);
    // tester.push_back(c);
    // tester.push_back(d);
    // for(Process pp : tester) qprocess.push(pp);
    // while(!qprocess.empty()){
    //     std::cout << qprocess.top().getname() << std::endl;
    //     qprocess.pop();
    // }
    
    for(Process pp : p) qprocess.push(p);
    std::cout << "time <" << t << ">ms: Start of simulation [Q ";
    std::priority_queue<Process, std::vector<Process>, decltype(comparator)> printstart(comparator)

    // for(int z = 0; z < p.size(); z++){
    //     std::cout << "name: " << p[z].getname() << "\narrival_time: " << p[z].getarrivaltime() << "\nbursts: " << p[z].getbursts() << std::endl;
    //     std::vector<int> testcpu = p[z].getcputime();
    //     std::vector<int> testio = p[z].getiotime();
    //     for(int y = 0; y < p[z].getcputime().size(); y++) std::cout << p[z].getname() << " cpu time: " << testcpu[y] << " io time: " << testio[y] << std::endl;
    //     std::cout << std::endl;
    // }

//         std::cout << "time <" << t << ">ms: Start of simulation [Q";
//     for(Process pp : sortedp) {
//         std::cout << ' ' << pp.getname();
// #ifdef DEBUG_MODE
//     std::cout << ",arrivaltime: " <<  pp.getarrivaltime() << " ||";
// #endif
//     }
//     std::cout << ']' << std::endl;
// }

//jackson legacy code lol

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