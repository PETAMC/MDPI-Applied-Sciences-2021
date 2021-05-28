#include <iostream>

#include <hardware/tile.hpp>

const sc_core::sc_time privatereaddelay( 9, sc_core::SC_NS); // \_ per token
const sc_core::sc_time privatewritedelay(7, sc_core::SC_NS); // /

Tile::Tile(sc_core::sc_module_name name, unsigned int maxiterations)
    : core::Master(sc_core::sc_module_name(name))
    , PrivateMemory(std::string(static_cast<const char*>(name))+".PM", 0x00000000, 32*1024, privatereaddelay, privatewritedelay)
    , maxiterations(maxiterations)
    , name(std::string(static_cast<const char*>(name)))
{
}
Tile::Tile(sc_core::sc_module_name name, unsigned int maxiterations, Monitor &monitor)
    : core::Master(sc_core::sc_module_name(name))
    , PrivateMemory(std::string(static_cast<const char*>(name))+".PM", 0x00000000, 32*1024, privatereaddelay, privatewritedelay, monitor)
    , maxiterations(maxiterations)
    , monitor(&monitor)
    , name(std::string(static_cast<const char*>(name)))
{
}



Tile& Tile::operator<< (Actor& actor)
{
    this->actors.push_back(&actor);
    actor.ChangeTile(this);
    return *this;
}



void Tile::Execute()
{
    for(auto actor : this->actors)
        actor->Initialize();

    if(this->monitor)
        this->monitor->ExpandTrace(this->name, "ACTIVE");   // Processing Element is active now

    for(unsigned int i=0; i<this->maxiterations; i++)
    {
        for(auto actor : this->actors)
            actor->Execute();
    }
}



void Tile::WriteWord(sc_dt::uint64 addr, unsigned int* word)
{
    auto privatebegin = this->PrivateMemory::GetAddress();
    auto privateend   = this->PrivateMemory::GetAddress() + this->PrivateMemory::GetSize();

    if(addr >= privatebegin and addr < privateend)  // Access Private Memory
    {
        this->PrivateMemory::Write(addr, *word);
    }
    else                                            // Access Interconnect
    {
        this->core::Master::WriteWord(addr, word);
    }
}



void Tile::ReadWord( sc_dt::uint64 addr, unsigned int* word)
{
    auto privatebegin = this->PrivateMemory::GetAddress();
    auto privateend   = this->PrivateMemory::GetAddress() + this->PrivateMemory::GetSize();
    unsigned long retval;

    if(addr >= privatebegin and addr < privateend)  // Access Private Memory
    {
        *word = this->PrivateMemory::Read(addr);
    }
    else                                            // Access Interconnect
    {
        this->core::Master::ReadWord(addr, word);
    }
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

