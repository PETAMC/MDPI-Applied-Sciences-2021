#include <core/bus.hpp>
#include <iostream>

namespace core
{

Bus::Bus(sc_core::sc_module_name name)
    : sc_core::sc_module(name)
    , target_socket("target_bus_socket")
    , initiator_socket("initiator_bus_socket")
{
    this->target_socket.register_b_transport(this, &Bus::b_transport);
}



void Bus::end_of_elaboration()
{
    //size_t slaves = this->initiator_socket.size();
    //size_t masters = this->target_socket.size();
}



void Bus::b_transport(int id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    // Try to arbitrate the bus.
    // If it is not possible, wait until the bus got released and try again.
    while(not this->mutex.try_lock())
        sc_core::wait(this->busrelease);

    sc_dt::uint64 global_addr = trans.get_address();
    int slaveid               = this->AddressToSlaveID(global_addr);
    if(slaveid < 0)
    {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid address " << global_addr << std::endl;

        this->mutex.unlock();
        this->busrelease.notify();
        return;
    }

    this->initiator_socket[slaveid]->b_transport(trans, delay);

    this->mutex.unlock();
    this->busrelease.notify();
}


int Bus::AddressToSlaveID(unsigned int addr)
{
    for(unsigned int i = 0; i < this->starts.size(); i++)
    {
        if( (addr >= this->starts[i]) && (addr <= this->ends[i]))
            return i;
    }

    return -1;
}

} // namespace core

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

