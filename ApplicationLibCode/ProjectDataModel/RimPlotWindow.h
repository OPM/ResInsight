/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "Appearance/RimFontSizeField.h"
#include "RimNameConfig.h"
#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

#include <QPageLayout>

class RimPlot;
class RimProject;
class RiuQwtPlotWidget;

class QwtPlotCurve;
class QKeyEvent;
class QWheelEvent;
class QPaintDevice;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotWindow : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum class LegendPosition
    {
        ABOVE,
        INSIDE_UPPER_RIGHT,
        INSIDE_UPPER_LEFT,
    };

    RimPlotWindow();
    ~RimPlotWindow() override;

    int id() const final;

    RimPlotWindow& operator=( RimPlotWindow&& rhs );

    bool plotTitleVisible() const;
    void setPlotTitleVisible( bool showPlotTitle );

    virtual QString               description() const = 0;
    bool                          legendsVisible() const;
    void                          setLegendsVisible( bool doShow );
    bool                          legendsHorizontal() const;
    void                          setLegendsHorizontal( bool horizontal );
    bool                          legendItemsClickable() const;
    void                          setLegendItemsClickable( bool clickable );
    void                          setLegendPosition( RimPlotWindow::LegendPosition legendPosition );
    RimPlotWindow::LegendPosition legendPosition() const;

    void updateFonts() override;

    int fontSize() const override;
    int titleFontSize() const;
    int legendFontSize() const;

    void setLegendFontSize( caf::FontTools::RelativeSize fontSize );

    void updateLayout();
    void updateParentLayout();

    virtual int columnCount() const;

    void        renderWindowContent( QPaintDevice* painter );
    QPageLayout pageLayout() const;
    int         bottomMargin() const;

    virtual bool handleGlobalKeyEvent( QKeyEvent* keyEvent );
    virtual bool handleGlobalWheelEvent( QWheelEvent* wheelEvent );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void uiOrderingForLegends( QString uiConfigName, caf::PdmUiOrdering& uiOrdering, bool showLegendPosition = false );
    void uiOrderingForFonts( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForLegendsAndFonts( QString uiConfigName, caf::PdmUiOrdering& uiOrdering, bool showLegendPosition = false );

    void updateWindowVisibility();
    void setBottomMargin( int bottomMargin );

private:
    virtual void doUpdateLayout() {}
    virtual bool hasCustomPageLayout( QPageLayout* customPageLayout ) const;
    virtual void doRenderWindowContent( QPaintDevice* paintDevice ) = 0;

private:
    friend class RimProject;
    void setId( int id );

    void assignIdIfNecessary() final;

protected:
    caf::PdmField<int>                          m_id;
    caf::PdmField<bool>                         m_showPlotTitle;
    caf::PdmField<bool>                         m_showPlotLegends;
    caf::PdmField<bool>                         m_plotLegendsHorizontal;
    caf::PdmField<bool>                         m_legendItemsClickable;
    caf::PdmField<caf::AppEnum<LegendPosition>> m_legendPosition;

    RimFontSizeField m_titleFontSize;
    RimFontSizeField m_legendFontSize;

private:
    int m_bottomMargin;
};
