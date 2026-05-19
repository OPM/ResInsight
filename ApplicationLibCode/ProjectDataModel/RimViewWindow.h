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

#include "cafFontTools.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimDockWindowController;

namespace ads
{
class CDockWidget;
} // namespace ads

class RimViewWindow : public caf::PdmObject, public caf::FontHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimViewWindow();
    ~RimViewWindow() override;

    virtual int id() const = 0;

    bool showWindow() const;
    void setShowWindow( bool showWindow );

    void setAsActiveViewer();
    bool isActive() const;

    bool isMainDockedWindow() const;

    bool isDockedIn3DView() const;
    bool isDockedInPlotView() const;

    void loadDataAndUpdate();
    void updateDockWindowVisibility();

    void removeWindowFromDock();

    void dockAs3DViewWindow();
    void dockAsPlotWindow();

    virtual QWidget* viewWidget() = 0;

    ads::CDockWidget* dockWidget();
    ads::CDockWidget* createDockWidget();

    virtual QImage snapshotWindowContent();
    virtual void   zoomAll() = 0;

    void viewNavigationChanged();

    virtual void updateWindowTitle();

    QString dockWindowName() const;

protected:
    //// Interface for the Window controller
    friend class RimDockWindowController;

    QString          windowTitle();
    virtual QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) = 0;
    virtual void     updateViewWidgetAfterCreation() {};
    virtual void     deleteViewWidget()    = 0;
    virtual void     onLoadDataAndUpdate() = 0;
    virtual void     onViewNavigationChanged();
    virtual bool     isWindowVisible() const; // Virtual To allow special visibility control
    void             deleteDockWidget();
    void             setActive( bool active );
    ////

    // Derived classes are not supposed to override this function. The intention is to always use m_showWindow
    // as the objectToggleField for this class. This way the visibility of a widget being part of a composite widget
    // can be controlled from the project tree using check box toggles
    caf::PdmFieldHandle* objectToggleField() final;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    friend class RimProject;

    void         dockInWindow( int mainWindowID );
    virtual void assignIdIfNecessary() = 0;

protected:
    caf::PdmField<bool>      m_showWindow;
    RimDockWindowController* m_windowController;
    ads::CDockWidget*        m_dockWidget;
    size_t                   m_dockWindowId;
    bool                     m_activeViewer;

    static size_t m_nextDockWindowId;
};
