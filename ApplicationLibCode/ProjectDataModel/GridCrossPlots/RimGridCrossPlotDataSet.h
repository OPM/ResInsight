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

#include "RigEclipseCrossPlotDataExtractor.h"
#include "RigGridCrossPlotCurveGrouping.h"

#include "RimCheckableNamedObject.h"
#include "RimNameConfig.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include <cvfArray.h>

#include <QList>
#include <QPointer>
#include <map>

class RifTextDataTableFormatter;
class RimCase;
class RimGridCrossPlotCurve;
class RimGridView;
class RimEclipseCase;
class RimEclipseResultCase;
class RimEclipseCellColors;
class RimEclipseResultDefinition;
class RimRegularLegendConfig;
class RimPlotCellFilterCollection;
class RimPlotCellFilter;
class RiuDraggableOverlayFrame;
class QwtPlot;
class QwtPlotCurve;
class QString;

class RimGridCrossPlotDataSetNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotDataSetNameConfig();

    caf::PdmField<bool> addCaseName;
    caf::PdmField<bool> addAxisVariables;
    caf::PdmField<bool> addTimestep;
    caf::PdmField<bool> addGrouping;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void doEnableAllAutoNameTags( bool enable ) override;
};

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotDataSet : public RimCheckableNamedObject, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    typedef caf::AppEnum<RigGridCrossPlotCurveGrouping> CurveGroupingEnum;

    enum NameComponents
    {
        GCP_CASE_NAME,
        GCP_AXIS_VARIABLES,
        GCP_TIME_STEP,
        GCP_GROUP_NAME
    };

public:
    RimGridCrossPlotDataSet();
    ~RimGridCrossPlotDataSet() override;

    void    setCellFilterView( RimGridView* cellFilterView );
    void    loadDataAndUpdate( bool updateParentPlot );
    void    setParentQwtPlotNoReplot( QwtPlot* parent );
    QString xAxisName() const;
    QString yAxisName() const;
    QString infoText() const;

    int     indexInPlot() const;
    QString createAutoName() const override;
    QString groupTitle() const;
    QString groupParameter() const;
    void    detachAllCurves();
    void    cellFilterViewUpdated();

    RimRegularLegendConfig* legendConfig() const;

    std::vector<RimGridCrossPlotCurve*> curves() const;

    QString caseNameString() const;
    QString axisVariableString() const;
    QString timeStepString() const;

    std::map<NameComponents, QString> nameComponents() const;

    void updateCurveNames( size_t dataSetIndex, size_t dataSetCount );
    void updateLegendRange();
    void updateLegendIcons();
    bool groupingByCategoryResult() const;
    bool groupingEnabled() const;
    void swapAxisProperties( bool updatePlot );
    void exportFormattedData( RifTextDataTableFormatter& formatter ) const;

    bool isXAxisLogarithmic() const;
    bool isYAxisLogarithmic() const;

    void configureForPressureSaturationCurves( RimEclipseResultCase* eclipseResultCase,
                                               const QString&        dynamicResultName,
                                               int                   timeStep );
    void addCellFilter( RimPlotCellFilter* cellFilter );
    void setCustomColor( const cvf::Color3f color );
    void destroyCurves();

    size_t visibleCurveCount() const;
    size_t sampleCount() const;

protected:
    void initAfterRead() override;
    void onLoadDataAndUpdate( bool updateParentPlot );

    void    assignCurveDataGroups( const RigEclipseCrossPlotResult& result );
    void    createCurves( const RigEclipseCrossPlotResult& result );
    void    fillCurveDataInExistingCurves( const RigEclipseCrossPlotResult& result );
    QString createGroupName( size_t curveIndex ) const;

    std::map<int, cvf::UByteArray> calculateCellVisibility( RimEclipseCase* eclipseCase ) const;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          triggerPlotNameUpdateAndReplot();
    void                          updateDataSetName();
    void                          performAutoNameUpdate() override;
    void                          setDefaults();
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    bool hasMultipleTimeSteps() const;
    void filterInvalidCurveValues( RigEclipseCrossPlotResult* result );

private:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimGridView*>                  m_cellFilterView;
    caf::PdmField<CurveGroupingEnum>                m_grouping;
    caf::PdmChildField<RimEclipseResultDefinition*> m_xAxisProperty;
    caf::PdmChildField<RimEclipseResultDefinition*> m_yAxisProperty;
    caf::PdmChildField<RimEclipseCellColors*>       m_groupingProperty;

    caf::PdmChildField<RimGridCrossPlotDataSetNameConfig*> m_nameConfig;

    caf::PdmChildArrayField<RimGridCrossPlotCurve*> m_crossPlotCurves;

    std::map<int, RigEclipseCrossPlotResult> m_groupedResults;

    caf::PdmField<bool>                              m_useCustomColor;
    caf::PdmField<cvf::Color3f>                      m_customColor;
    caf::PdmChildField<RimPlotCellFilterCollection*> m_plotCellFilterCollection;

    QPointer<RiuDraggableOverlayFrame> m_legendOverlayFrame;
};
