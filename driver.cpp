/*
name - rcsid

Ryan Prashad - prashr
Jackson Rothgeb - rothgj
Emily Martsolf - martse
*/
#include <iostream>
#include <string>

#include "fcfs.h"

int main(int argc, char** argv){
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argc < 8) return EXIT_FAILURE;
    int n = (int)argv[1];
    long int seed = (long int)argv[2]; //long int from manpage
    double lambda = (double)argv[3];
    int upperbound = (int)argv[4];
    int tcs = (int)argv[5]; //positive even int
    int alpha = (int)argv[6]; //correct type??
    int tslice = (int)argv[7];
    std::string rradd("END");
    if(argc == 9) rradd = argv[8];
    
    test();

    return EXIT_SUCCESS;
}