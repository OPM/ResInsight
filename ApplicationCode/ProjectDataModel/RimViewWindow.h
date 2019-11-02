/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#pragma once

#include "RiaDefines.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <vector>

class RimMdiWindowController;

struct RimMdiWindowGeometry
{
    RimMdiWindowGeometry()
        : mainWindowID( -1 )
        , x( 0 )
        , y( 0 )
        , width( -1 )
        , height( -1 )
        , isMaximized( false )
    {
    }
    bool isValid() const
    {
        return ( mainWindowID >= 0 && width >= 0 && height >= 0 );
    }

    int mainWindowID;

    int  x;
    int  y;
    int  width;
    int  height;
    bool isMaximized;
};

class RimViewWindow : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimViewWindow( void );
    ~RimViewWindow( void ) override;

    int  id() const;
    void setId( int id );

    void loadDataAndUpdate();
    void handleMdiWindowClosed();
    void updateMdiWindowVisibility();

    void setAs3DViewMdiWindow()
    {
        setAsMdiWindow( 0 );
    }
    void setAsPlotMdiWindow()
    {
        setAsMdiWindow( 1 );
    }
    void revokeMdiWindowStatus();

    bool isMdiWindow() const;

    void                 setMdiWindowGeometry( const RimMdiWindowGeometry& windowGeometry );
    RimMdiWindowGeometry mdiWindowGeometry();

    virtual QWidget* viewWidget() = 0;

    virtual QImage snapshotWindowContent() = 0;
    virtual void   zoomAll()               = 0;

    void viewNavigationChanged();

    virtual bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const
    {
        return false;
    }
    virtual bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                                int                         oldFontSize,
                                int                         fontSize,
                                bool                        forceChange = false )
    {
        return false;
    }

protected:
    void removeMdiWindowFromMdiArea();

    ///////// Interface for the Window controller
    friend class RimMdiWindowController;

    QString          windowTitle();
    virtual QWidget* createViewWidget( QWidget* mainWindowParent ) = 0;
    virtual void     updateViewWidgetAfterCreation(){};
    virtual void     updateMdiWindowTitle(); // Has real default implementation
    virtual void     deleteViewWidget()    = 0;
    virtual void     onLoadDataAndUpdate() = 0;
    virtual void     onViewNavigationChanged();
    virtual bool     isWindowVisible() const; // Virtual To allow special visibility control
    //////////

    // Derived classes are not supposed to override this function. The intention is to always use m_showWindow
    // as the objectToggleField for this class. This way the visibility of a widget being part of a composite widget
    // can be controlled from the project tree using check box toggles
    caf::PdmFieldHandle* objectToggleField() final;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue ) override;
    void                 initAfterRead() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void setAsMdiWindow( int mainWindowID );

protected:
    caf::PdmField<bool> m_showWindow;

private:
    caf::PdmChildField<RimMdiWindowController*> m_windowController;
    caf::PdmField<int>                          m_viewId;

    // Obsoleted field
    caf::PdmField<std::vector<int>> obsoleteField_windowGeometry;
};
