#ifndef FCFS_INTERCONNECT_HPP
#define FCFS_INTERCONNECT_HPP

#include <hardware/interconnect.hpp>

class FCFSInterconnect
    : public Interconnect
{
    public:
        FCFSInterconnect(const char* name)
            : Interconnect(name) {};

};


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

