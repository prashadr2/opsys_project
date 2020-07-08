#ifndef __projectprocess_
#define __projectprocess_

//cpp includes
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
    
    std::string getname() const {return name;}
    int getarrivaltime() const {return arrival_time;}
    int getbursts() const {return bursts;}
    std::vector<int> getcputime() const {return cpu_time;}
    std::vector<int> getiotime() const {return io_time;}

    private:
    std::string name;
    int arrival_time;
    int bursts;
    std::vector<int> cpu_time;
    std::vector<int> io_time; //if this is -1, that means no io burst time is here
};

#endif