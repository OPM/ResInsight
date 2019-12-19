#include "RiuMdiMaximizeWindowGuard.h"

#include "RiaGuiApplication.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiMaximizeWindowGuard::RiuMdiMaximizeWindowGuard()
{
    {
        RiuMainWindow* mainWindow = RiaGuiApplication::instance()->mainWindow();
        if ( mainWindow )
        {
            mainWindow->enableShowFirstVisibleMdiWindowMaximized( false );
        }
    }

    {
        RiuPlotMainWindow* plotMainWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( plotMainWindow )
        {
            plotMainWindow->enableShowFirstVisibleMdiWindowMaximized( false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiMaximizeWindowGuard::~RiuMdiMaximizeWindowGuard()
{
    {
        RiuMainWindow* mainWindow = RiaGuiApplication::instance()->mainWindow();
        if ( mainWindow )
        {
            mainWindow->enableShowFirstVisibleMdiWindowMaximized( true );
        }
    }

    {
        RiuPlotMainWindow* plotMainWindow = RiaGuiApplication::instance()->mainPlotWindow();
        if ( plotMainWindow )
        {
            plotMainWindow->enableShowFirstVisibleMdiWindowMaximized( true );
        }
    }
}
