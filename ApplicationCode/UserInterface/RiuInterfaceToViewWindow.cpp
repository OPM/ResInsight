#include "RiuInterfaceToViewWindow.h"

#include <QWidget>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuInterfaceToViewWindow::viewWindowFromWidget(QWidget* possibleViewWindowWidget)
{
    auto interfaceToViewWindow =  dynamic_cast<RiuInterfaceToViewWindow*>(possibleViewWindowWidget);
    if ( interfaceToViewWindow )
    {
        return (interfaceToViewWindow->ownerViewWindow());
    }
    else
    {
        return nullptr;
    }
}
