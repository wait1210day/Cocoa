#ifndef COCOA_KEVENT_H
#define COCOA_KEVENT_H

#include <stdexcept>

#include "include/core/SkRect.h"

#include "Komorebi/Komorebi.h"
KMR_NAMESPACE_BEGIN

class KEvent
{
public:
    enum class Kind
    {
        kRepaintEvent
    };

    explicit KEvent(Kind kind);
    virtual ~KEvent() = default;

    Kind kind();

    template<typename T>
    T *cast()
    {
        if (this->kind() != T::EVENT_KIND)
            throw std::runtime_error("KEvent::cast: Bad cast, event type mismatched");
        T *ptr = dynamic_cast<T*>(this);
        if (ptr != nullptr)
            throw std::runtime_error("KEvent::cast: dyanmic_cast<T>(this) returns nullptr");
        return ptr;
    }

private:
    Kind    fKind;
};

class KRepaintEvent : public KEvent
{
public:
    constexpr static KEvent::Kind EVENT_KIND = KEvent::Kind::kRepaintEvent;

    explicit KRepaintEvent(const SkRect& clip);
    ~KRepaintEvent() override = default;

    SkRect clip() const;
    void setClip(const SkRect& clip);

private:
    SkRect      fClip;
};

KMR_NAMESPACE_END
#endif //COCOA_KEVENT_H
