/*
name - rcsid

Ryan Prashad - prashr
Jackson Rothgeb - rothgj
Emily Martsolf - martse
*/
#include <iostream>
#include <string>
#include <fstream>

#include "fcfs.h"
#include "sjf.h"
#include "srt.h"
#include "rr.h"
#include "process.h"

int main(int argc, char** argv){
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argc < 8){
        std::cerr << "ERROR: Correct Usage --> ./a.o [n] [seed] [lambda] [upper_bound] [t_cs] [alpha] [t_slice] [rr_add (OPTIONAL)]" << std::endl;
        return EXIT_FAILURE;
    }
    std::ofstream outfile("simout.txt");
    int n = atoi(argv[1]);
    long int seed = atol(argv[2]); //long int from manpage
    float lambda = atof(argv[3]);
    int upperbound = atoi(argv[4]);
    int tcs = atoi(argv[5]); //positive even int
    int alpha = atoi(argv[6]); //correct type??
    int tslice = atoi(argv[7]);
    std::string rradd("END");
    if(argc == 9) rradd = argv[8];

    Process test("A", 2,3,4,5); //process test
    std::cout << test.getarrivaltime() << test.getbursts() << test.getcputime() << test.getiotime() << test.getname() << std::endl;
    
    fcfs(outfile, n, seed);
    sjftest();
    srttest();
    rrtest();


    outfile.close();
    return EXIT_SUCCESS;
}