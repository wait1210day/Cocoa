#ifndef COCOA_XCBWINDOW_H
#define COCOA_XCBWINDOW_H

#include <thread>

#include <xcb/xcb.h>

#include "Ciallo/GrBase.h"
#include "Ciallo/BaseWindow.h"

CIALLO_BEGIN_NS

class XCBWindow : public BaseWindow
{
public:
    explicit XCBWindow(const char *pDisplayName, int32_t width, int32_t height);
    ~XCBWindow() override;

private:
    enum class EventResponse
    {
        kNormal,
        kThreadExit
    };

    void connectionThread();
    EventResponse threadHandleEvent(xcb_generic_event_t *ev);
    EventResponse threadHandleExpose(xcb_expose_event_t *ev);
    EventResponse threadHandleClientMessage(xcb_client_message_event_t *ev);

    bool onIsClosed() override;
    void onWindowExpose() override;
    void onCreateWindow() override;
    void onSetWindowTitle(const std::string& name) override;

private:
    bool                     fIsClosed;
    xcb_connection_t        *fConnection;
    const xcb_setup_t       *fSetup;
    xcb_screen_t            *fScreen;
    int                      fScreenNum;
    std::thread              fConnectionThread;
    xcb_window_t             fWindow;

    xcb_atom_t               fWmDeleteWindowAtom;
    xcb_atom_t               fWmProtocolAtom;
};

CIALLO_END_NS
#endif //COCOA_XCBWINDOW_H
