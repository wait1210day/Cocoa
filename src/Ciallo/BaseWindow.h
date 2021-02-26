#ifndef COCOA_BASEWINDOW_H
#define COCOA_BASEWINDOW_H

#include <string>

#include "Ciallo/GrBase.h"
#include "Ciallo/DDR/GrBasePlatform.h"
#include "Ciallo/GraphicsContext.h"
CIALLO_BEGIN_NS

/**
 * A Window object is the base of Ciallo. Window object processes
 * input and output events from the display server (like X11).
 * It also holds a GraphicsContext.
 */
class BaseWindow
{
public:
    virtual ~BaseWindow() = default;

    inline int32_t width() const {
        return fWidth;
    }

    inline int32_t height() const {
        return fHeight;
    }

    std::string backendName() const;
    bool isClosed();
    void update();
    void createWindow();
    void setWindowTitle(const std::string& name);

    /**
     * @brief Get current GraphicsContext.
     * @return A pointer of GraphicsContext, you shouldn't take its ownership.
     *         Maybe nullptr if there's no created window.
     */
    inline GraphicsContext *GContext()
    {
        return fGContext;
    }

protected:
    explicit BaseWindow(const std::string& backendName, int32_t w, int32_t h);
    void createGContext(std::unique_ptr<GrBasePlatform> platform);
    void destroyGContext();

    virtual bool onIsClosed() = 0;
    virtual void onWindowExpose() = 0;
    virtual void onCreateWindow() = 0;
    virtual void onSetWindowTitle(const std::string& name) = 0;

private:
    std::string              fBackendName;
    int32_t                  fWidth;
    int32_t                  fHeight;
    GraphicsContext         *fGContext;
};

CIALLO_END_NS
#endif //COCOA_BASEWINDOW_H
