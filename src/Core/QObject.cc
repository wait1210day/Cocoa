#include <stdexcept>
#include <iostream>

#include "Core/QObject.h"
#if defined(COCOA_PROJECT)
#include "Core/Journal.h"
#endif

namespace cocoa
{

QObject::~QObject()
{
    /**
     * All of the signals and slots connected to this class should
     * be removed when it's destructed.
    */
    destruct_this_signals(this);
    for (auto itr = fSlotInterfaces.begin(); itr != fSlotInterfaces.end(); itr++)
    {
        QSlotInterface *slot = *itr;
        if (slot->pThis == this)
        {
            slot->pSignal->slots.remove(slot);
            itr = fSlotInterfaces.erase(itr);
            delete slot;
        }
        if (itr == fSlotInterfaces.end())
            break;
    }
}

std::map<std::string, QSignalInterface*> QObject::fSignalInterfaces;
std::list<QSlotInterface*> QObject::fSlotInterfaces;

void QObject::construct_signal_interface(uint32_t nargs, ::ffi_type **type, const std::string& name, QObject *owner)
{
    if (nargs > MAX_SIGNAL_PARAMS)
        throw std::runtime_error("QObject::construct_signal_interface(): Too many paramaters");
    QSignalInterface *interface = new QSignalInterface;
    interface->name = name;
    interface->owner = owner;
    interface->nargs = nargs;
    for (uint32_t i = 0; i < nargs; i++)
        interface->artypes[i] = type[i];
    
    ::ffi_status ret = ::ffi_prep_cif(&interface->cif,
        ::ffi_abi::FFI_DEFAULT_ABI, nargs, &::ffi_type_void, interface->artypes);
    if (ret != ::ffi_status::FFI_OK)
        throw std::runtime_error("QObject::construct_signal_interface(): ABI error (FFI)");
    fSignalInterfaces[name] = interface;
}

void QObject::construct_slot_interface(uint32_t nargs, ::ffi_type **artypes, void *pThis, void(*fn)(), const std::string& sig)
{
    if (!fSignalInterfaces.contains(sig))
        throw std::runtime_error("QObject::construct_slot_interface: Undefined signal name to connect with");
    QSignalInterface *pSignal = fSignalInterfaces[sig];
    if (nargs != pSignal->nargs)
        throw std::runtime_error("QObject::construct_slot_interface: The number of arguments is invalid");
    
    for (uint32_t i = 1; i < nargs; i++)
    {
        if (artypes[i] != pSignal->artypes[i])
        {
            std::ostringstream oss;
            oss << "QObject::construct_slot_interface: Parameter #" << i
                << " has a mismatched type";
            throw std::runtime_error(oss.str());
        }
    }

    QSlotInterface *pSlot = new QSlotInterface;
    pSlot->pMethod = fn;
    pSlot->pThis = pThis;
    pSlot->pSignal = pSignal;
    fSlotInterfaces.push_back(pSlot);
    pSignal->slots.push_back(pSlot);
}

void QObject::call_slots(uint32_t nargs, void **pArgs, ::ffi_type **artypes, const std::string& sig)
{
    if (!fSignalInterfaces.contains(sig))
        throw std::runtime_error("QObject::call_slots: Undefined signal name to connect with");
    QSignalInterface *pSignal = fSignalInterfaces[sig];
    if (nargs != pSignal->nargs)
        throw std::runtime_error("QObject::call_slots: The number of arguments is invalid");
    
    for (uint32_t i = 1; i < nargs; i++)
    {
        if (artypes[i] != pSignal->artypes[i])
        {
            std::ostringstream oss;
            oss << "QObject::call_slots: Argument #" << i
                << " has a mismatched type";
            throw std::runtime_error(oss.str());
        }
    }

    for (QSlotInterface *pSlot : pSignal->slots)
    {
        pArgs[0] = &pSlot->pThis;
        ::ffi_call(&pSignal->cif, pSlot->pMethod, nullptr, pArgs);
    }
}

void QObject::destruct_this_signals(QObject *self)
{
    auto itr = fSignalInterfaces.begin();
    while (itr != fSignalInterfaces.end())
    {
        QSignalInterface *pInterface = (*itr).second;
        if (pInterface->owner == self)
        {
            for (QSlotInterface *slot : pInterface->slots)
                remove_slot(slot, false);
            delete pInterface;
            itr = fSignalInterfaces.erase(itr);
        }
        if (itr == fSignalInterfaces.end())
            break;
        itr++;
    }
}

void QObject::remove_slot(QSlotInterface *slot, bool remove_from_parents)
{
    fSlotInterfaces.remove(slot);
    if (remove_from_parents)
        slot->pSignal->slots.remove(slot);
    delete slot;
}

#if defined(COCOA_PROJECT)
void QObject::dumpToJournal()
{
    log_write(LOG_INFO) << "QObject slots:" << log_endl;
    for (auto sp : fSignalInterfaces)
    {
        QSignalInterface *si = sp.second;
        for (QSlotInterface *slot : si->slots)
        {
            std::string sym;
            gom_utils::resolve_address_symbol(reinterpret_cast<const void*>(slot->pMethod), sym);
            log_write(LOG_INFO) << "    signal=" << si->name << " this=" << slot->pThis << ' ' << sym << log_endl;
        }
    }
}
#endif

} // namespace cocoa
