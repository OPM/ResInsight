/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////
#include "RiuMdiArea.h"

#include "RimProject.h"
#include "RiuMainWindow.h"
#include "RiuMdiSubWindow.h"
#include "RiuPlotMainWindow.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiArea::RiuMdiArea( QWidget* parent /*= nullptr*/ )
    : QMdiArea( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMdiArea::~RiuMdiArea()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WindowTileMode RiuMdiArea::tileMode() const
{
    auto* mainWindow = dynamic_cast<RiuMainWindow*>( window() );
    if ( mainWindow ) return RimProject::current()->subWindowsTileMode3DWindow();

    auto* plotMainWindow = dynamic_cast<RiuPlotMainWindow*>( window() );
    if ( plotMainWindow ) return RimProject::current()->subWindowsTileModePlotWindow();

    return RiaDefines::WindowTileMode::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<QMdiSubWindow*> RiuMdiArea::subWindowListSortedByPosition()
{
    // Tile Windows so the one with the leftmost left edge gets sorted first.
    std::list<QMdiSubWindow*> windowList;
    for ( QMdiSubWindow* subWindow : subWindowList( QMdiArea::CreationOrder ) )
    {
        windowList.push_back( subWindow );
    }

    // Sort of list so we first sort by window position but retain activation order
    // for windows with the same position
    windowList.sort( [this]( QMdiSubWindow* lhs, QMdiSubWindow* rhs ) {
        if ( lhs->frameGeometry().topLeft().rx() == rhs->frameGeometry().topLeft().rx() )
        {
            return lhs->frameGeometry().topLeft().ry() < rhs->frameGeometry().topLeft().ry();
        }
        return lhs->frameGeometry().topLeft().rx() < rhs->frameGeometry().topLeft().rx();
    } );
    return windowList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::tileWindowsHorizontally()
{
    QPoint position( 0, 0 );

    for ( auto* window : subWindowList() )
    {
        QRect rect( 0, 0, width() / subWindowList().count(), height() );

        window->setGeometry( rect );
        window->move( position );
        position.setX( position.x() + window->width() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::tileWindowsVertically()
{
    QPoint position( 0, 0 );
    for ( auto* window : subWindowList() )
    {
        QRect rect( 0, 0, width(), height() / subWindowList().count() );

        window->setGeometry( rect );
        window->move( position );
        position.setY( position.y() + window->height() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::tileWindowsDefault()
{
    for ( auto subWindow : subWindowList() )
    {
        auto riuWindow = dynamic_cast<RiuMdiSubWindow*>( subWindow );
        riuWindow->blockTilingChanges( true );
    }

    RiuMainWindowBase* mainWindow = dynamic_cast<RiuMainWindowBase*>( window() );
    mainWindow->setBlockSubWindowActivatedSignal( true );

    // Workaround for Qt bug #51761: https://bugreports.qt.io/browse/QTBUG-51761
    // Set the first window to be the active window then perform resize event and set back.
    auto a = activeSubWindow();
    setActiveSubWindow( subWindowListSortedByPosition().front() );

    // QMdiArea::resizeEvent( resizeEvent );
    tileSubWindows();

    setActiveSubWindow( a );

    mainWindow->setBlockSubWindowActivatedSignal( false );

    for ( auto subWindow : subWindowList() )
    {
        auto riuWindow = dynamic_cast<RiuMdiSubWindow*>( subWindow );
        riuWindow->blockTilingChanges( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::resizeEvent( QResizeEvent* resizeEvent )
{
    if ( tileMode() != RiaDefines::WindowTileMode::UNDEFINED )
    {
        updateTiling();
    }
    else
    {
        QMdiArea::resizeEvent( resizeEvent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::moveEvent( QMoveEvent* event )
{
    for ( auto subWindow : subWindowList() )
    {
        auto riuWindow = dynamic_cast<RiuMdiSubWindow*>( subWindow );
        riuWindow->blockTilingChanges( true );
    }

    QMdiArea::moveEvent( event );

    for ( auto subWindow : subWindowList() )
    {
        auto riuWindow = dynamic_cast<RiuMdiSubWindow*>( subWindow );
        riuWindow->blockTilingChanges( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMdiArea::updateTiling()
{
    switch ( tileMode() )
    {
        case RiaDefines::WindowTileMode::UNDEFINED:
            break;
        case RiaDefines::WindowTileMode::DEFAULT:
            tileWindowsDefault();
            break;
        case RiaDefines::WindowTileMode::VERTICAL:
            tileWindowsVertically();
            break;
        case RiaDefines::WindowTileMode::HORIZONTAL:
            tileWindowsHorizontally();
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMdiArea::subWindowsAreTiled() const
{
    RiuMainWindow* mainWindow = dynamic_cast<RiuMainWindow*>( window() );

    if ( mainWindow )
    {
        return mainWindow->subWindowsAreTiled() && subWindowList().size() > 0;
    }
    else
    {
        RiuPlotMainWindow* plotWindow = dynamic_cast<RiuPlotMainWindow*>( window() );
        if ( plotWindow )
        {
            return plotWindow->subWindowsAreTiled() && subWindowList().size() > 0;
        }
    }

    return false;
}
