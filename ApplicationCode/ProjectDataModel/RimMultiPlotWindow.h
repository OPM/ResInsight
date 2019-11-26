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
#include "RiuMultiPlotWindow.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

#include <QPointer>
#include <QString>

#include <vector>

class RimPlot;

class RimMultiPlotWindow : public RimPlotWindow
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
    using ColumnCountEnum = caf::AppEnum<ColumnCount>;

public:
    RimMultiPlotWindow();
    ~RimMultiPlotWindow() override;

    RimMultiPlotWindow& operator=( RimMultiPlotWindow&& rhs );

    QWidget* viewWidget() override;

    QString description() const override;

    bool    isMultiPlotTitleVisible() const;
    void    setMultiPlotTitleVisible( bool visible );
    QString multiPlotTitle() const;
    void    setMultiPlotTitle( const QString& title );

    void addPlot( RimPlot* plot );
    void insertPlot( RimPlot* plot, size_t index );
    void removePlot( RimPlot* plot );
    void movePlotsToThis( const std::vector<RimPlot*>& plots, RimPlot* plotToInsertAfter );

    size_t   plotCount() const;
    size_t   plotIndex( const RimPlot* plot ) const;
    RimPlot* plotByIndex( size_t index ) const;

    std::vector<RimPlot*> plots() const;
    std::vector<RimPlot*> visiblePlots() const;

    void updatePlotOrderFromGridWidget();

    void setAutoScaleXEnabled( bool enabled );
    void setAutoScaleYEnabled( bool enabled );

    int                  columnCount() const;
    caf::PdmFieldHandle* columnCountField();
    bool                 showPlotTitles() const;

    void zoomAll() override;

    QString      asciiDataForPlotExport() const;
    virtual void onPlotAdditionOrRemoval();
    void         setAcceptDrops( bool acceptDrops );
    bool         acceptDrops() const;

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void uiOrderingForPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          onLoadDataAndUpdate() override;
    void                          initAfterRead() override;

    void         applyPlotWindowTitleToWidgets();
    void         updatePlots();
    virtual void updateZoom();
    void         recreatePlotWidgets();

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

private:
    void         cleanupBeforeClose();
    void         doUpdateLayout() override;
    virtual void updateSubPlotNames();
    virtual void updatePlotWindowTitle();

protected:
    caf::PdmField<bool>            m_showPlotWindowTitle;
    caf::PdmField<QString>         m_plotWindowTitle;
    caf::PdmField<ColumnCountEnum> m_columnCountEnum;
    caf::PdmField<bool>            m_showIndividualPlotTitles;

    friend class RiuMultiPlotWindow;
    QPointer<RiuMultiPlotWindow> m_viewer;

private:
    caf::PdmChildArrayField<RimPlot*> m_plots;

    bool m_acceptDrops;
};
