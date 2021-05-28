#include <hardware/memory.hpp>

unsigned int Memory::GetSize() const
{
    return this->size;
}

uint64_t Memory::GetAddress() const
{
    return this->baseaddress;
}



unsigned long Memory::Read(uint64_t address) const
{
    uint64_t index = address - this->baseaddress;
    if(index >= this->memory.size())
    {
        std::cerr << "\e[1;31mERROR:\e[0m Memory read access out of range. Attempt index: " << index << "; Memory size: " << this->memory.size() << std::endl;
        if(this->monitor)
            this->monitor->ExpandTrace(this->name, "ERROR");
        return 0;
    }

    if(this->monitor)
        this->monitor->ExpandTrace(this->name, "R");

    sc_core::wait(this->readdelay);

    if(this->monitor)
        this->monitor->ExpandTrace(this->name, "IDLE");

    return this->memory[index];
}
void Memory::Write(uint64_t address, unsigned long word)
{
    uint64_t index = address - this->baseaddress;
    if(index >= this->memory.size())
    {
        std::cerr << "\e[1;31mERROR:\e[0m Memory write access out of range. Attempt index: " << index << "; Memory size: " << this->memory.size() << std::endl;
        if(this->monitor)
            this->monitor->ExpandTrace(this->name, "ERROR");
        return;
    }

    if(this->monitor)
        this->monitor->ExpandTrace(this->name, "W");

    sc_core::wait(this->writedelay);
    this->memory[index] = word;

    if(this->monitor)
        this->monitor->ExpandTrace(this->name, "IDLE");
}



unsigned long long Memory::AllocateMemory(size_t numofwords)
{
    if(this->used + numofwords > this->size)
        return 0;

    unsigned long long newaddress;
    newaddress  = this->baseaddress + this->used;
    this->used += numofwords;

    return newaddress;
}



Memory& Memory::operator<< (Channel& channel)
{
    unsigned int memorysize = channel.fifosize + 2; // +2 for usage and firsttoken token
    if(this->used + memorysize > this->size)
        return *this;

    // calculate and assign new address for channel
    channel.usageaddress = this->AllocateMemory(1);
    channel.indexaddress = this->AllocateMemory(1);
    channel.fifoaddress  = this->AllocateMemory(channel.fifosize);

    // remember channel
    this->channels.push_back(&channel);
    return *this;
}



Channel* Memory::GetChannelByOffset(unsigned long long offset)
{
    for(size_t i = 0; i < this->channels.size(); i++)
    {
        Channel* channel;
        unsigned long long channeloffset;
        unsigned int channelsize;

        channel       = this->channels[i];
        channeloffset = channel->usageaddress - this->baseaddress;  // usage is the first address managed by the SM
        channelsize   = channel->fifosize + 2;  // + usage and findex address
        if(channeloffset <= offset && channeloffset + channelsize > offset)
            return channel;
    }
    return NULL;
}



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

