#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>
#include <string>
#include <core/slave.hpp>
#include <software/channel.hpp>
#include <monitor.hpp>


class Memory
{
    public:
        Memory(std::string name, uint64_t address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay, Monitor &monitor)
            : baseaddress(address), size(size), used(0), channels(0)
            , memory(size, 0)
            , readdelay(readdelay)
            , writedelay(writedelay)
            , name(name)
            , monitor(&monitor)
            {};
        Memory(std::string name, uint64_t address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay)
            : baseaddress(address), size(size), used(0), channels(0)
            , memory(size, 0)
            , readdelay(readdelay)
            , writedelay(writedelay)
            , name(name)
            , monitor(nullptr)
            {};

        unsigned int GetSize() const; // in words
        uint64_t GetAddress() const;

        virtual unsigned long Read(uint64_t address) const;
        virtual void Write(uint64_t address, unsigned long word);

        Memory& operator<< (Channel& channel); 


    private:
        std::string name;
        unsigned long long AllocateMemory(size_t numbytes);
        Channel* GetChannelByOffset(unsigned long long offset);

        size_t size; // actual size in words
        size_t used; // used words
        uint64_t baseaddress;
        std::vector<Channel*> channels;
        std::vector<unsigned long> memory;

        Monitor *monitor;   // Can be NULL!

        sc_core::sc_time readdelay;
        sc_core::sc_time writedelay;
};


// Shared Memory ///////////////


class SharedMemory : public core::Slave, public Memory
{
    public:
        SharedMemory(std::string name, sc_dt::uint64 address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay)
            : core::Slave(name.c_str())
            , Memory(name, address, size, readdelay, writedelay)
            {};
        SharedMemory(std::string name, sc_dt::uint64 address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay, Monitor &monitor)
            : core::Slave(name.c_str())
            , Memory(name, address, size, readdelay, writedelay, monitor)
            {};
        virtual unsigned long Read(uint64_t address) const
        {
            return this->Memory::Read(address);
        }
        virtual void Write(uint64_t address, unsigned long word)
        {
            this->Memory::Write(address, word);
        }
};


// Private Memory ///////////////


class PrivateMemory : public Memory
{
    public:
        PrivateMemory(std::string name, sc_dt::uint64 address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay)
            : Memory(name, address, size, readdelay, writedelay)
            {};
        PrivateMemory(std::string name, sc_dt::uint64 address, size_t size, sc_core::sc_time readdelay, sc_core::sc_time writedelay, Monitor &monitor)
            : Memory(name, address, size, readdelay, writedelay, monitor)
            {};
};

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

