#ifndef COCOA_SLOT_H
#define COCOA_SLOT_H

#include <tuple>
#include <type_traits>
#include <utility>

#include "core/Project.h"
namespace cocoa {

namespace trains {
/* Implementation of arity */
template<typename... ArgTypes>
struct count : std::integral_constant<std::size_t, sizeof...(ArgTypes)> {};

// Most functors - defer to plain member function case
template<typename T>
struct arity : arity<decltype(&std::remove_reference_t<T>::operator())> {};

// Plain function pointers
template<typename ReturnType, typename... ArgTypes>
struct arity<ReturnType(*)(ArgTypes...)> : count<ArgTypes...> {};

// Member function pointers
template<typename ClassType, typename ReturnType, typename... ArgTypes>
struct arity<ReturnType(ClassType::*)(ArgTypes...)> : count<ArgTypes...> {};

// Const member function pointers
template<typename ClassType, typename ReturnType, typename... ArgTypes>
struct arity<ReturnType(ClassType::*)(ArgTypes...) const> : count<ArgTypes...> {};

} // namespace trains

/* Implementation of slot */
template<typename Ret, typename ...ArgsT>
class Slot
{
public:
    virtual ~Slot() = default;
    virtual Ret operator()(const void *object, ArgsT...) = 0;

protected:
    Slot() = default;
};

/* Implementation for function pointers and lambdas */
template<typename Callable, typename Ret = void, typename ...ArgsT>
class FunctorSlot : public Slot<Ret, ArgsT...>
{
public:
    explicit FunctorSlot(Callable callable)
        : fCallable(callable) {}

    Ret operator()(const void *, ArgsT ...args) override
    {
        return invoke(std::make_index_sequence<trains::arity<Callable>::value>{},
                      std::forward<ArgsT>(args)...);
    }

private:
    template<std::size_t ...Indices>
    Ret invoke(const std::index_sequence<Indices...>&, ArgsT&&... args)
    {
        ((void)args, ...);
        return fCallable(std::get<Indices>(std::make_tuple(args...))...);
    }

    Callable    fCallable;
};

template<typename Class, typename MemberFuncPtr, typename Ret, typename ...ArgsT>
class MemberFunctionSlot : public Slot<Ret, ArgsT...>
{
public:
    explicit MemberFunctionSlot(MemberFuncPtr ptr)
        : fMemberFuncPtr(ptr) {}

    Ret operator()(const void *object, ArgsT ...args) override
    {
        return invoke(std::make_index_sequence<trains::arity<MemberFuncPtr>::value>{},
                      const_cast<Class*>(static_cast<const Class*>(object)),
                      std::forward<ArgsT>(args)...);
    }

private:
    template<std::size_t ...Indices>
    Ret invoke(const std::index_sequence<Indices...>&,
               Class *object,
               ArgsT&& ...args)
    {
        ((void)args, ...);
        return (object->*fMemberFuncPtr)(std::get<Indices>(std::make_tuple(args...))...);
    }

    MemberFuncPtr   fMemberFuncPtr;
};

} // namespace cocoa
#endif //COCOA_SLOT_H
