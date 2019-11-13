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

#include "RimPlotWindow.h"
#include "RiuGridPlotWindow.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

#include <QPointer>
#include <QString>

#include <vector>

class RimPlotInterface;

class RimGridPlotWindow : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum ColumnCount
    {
        COLUMNS_1         = 1,
        COLUMNS_2         = 2,
        COLUMNS_3         = 3,
        COLUMNS_4         = 4,
        COLUMNS_UNLIMITED = 1000,
    };
    typedef caf::AppEnum<ColumnCount> ColumnCountEnum;

public:
    RimGridPlotWindow();
    ~RimGridPlotWindow() override;

    RimGridPlotWindow& operator=( RimGridPlotWindow&& rhs );

    QWidget* viewWidget() override;

    void addPlot( RimPlotInterface* plot );
    void insertPlot( RimPlotInterface* plot, size_t index );
    void removePlot( RimPlotInterface* plot );
    void movePlotsToThis( const std::vector<RimPlotInterface*>& plots, RimPlotInterface* plotToInsertAfter );

    size_t            plotCount() const;
    size_t            plotIndex( const RimPlotInterface* plot ) const;
    RimPlotInterface* plotByIndex( size_t index ) const;

    std::vector<RimPlotInterface*> plots() const;
    std::vector<RimPlotInterface*> visiblePlots() const;

    void         updateLayout() override;
    virtual void updatePlotNames();
    void         updatePlotOrderFromGridWidget();

    virtual void setAutoScaleXEnabled( bool enabled );
    virtual void setAutoScaleYEnabled( bool enabled );

    int                  columnCount() const;
    caf::PdmFieldHandle* columnCountField();
    bool                 showPlotTitles() const;

    void zoomAll() override;

    QString asciiDataForPlotExport() const;

    virtual void onPlotAdditionOrRemoval();

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void uiOrderingForPlotLayout( caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          onLoadDataAndUpdate() override;
    void                          initAfterRead() override;

    void         updatePlotTitle() override;
    void         updatePlots();
    virtual void updateZoom();
    void         recreatePlotWidgets();

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

private:
    void cleanupBeforeClose();

    static RimPlotInterface*       toPlotInterfaceAsserted( caf::PdmObject* pdmObject );
    static const RimPlotInterface* toPlotInterfaceAsserted( const caf::PdmObject* pdmObject );
    static caf::PdmObject*         toPdmObjectAsserted( RimPlotInterface* plotInterface );
    static const caf::PdmObject*   toPdmObjectAsserted( const RimPlotInterface* plotInterface );

protected:
    caf::PdmField<ColumnCountEnum> m_columnCountEnum;
    caf::PdmField<bool>            m_showIndividualPlotTitles;

    friend class RiuGridPlotWindow;
    QPointer<RiuGridPlotWindow> m_viewer;

private:
    caf::PdmChildArrayField<caf::PdmObject*> m_plots;
};
