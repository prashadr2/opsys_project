#ifndef __projectprocess_
#define __projectprocess_

#include <iostream>
#include <string>

class Process{
    public:
    Process(std::string zname, int zarrival_time, int zbursts, int zcpu_time, int zio_time){
        name = zname;
        arrival_time = zarrival_time;
        bursts = zbursts;
        cpu_time = zcpu_time;
        io_time = zio_time;
    }
    
    std::string getname() {return name;}
    int getarrivaltime() {return arrival_time;}
    int getbursts() {return bursts;}
    int getcputime() {return cpu_time;}
    int getiotime() {return io_time;}

    private:
    std::string name;
    int arrival_time;
    int bursts;
    int cpu_time;
    int io_time; //if this is -1, that means no io burst time is here....
};
#endif