#ifndef SLAVE_HPP
#define SLAVE_HPP

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>
#include <tlm.h>
#include <vector>

namespace core
{

class Slave
    : public sc_core::sc_module
    , protected tlm::tlm_fw_transport_if<>
{
    public:
        tlm::tlm_target_socket<> target_socket;
        Slave(sc_core::sc_module_name name);
        virtual ~Slave(){};

        virtual unsigned long Read(uint64_t address) const = 0;
        virtual void Write(uint64_t address, unsigned long word) = 0;

    private:
        int CheckIndex(int index);
        virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

        virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
        virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&);
        virtual unsigned int transport_dbg(tlm::tlm_generic_payload&);
};

} // namespace core
#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

