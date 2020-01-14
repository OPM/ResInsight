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
    using ColumnCount     = RiuMultiPlotWindow::ColumnCount;
    using ColumnCountEnum = caf::AppEnum<ColumnCount>;

    enum RowCount
    {
        ROWS_1 = 1,
        ROWS_2 = 2,
        ROWS_3 = 3,
        ROWS_4 = 4,
    };
    using RowCountEnum = caf::AppEnum<RowCount>;

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
    void movePlotsToThis( const std::vector<const RiuQwtPlotWidget*>& plotWidgetsToMove,
                          const RiuQwtPlotWidget*                     plotWidgetToInsertAfter );

    size_t   plotCount() const;
    size_t   plotIndex( const RimPlot* plot ) const;
    RimPlot* plotByIndex( size_t index ) const;

    std::vector<RimPlot*> plots() const;
    std::vector<RimPlot*> visiblePlots() const;

    void updatePlotOrderFromGridWidget();

    void setAutoScaleXEnabled( bool enabled );
    void setAutoScaleYEnabled( bool enabled );

    int                  columnCount() const override;
    int                  rowsPerPage() const;
    caf::PdmFieldHandle* columnCountField();
    caf::PdmFieldHandle* rowsPerPageField();
    bool                 showPlotTitles() const;

    void zoomAll() override;

    QString asciiDataForPlotExport() const;
    void    setAcceptDrops( bool acceptDrops );
    bool    acceptDrops() const override;

    bool previewModeEnabled() const;

    RimPlot* plotFromWidget( const RiuQwtPlotWidget* plotWidget ) override;

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void uiOrderingForMultiPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          onLoadDataAndUpdate() override;
    void                          initAfterRead() override;

    void applyPlotWindowTitleToWidgets();
    void updatePlots();
    void updateZoom();
    void recreatePlotWidgets();

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

private:
    void cleanupBeforeClose();
    void doUpdateLayout() override;
    void updateSubPlotNames();
    void updatePlotWindowTitle();
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;
    void onPlotAdditionOrRemoval();

protected:
    caf::PdmField<bool>            m_showPlotWindowTitle;
    caf::PdmField<QString>         m_plotWindowTitle;
    caf::PdmField<ColumnCountEnum> m_columnCount;
    caf::PdmField<RowCountEnum>    m_rowsPerPage;
    caf::PdmField<bool>            m_showIndividualPlotTitles;

    friend class RiuMultiPlotWindow;
    QPointer<RiuMultiPlotWindow> m_viewer;

private:
    caf::PdmChildArrayField<RimPlot*> m_plots;

    bool m_acceptDrops;
};
