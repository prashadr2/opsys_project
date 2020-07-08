#ifndef __projectprocess_
#define __projectprocess_

//cpp includes
#include <iostream>
#include <vector>
#include <string>
#include <list>

class Process{
    public:
    Process(std::string zname, int zarrival_time, int zbursts, std::list<int> zcpu_time, std::list<int> zio_time){
        name = zname;
        arrival_time = zarrival_time;
        bursts = zbursts;
        cpu_time = zcpu_time;
        io_time = zio_time;
    }
    
    std::string getname() const {return name;}
    int getarrivaltime() const {return arrival_time;}
    int getbursts() const {return bursts;}
    std::list<int> getcputime() const {return cpu_time;}
    std::list<int> getiotime() const {return io_time;}

    int getcurrentwait() {return io_time.front();}
    void movenextwait() {io_time.pop_front();}

    int getcurrentruntime() {return cpu_time.front();}
    void movenextruntime() {cpu_time.pop_front();}

    private:
    std::string name;
    int arrival_time;
    int bursts;
    std::list<int> cpu_time;
    std::list<int> io_time; //if this is -1, that means no io burst time is here
    bool waiting;
};

#endif