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

#include "RiaDefines.h"

#include "RimAbstractPlotCollection.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotWindow.h"

#include "RiuMultiPlotBook.h"

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

class RimMultiPlot : public RimPlotWindow, public RimTypedPlotCollection<RimPlot>
{
    CAF_PDM_HEADER_INIT;

public:
    using ColumnCountEnum = caf::AppEnum<RiaDefines::ColumnCount>;
    using RowCountEnum    = caf::AppEnum<RiaDefines::RowCount>;

public:
    RimMultiPlot();
    ~RimMultiPlot() override;

    RimMultiPlot& operator=( RimMultiPlot&& rhs );

    QWidget* viewWidget() override;

    QString description() const override;
    QString projectFileVersionString() const;

    bool    isMultiPlotTitleVisible() const;
    void    setMultiPlotTitleVisible( bool visible );
    QString multiPlotTitle() const;
    void    setMultiPlotTitle( const QString& title );

    void insertPlot( RimPlot* plot, size_t index ) override;
    void removePlot( RimPlot* plot ) override;
    void movePlotsToThis( const std::vector<RimPlot*>& plots, int insertAtPosition );

    virtual void startBatchAddOperation();
    virtual void endBatchAddOperation();

    virtual void removePlotNoUpdate( RimPlot* plot );
    virtual void updateAfterPlotRemove();

    void         deleteAllPlots() override;
    void         updatePlots();
    virtual void updatePlotTitles();

    size_t plotCount() const override;
    size_t plotIndex( const RimPlot* plot ) const;

    std::vector<RimPlot*> plots() const override;
    std::vector<RimPlot*> visiblePlots() const;

    void updatePlotOrderFromGridWidget();

    void setAutoScaleXEnabled( bool enabled );
    void setAutoScaleYEnabled( bool enabled );

    void setColumnCount( RiaDefines::ColumnCount columnCount );
    void setRowCount( RiaDefines::RowCount rowCount );
    void setTickmarkCount( RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum tickmarkCount );

    int columnCount() const override;
    int rowsPerPage() const;

    void setShowPlotTitles( bool enable );
    bool showPlotTitles() const;

    void zoomAll() override;
    void zoomAllYAxes();

    QString asciiDataForPlotExport() const;

    bool previewModeEnabled() const;

    int subTitleFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

    virtual std::vector<caf::PdmFieldHandle*> fieldsToShowInToolbar();
    virtual std::vector<caf::PdmFieldHandle*> fieldsToShowInLayoutToolbar();

    bool isValid() const;

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void uiOrderingForMultiPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void onLoadDataAndUpdate() override;
    void initAfterRead() override;

    void applyPlotWindowTitleToWidgets();
    void updateZoom();
    void recreatePlotWidgets();

    virtual void onPlotAdditionOrRemoval();

    bool isMouseCursorInsidePlot();

private:
    void cleanupBeforeClose();
    void setupBeforeSave() override;
    void doUpdateLayout() override;
    void updateSubPlotNames();
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;
    void onPlotsReordered( const caf::SignalEmitter* emitter );
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    static void setTickmarkCount( RimPlot* plot, RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum tickmarkCount );

protected:
    caf::PdmField<QString>         m_projectFileVersionString;
    caf::PdmField<bool>            m_showPlotWindowTitle;
    caf::PdmField<QString>         m_plotWindowTitle;
    caf::PdmField<ColumnCountEnum> m_columnCount;
    caf::PdmField<RowCountEnum>    m_rowsPerPage;
    caf::PdmField<bool>            m_showIndividualPlotTitles;
    RimFontSizeField               m_subTitleFontSize;
    RimFontSizeField               m_axisTitleFontSize;
    RimFontSizeField               m_axisValueFontSize;
    caf::PdmField<bool>            m_pagePreviewMode;

    caf::PdmField<RimPlotAxisPropertiesInterface::LegendTickmarkCountEnum> m_majorTickmarkCount;

    friend class RiuMultiPlotBook;
    QPointer<RiuMultiPlotBook> m_viewer;

    bool m_delayPlotUpdatesDuringBatchAdd;

private:
    caf::PdmChildArrayField<RimPlot*> m_plots;

    bool m_isValid;
};
