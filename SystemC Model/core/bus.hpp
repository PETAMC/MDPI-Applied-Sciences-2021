#ifndef CORE_BUS_HPP
#define CORE_BUS_HPP

#include <vector>
#include <mutex>

#define SC_INCLUDE_DYNAMIC_PROCESS

#include <systemc>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm.h>

namespace core
{

class Bus
    : public sc_core::sc_module
{
    public:
        tlm_utils::multi_passthrough_initiator_socket<Bus> initiator_socket;
        tlm_utils::multi_passthrough_target_socket<Bus>    target_socket;
        Bus(sc_core::sc_module_name name);
        ~Bus(){};

    protected:
        std::vector<sc_dt::uint64> starts;
        std::vector<sc_dt::uint64> ends;

        // Address mapping
        int AddressToSlaveID(unsigned int addr);

    private:
        virtual void b_transport(int id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
        virtual void end_of_elaboration();

        std::mutex mutex;
        sc_core::sc_event busrelease;
};

}   // namespace core

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

