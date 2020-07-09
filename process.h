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

    Process(const Process& p){ //shallow copies cause big worries later on
        this->name = p.getname();
        this->arrival_time = p.getarrivaltime();
        this->bursts = p.getbursts();
        std::list<int> cpycpu;
        std::list<int> cpyio;
        for(auto const& data : p.getcputime()) cpycpu.push_back(data);
        for(auto const& data : p.getiotime()) cpyio.push_back(data);
        this->cpu_time = cpycpu;
        this->io_time = cpyio;
    }
    
    std::string getname() const {return name;}
    
    int getarrivaltime() const {return arrival_time;}

    int getbursts() const {return bursts;}
    void decreaseburst() {bursts--;}

    std::list<int> getiotime() const {return io_time;}
    int getcurrentwait() {return io_time.front();}
    void movenextwait() {io_time.pop_front();}

    std::list<int> getcputime() const {return cpu_time;}
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