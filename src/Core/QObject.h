#ifndef __Included_QObject__
#define __Included_QObject__

#include <string>
#include <ffi.h>
#include <list>
#include <map>
#include <concepts>
#include <stdexcept>

#include "Project.h"

/* Function signatures */
#define qobj_slot

namespace cocoa
{

struct QSignalInterface;
struct QSlotInterface;
class QObject;

constexpr uint32_t MAX_SIGNAL_PARAMS = 32;

struct QSignalInterface
{
    std::string     name;
    std::list<QSlotInterface*>
                    slots;
    ::ffi_cif       cif;
    ::ffi_type     *artypes[MAX_SIGNAL_PARAMS];
    uint32_t        nargs;
    QObject        *owner;
};

using QSlotMethod = void(*)(void);
struct QSlotInterface
{
    void          *pThis;
    QSignalInterface *pSignal;
    QSlotMethod       pMethod;
};

template<typename T>
concept IsQObjectChild = std::is_base_of<QObject, T>::value;

namespace ffi_type_trains
{

template<typename T>
concept IsPointer = std::is_pointer<T>::value;
template<typename T>
concept IsReference = std::is_reference<T>::value;

template<typename T> inline ffi_type *asFFIType(T&&)       { return nullptr; }
template<> inline ffi_type *asFFIType(int8_t&&)            { return &ffi_type_sint8; }
template<> inline ffi_type *asFFIType(uint8_t&&)           { return &ffi_type_uint8; }
template<> inline ffi_type *asFFIType(int16_t&&)           { return &ffi_type_sint16; }
template<> inline ffi_type *asFFIType(uint16_t&&)          { return &ffi_type_uint16; }
template<> inline ffi_type *asFFIType(int32_t&&)           { return &ffi_type_sint32; }
template<> inline ffi_type *asFFIType(uint32_t&&)          { return &ffi_type_uint32; }
template<> inline ffi_type *asFFIType(int64_t&&)           { return &ffi_type_sint64; }
template<> inline ffi_type *asFFIType(uint64_t&&)          { return &ffi_type_uint64; }
template<> inline ffi_type *asFFIType(bool&&)              { return &ffi_type_uint8; }
template<> inline ffi_type *asFFIType(float&&)             { return &ffi_type_float; }
template<> inline ffi_type *asFFIType(double&&)            { return &ffi_type_double; }
template<> inline ffi_type *asFFIType(long double&&)       { return &ffi_type_longdouble; }
template<IsPointer T> inline ffi_type *asFFIType(T&&)      { return &ffi_type_pointer; }
template<IsReference T> inline ffi_type *asFFIType(T&&)    { return nullptr; }

template<typename T>
inline void expandAsFFIType(::ffi_type **out, T*)
{ *out = nullptr; }

template<>
inline void expandAsFFIType(::ffi_type **out, int8_t*)
{ *out = &ffi_type_sint8; }

template<>
inline void expandAsFFIType(::ffi_type **out, uint8_t*)
{ *out = &ffi_type_uint8; }

template<>
inline void expandAsFFIType(::ffi_type **out, int16_t*)
{ *out = &ffi_type_sint16; }

template<>
inline void expandAsFFIType(::ffi_type **out, uint16_t*)
{ *out = &ffi_type_uint16; }

template<>
inline void expandAsFFIType(::ffi_type **out, int32_t*)
{ *out = &ffi_type_sint32; }

template<>
inline void expandAsFFIType(::ffi_type **out, uint32_t*)
{ *out = &ffi_type_uint32; }

template<>
inline void expandAsFFIType(::ffi_type **out, int64_t*)
{ *out = &ffi_type_sint64; }

template<uint64_t>
inline void expandAsFFIType(::ffi_type **out, uint64_t*)
{ *out = &ffi_type_uint64; }

template<>
inline void expandAsFFIType(::ffi_type **out, float*)
{ *out = &ffi_type_float; }

template<>
inline void expandAsFFIType(::ffi_type **out, double*)
{ *out = &ffi_type_double; }

template<>
inline void expandAsFFIType(::ffi_type **out, long double*)
{ *out = &ffi_type_longdouble; }

template<IsPointer T>
inline void expandAsFFIType(::ffi_type **out, T*)
{ *out = &ffi_type_pointer; }

template<typename...Args>
void expandParamPack(::ffi_type **begin) {
    [[maybe_unused]] int arr[] = { (expandAsFFIType<Args>(begin++, nullptr), 1)... };
}

} // namespace ffi_type_trains

class QObject
{
public:
    virtual ~QObject();

    template<typename... Args>
    static void signal(QObject *owner, const std::string& name)
    {
        ::ffi_type *types[sizeof...(Args) + 1];
        types[0] = &::ffi_type_pointer;
        if (sizeof...(Args))
            ffi_type_trains::expandParamPack<Args...>(&types[1]);
        construct_signal_interface(sizeof...(Args) + 1, types, name, owner);
    }

    template<IsQObjectChild _Tp, typename... Args>
    static void connect(_Tp *pObject, void (_Tp::*fn)(Args...), const std::string& sig)
    {
        void (*normalFn)() = QObject::ptr_force_cast<void(*)(), void(_Tp::*)(Args...)>(fn);
        ::ffi_type *types[sizeof...(Args) + 1];
        types[0] = &::ffi_type_pointer;
        if (sizeof...(Args))
            ffi_type_trains::expandParamPack<Args...>(&types[1]);
        construct_slot_interface(sizeof...(Args) + 1, types, pObject, normalFn, sig);
    }

    template<typename... Args>
    static void emit(const std::string& sig, Args... args)
    {
        constexpr uint32_t nargs = sizeof...(Args) + 1;

        void *pArgs[nargs] = { nullptr, static_cast<void*>(&args)... };
        ::ffi_type *artypes[nargs] = { &::ffi_type_pointer, ffi_type_trains::asFFIType<Args>(std::forward<Args>(args))... };

        call_slots(nargs, pArgs, artypes, sig);
    }
#if defined(COCOA_PROJECT)
    static void dumpToJournal();
#endif

private:
    template<typename Td, typename Ts>
    static inline constexpr Td ptr_force_cast(Ts src) {
        return *static_cast<Td*>(static_cast<void*>(&src));
    }

    static void construct_signal_interface(uint32_t nargs, ::ffi_type **type, const std::string& name, QObject *owner);
    static void construct_slot_interface(uint32_t nargs, ::ffi_type **artypes, void *pThis, void(*fn)(), const std::string& sig);
    static void call_slots(uint32_t nargs, void **pArgs, ::ffi_type **artypes, const std::string& sig);
    static void destruct_this_signals(QObject *self);
    static void remove_slot(QSlotInterface *slot, bool remove_from_signal);

private:
    static std::map<std::string, QSignalInterface*>    fSignalInterfaces;
    static std::list<QSlotInterface*>                  fSlotInterfaces;
};

} // namespace cocoa
#endif /* __Included_QObject__ */
