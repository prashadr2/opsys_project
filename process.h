#ifndef __projectprocess_
#define __projectprocess_

#include <iostream>
#include <vector>
#include <string>

class Process{
    public:
    Process(std::string zname, int zarrival_time, int zbursts, std::vector<int> zcpu_time, std::vector<int> zio_time){
        name = zname;
        arrival_time = zarrival_time;
        bursts = zbursts;
        cpu_time = zcpu_time;
        io_time = zio_time;
    }
    
    std::string getname() {return name;}
    int getarrivaltime() {return arrival_time;}
    int getbursts() {return bursts;}
    std::vector<int> getcputime() {return cpu_time;}
    std::vector<int> getiotime() {return io_time;}

    private:
    std::string name;
    int arrival_time;
    int bursts;
    std::vector<int> cpu_time;
    std::vector<int> io_time; //if this is -1, that means no io burst time is here....
};
#endif