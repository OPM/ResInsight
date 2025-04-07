/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuMainWindowTools.h"

#include "RiaGuiApplication.h"

#include "RimViewWindow.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimProject.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowBase.h"
#include "RiuPlotMainWindow.h"
#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeView.h"

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QModelIndex>

// Dark title bar is taken from
// https://envyen.com/posts/2021-10-24-QT-Windows-Dark-theme/

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#pragma comment( lib, "Dwmapi.lib" )

enum PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED                     = 0,
    WCA_NCRENDERING_ENABLED           = 1,
    WCA_NCRENDERING_POLICY            = 2,
    WCA_TRANSITIONS_FORCEDISABLED     = 3,
    WCA_ALLOW_NCPAINT                 = 4,
    WCA_CAPTION_BUTTON_BOUNDS         = 5,
    WCA_NONCLIENT_RTL_LAYOUT          = 6,
    WCA_FORCE_ICONIC_REPRESENTATION   = 7,
    WCA_EXTENDED_FRAME_BOUNDS         = 8,
    WCA_HAS_ICONIC_BITMAP             = 9,
    WCA_THEME_ATTRIBUTES              = 10,
    WCA_NCRENDERING_EXILED            = 11,
    WCA_NCADORNMENTINFO               = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW     = 13,
    WCA_VIDEO_OVERLAY_ACTIVE          = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK                 = 16,
    WCA_CLOAK                         = 17,
    WCA_CLOAKED                       = 18,
    WCA_ACCENT_POLICY                 = 19,
    WCA_FREEZE_REPRESENTATION         = 20,
    WCA_EVER_UNCLOAKED                = 21,
    WCA_VISUAL_OWNER                  = 22,
    WCA_HOLOGRAPHIC                   = 23,
    WCA_EXCLUDED_FROM_DDA             = 24,
    WCA_PASSIVEUPDATEMODE             = 25,
    WCA_USEDARKMODECOLORS             = 26,
    WCA_LAST                          = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID                   pvData;
    SIZE_T                  cbData;
};

using fnAllowDarkModeForWindow        = BOOL( WINAPI* )( HWND hWnd, BOOL allow );
using fnSetPreferredAppMode           = PreferredAppMode( WINAPI* )( PreferredAppMode appMode );
using fnSetWindowCompositionAttribute = BOOL( WINAPI* )( HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA* );

static void setDarkTitlebar( HWND hwnd )
{
    HMODULE hUxtheme = LoadLibraryExW( L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 );
    if ( !hUxtheme ) return;

    HMODULE hUser32 = GetModuleHandleW( L"user32.dll" );
    if ( !hUser32 ) return;

    fnAllowDarkModeForWindow AllowDarkModeForWindow =
        reinterpret_cast<fnAllowDarkModeForWindow>( GetProcAddress( hUxtheme, MAKEINTRESOURCEA( 133 ) ) );
    fnSetPreferredAppMode SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>( GetProcAddress( hUxtheme, MAKEINTRESOURCEA( 135 ) ) );
    fnSetWindowCompositionAttribute SetWindowCompositionAttribute =
        reinterpret_cast<fnSetWindowCompositionAttribute>( GetProcAddress( hUser32, "SetWindowCompositionAttribute" ) );

    SetPreferredAppMode( AllowDark );
    BOOL dark = TRUE;
    AllowDarkModeForWindow( hwnd, dark );
    WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof( dark ) };
    SetWindowCompositionAttribute( hwnd, &data );
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::collapseSiblings( const caf::PdmUiItem* sourceUiItem )
{
    if ( !sourceUiItem ) return;

    if ( !RiaGuiApplication::isRunning() ) return;

    caf::PdmUiTreeView* sourceTreeView = nullptr;

    if ( RiuMainWindow::instance() )
    {
        sourceTreeView = RiuMainWindow::instance()->getTreeViewWithItem( sourceUiItem );
    }
    if ( !sourceTreeView )
    {
        RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
        if ( mpw )
        {
            sourceTreeView = mpw->getTreeViewWithItem( sourceUiItem );
        }
    }

    if ( !sourceTreeView ) return;

    caf::PdmUiTreeOrdering* sourceTreeOrderingItem = nullptr;
    QModelIndex             modIndex               = sourceTreeView->findModelIndex( sourceUiItem );
    if ( !modIndex.isValid() ) return;

    sourceTreeOrderingItem = sourceTreeView->uiTreeOrderingFromModelIndex( modIndex );
    if ( sourceTreeOrderingItem == nullptr ) return;

    if ( sourceTreeOrderingItem && sourceTreeOrderingItem->parent() )
    {
        for ( int i = 0; i < sourceTreeOrderingItem->parent()->childCount(); i++ )
        {
            auto siblingTreeOrderingItem = sourceTreeOrderingItem->parent()->child( i );
            if ( siblingTreeOrderingItem != sourceTreeOrderingItem )
            {
                sourceTreeView->setExpanded( siblingTreeOrderingItem->activeItem(), false );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::setWindowSizeOnWidgetsInMdiWindows( RiuMainWindowBase* mainWindow, int width, int height )
{
    if ( !mainWindow ) return;

    auto widgets = mainWindow->findChildren<QMdiSubWindow*>();
    for ( auto w : widgets )
    {
        if ( !w ) continue;

        w->showNormal();
    }

    // Process events before resize to make sure the widget is ready for resize
    // If not, a maximized window with not get the prescribed window size
    QApplication::processEvents();

    for ( auto w : widgets )
    {
        if ( !w ) continue;
        auto viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( w->widget() );

        if ( viewWindow && viewWindow->viewWidget() )
        {
            QWidget* viewWidget = viewWindow->viewWidget();

            viewWidget->resize( width, height );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::setFixedWindowSizeFor3dViews( RiuMainWindowBase* mainWindow, int width, int height )
{
    if ( !mainWindow ) return;

    RimProject* proj = RimProject::current();
    if ( !proj ) return;

    for ( Rim3dView* riv : proj->allViews() )
    {
        if ( riv && riv->viewer() )
        {
            // Make sure all views are maximized for snapshotting
            QMdiSubWindow* subWnd = mainWindow->findMdiSubWindow( riv->viewer()->layoutWidget() );
            if ( subWnd )
            {
                subWnd->showMaximized();
            }

            // This size is set to match the regression test reference images
            QSize windowSize( width, height );

            riv->viewer()->setFixedSize( windowSize );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::setDarkTitleBarWindows( QWidget* widget )
{
    if ( !widget ) return;

#ifdef Q_OS_WIN
    setDarkTitlebar( reinterpret_cast<HWND>( widget->winId() ) );
#endif
}
