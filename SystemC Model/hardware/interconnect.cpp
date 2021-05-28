#include <iostream>

#include <hardware/interconnect.hpp>
#include <hardware/tile.hpp>
#include <hardware/memory.hpp>

Interconnect& Interconnect::operator<< (Tile& tile)
{
    tile.initiator_socket(this->target_socket);
    return *this;
}



Interconnect& Interconnect::operator<< (SharedMemory& sharedmemory)
{
    this->initiator_socket(sharedmemory.target_socket);
    sc_dt::uint64 address = sharedmemory.GetAddress();
    unsigned int  size    = sharedmemory.GetSize();
    this->starts.push_back(address);
    this->ends.push_back(address + size);
    return *this;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

