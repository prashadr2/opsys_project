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

}


#endif