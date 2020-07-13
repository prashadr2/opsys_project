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
        tau = -1;
        previousBurst = -1;
    }

    Process(std::string zname, int zarrival_time, int zbursts, std::list<int> zcpu_time, std::list<int> zio_time, int ztau, int zpreviousBurst){
        name = zname;
        arrival_time = zarrival_time;
        bursts = zbursts;
        cpu_time = zcpu_time;
        io_time = zio_time;
        tau = ztau;
        previousBurst = zpreviousBurst;
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
        this->tau = p.getTau();
        this->previousBurst = p.getPreviousBurst();
    }
    
    std::string getname() const {return name;}

    int getarrivaltime() const {return arrival_time;}

    int getbursts() const {return bursts;}
    void decreaseburst() {bursts--;}
    //calling front on an empty list is big worries also (undefined behavior)....
    //use shortcutting to get around this ---> (!waiting.empty() && someconditon > waiting.front.getcurrentwait())
    std::list<int> getiotime() const {return io_time;}
    int getcurrentwait() {return io_time.front();}
    void movenextwait() {io_time.pop_front();}
    void decreasewaittime(int d) {*(io_time.begin()) -= d;}

    std::list<int> getcputime() const {return cpu_time;}
    int getcurrentruntime() {return cpu_time.front();}
    void movenextruntime() {cpu_time.pop_front();}
    void decreaseruntime(int d) {*(cpu_time.begin()) -= d;}
    
    void setPreviousBurst(int b) {previousBurst = b;}
    int getPreviousBurst() const {return previousBurst;}

    void setTau(int tt) {tau = tt;}
    int getTau() const {return tau;}
    void recalculateTau(int alpha) {tau = (alpha * previousBurst) + ((1 - alpha) * tau);}

    private:
    std::string name;
    int arrival_time;
    int bursts;
    std::list<int> cpu_time;
    std::list<int> io_time; //if the last element is -2, that means no io burst time is here
    int tau;
    int previousBurst;
};

#endif
