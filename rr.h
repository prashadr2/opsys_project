#ifndef __roundrobin_h
#define __roundrobin_h

//cpp includes
#include <iostream>
#include <vector>

void rrtest();
//For a preemption during the RR algorithm, the process always goes to the end of the queue.
void rrtest(){
    std::cout << "void rr test" << std::endl;
}


#endif