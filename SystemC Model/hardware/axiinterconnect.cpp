#include <iostream>

#include <hardware/interconnect.hpp>
#include <hardware/axiinterconnect.hpp>
#include <hardware/tile.hpp>
#include <hardware/memory.hpp>

AXIInterconnect::AXIInterconnect(const char* name)
    : Interconnect(name)
    , contender(0)
    , readpenalties( { -1,  0,  5,  3, 11, 19, 27, 35}) // -1: there is at least 1 PE accessing the bus
    , writepenalties({ -1,  0,  0,  7,  2,  7, 12, 17}) // so [0] is invalid and therefore [0] = -1
{
}


void AXIInterconnect::PenaltyWait(tlm::tlm_command command)
{
    if(this->contender > 7)
    {
        std::cerr << "\e[1;31mERROR: More than 7 bus participants are trying to communicate. Limit is 7\n";
        return;
    }

    int penalty;

    switch(command)
    {
        case tlm::TLM_WRITE_COMMAND:
            penalty = this->writepenalties[this->contender];
            break;

        case tlm::TLM_READ_COMMAND:
            penalty = this->readpenalties[this->contender];
            break;

        case tlm::TLM_IGNORE_COMMAND:
        default:
            penalty = 0;
            break;
    }

    if(penalty > 0)
        sc_core::wait(penalty, sc_core::SC_NS);

    return;
}



void AXIInterconnect::b_transport(int id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    // Arbitrate the bus. This is a blocking function.
    // This thread remains inside the function until the bus is granted for this thread.
    this->Arbitrate();

    // Perform the transaction on the bus
    sc_dt::uint64 global_addr = trans.get_address();
    int slaveid               = this->AddressToSlaveID(global_addr);
    if(slaveid < 0)
    {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid address " << global_addr << std::endl;
        goto exit;
    }

    this->initiator_socket[slaveid]->b_transport(trans, delay);

    // Beside the minimal transaction time, there is a penalty depending on the bus contention
    this->PenaltyWait(trans.get_command());

exit:
    // Release the bus so that the next thread can access the bus
    this->Release();
    return;
}



void AXIInterconnect::Arbitrate()
{
    this->contender++;

    // Enqueue this thread to get access to the bus
    size_t thisthread = std::hash<std::thread::id>{}(std::this_thread::get_id());
    this->requestqueue.push(thisthread);

    // Wait until the bus gets granted for this thread
    while(this->requestqueue.front() != thisthread)
        sc_core::wait(this->busrelease);
    return;
}



void AXIInterconnect::Release()
{
    this->contender--;

    // This thread is done, let the next thread access the bus
    this->requestqueue.pop();
    this->busrelease.notify();
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

