#include <string>
#include <iostream>

#include "Core/Exception.h"
#include "Ciallo/GrBase.h"
#include "Ciallo/BaseWindow.h"
CIALLO_BEGIN_NS

BaseWindow::BaseWindow(const std::string& backendName, int32_t w, int32_t h)
    : fBackendName(backendName),
      fWidth(w),
      fHeight(h),
      fGContext(nullptr)
{
}

std::string BaseWindow::backendName() const
{
    return fBackendName;
}

void BaseWindow::update()
{
    onWindowExpose();
}

void BaseWindow::createWindow()
{
    onCreateWindow();
}

void BaseWindow::setWindowTitle(const std::string& name)
{
    onSetWindowTitle(name);
}

bool BaseWindow::isClosed()
{
    return onIsClosed();
}

void BaseWindow::createGContext(std::unique_ptr<GrBasePlatform> platform)
{
    if (fGContext != nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("It\'s not allowed to create multiple GraphicsContext")
                .make<RuntimeException>();
    }

    fGContext = new GraphicsContext(std::move(platform));
}

void BaseWindow::destroyGContext()
{
    delete fGContext;
}

CIALLO_END_NS
