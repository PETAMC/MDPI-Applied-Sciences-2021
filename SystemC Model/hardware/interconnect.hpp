#ifndef INTERCONNECT_HPP
#define INTERCONNECT_HPP

#include <core/bus.hpp>
class Tile;
class SharedMemory;

class Interconnect
    : public core::Bus
{
    public:
        Interconnect(const char* name)
            : core::Bus(name) {};

        virtual Interconnect& operator<< (Tile& tile);
        virtual Interconnect& operator<< (SharedMemory& sharedmemory);
};


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

