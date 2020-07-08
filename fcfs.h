#ifndef __fcfs_h
#define __fcfs_h

//cpp includes
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
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
    std::vector<Process> sortedp(p);
    std::sort(sortedp.begin(), sortedp.end(), [](Process p1, Process p2) -> bool {return p1.getarrivaltime() < p2.getarrivaltime();});
    
    std::queue<Process, std::deque<Process> > readyq(std::deque<Process>(sortedp.begin(), sortedp.end()));
    std::queue<Process> waitingq;
    std::queue<Process> runningq; //does this need to be a queue? does the cpu only run one process at a time??

    std::cout << "time <" << t << ">ms: Start of simulation [Q";
    for(Process pp : sortedp) {
        std::cout << ' ' << pp.getname();
#ifdef DEBUG_MODE
    std::cout << ",arrivaltime: " <<  pp.getarrivaltime() << " ||";
#endif
    }
    std::cout << ']' << std::endl;
}

//rm -rf .git/refs/remotes/origin/
//./a.o 5 231313123213123123 0.001343232423 545 5 5 5
//g++ -D DEBUG_MODE -o a.o driver.cpp
#endif