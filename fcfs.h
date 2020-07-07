#ifndef __fcfs_h
#define __fcfs_h

#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include "process.h"

void fcfs(std::ofstream& outfile, int n, long int seed);
std::vector<Process> processcreator(int n, long int seed);

std::vector<Process> processcreator(int n, long int seed){
    std::vector<Process> ret;
    srand48(seed);


    return ret;

}

void fcfs(std::ofstream& outfile, int n, long int seed){
    std::vector<Process> p = processcreator(seed,n);
    outfile << "Algorithm FCFS\n"; //write to file test... working
    int t = 0; //elapsed time

}


#endif