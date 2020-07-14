/*
name - rcsid

Ryan Prashad - prashr
Jackson Rothgeb - rothgj
Emily Martsolf - martse
*/

//cpp includes
#include <iostream>
#include <string>
#include <fstream>
//c includes
#include <math.h>
//custom includes
#include "fcfs.h"
#include "sjf.h"
#include "srt.h"
#include "rr.h"
#include "process.h"

double nextrandnum(double lambda, int upperbound){
    double r = drand48();
    double x = -log(r) / lambda;
    while(x > (double)upperbound){
        r = drand48();
        x = -log(r) / lambda;
    }
    return x;
}

int main(int argc, char** argv){
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argc < 8){
        std::cerr << "ERROR: Correct Usage --> ./a.o [n] [seed] [lambda] [upper_bound] [t_cs] [alpha] [t_slice] [rr_add (OPTIONAL)]" << std::endl;
        return EXIT_FAILURE;
    }
    std::ofstream outfile("simout.txt");
    const std::string names("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    int n = atoi(argv[1]);
    long int seed = atol(argv[2]); //long int from manpage
    double lambda = (double)atof(argv[3]);
    int upperbound = atoi(argv[4]);
    int tcs = atoi(argv[5]); //positive even int
    double alpha = (double)atof(argv[6]); //correct type??
    int tslice = atoi(argv[7]);
    std::string rradd("END");
    if(argc == 9) rradd = argv[8];
    
    //start process setup...
    std::vector<Process> psetup;
    srand48(seed);
    for(int z = 0; z < n; z++){
        std::string name = names.substr(z,1);
        std::list<int> cputimes;
        std::list<int> iotimes;
        int arrivaltime = (int)floor(nextrandnum(lambda,upperbound));
        int burstamt = (int)trunc(drand48() * 100) + 1;
        for(int y = 0; y < burstamt; y++){
            cputimes.push_back((int)ceil(nextrandnum(lambda,upperbound)));
            if(y == burstamt - 1){ //last cpu burst
                iotimes.push_back(-2); //-1 val means NO ioburst on this run... make sure to reflect this in algorithms!!
            } else {
                iotimes.push_back((int)ceil(nextrandnum(lambda,upperbound)));
            }
        }
        psetup.push_back(Process(name,arrivaltime,burstamt,cputimes,iotimes));
    }

    // fcfs(outfile, psetup, tcs);
    // std::cout << std::endl;
    sjf(outfile, psetup, tcs, alpha, lambda);
    std::cout << std::endl;
    srttest();
    std::cout << std::endl;
    rrtest();
    std::cout << std::endl;


// g++ -g -D DEBUG_MODE -o a.o driver.cpp && ./a.o 8 64 0.001 4096 4 0.5 2048

// Test 2     ./a.out 1 2 0.01 256 4 0.5 128

// Test 3     ./a.out 2 2 0.01 256 4 0.5 128

// Test 4     ./a.out 16 2 0.01 256 4 0.75 64

// Test 5     ./a.out 8 64 0.001 4096 4 0.5 2048
//rm -rf .git/refs/remotes/origin/
    outfile.close();
    return EXIT_SUCCESS;
}