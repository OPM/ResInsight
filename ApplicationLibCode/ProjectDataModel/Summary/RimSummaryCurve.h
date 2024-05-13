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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"

#include "RiaDateTimeDefines.h"
#include "RiaDefines.h"
#include "RiaSummaryCurveAddress.h"
#include "RiaSummaryDefines.h"

#include "RifEclipseSummaryAddressQMetaType.h"
#include "RimStackablePlotCurve.h"

#include "cafAppEnum.h"
#include "cafTristate.h"

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimSummaryCurveAutoName;
class RimSummaryAddress;
class RiaSummaryCurveDefinition;
class RimPlotAxisPropertiesInterface;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCurve : public RimStackablePlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurve();
    ~RimSummaryCurve() override;

    RiaSummaryCurveDefinition curveDefinition() const;
    RiaSummaryCurveAddress    curveAddress() const;

    // Y Axis functions
    RimSummaryCase*             summaryCaseY() const;
    RifEclipseSummaryAddress    summaryAddressY() const;
    std::string                 unitNameY() const;
    virtual std::vector<double> valuesY() const;

    void setSummaryCaseY( RimSummaryCase* sumCase );
    void setSummaryAddressYAndApplyInterpolation( const RifEclipseSummaryAddress& address );
    void setSummaryAddressY( const RifEclipseSummaryAddress& address );
    void setResampling( RiaDefines::DateTimePeriodEnum resampling );

    RifEclipseSummaryAddress    errorSummaryAddressY() const;
    std::vector<double>         errorValuesY() const;
    void                        setLeftOrRightAxisY( RiuPlotAxis plotAxis );
    RiuPlotAxis                 axisY() const;
    virtual std::vector<time_t> timeStepsY() const;
    double                      yValueAtTimeT( time_t time ) const;
    void                        setOverrideCurveDataY( const std::vector<time_t>& xValues, const std::vector<double>& yValues );

    // X Axis functions
    void                           setAxisTypeX( RiaDefines::HorizontalAxisType axisType );
    RiaDefines::HorizontalAxisType axisTypeX() const;
    RimSummaryCase*                summaryCaseX() const;
    RifEclipseSummaryAddress       summaryAddressX() const;
    std::string                    unitNameX() const;
    virtual std::vector<double>    valuesX() const;

    void        setSummaryCaseX( RimSummaryCase* sumCase );
    void        setSummaryAddressX( const RifEclipseSummaryAddress& address );
    void        setTopOrBottomAxisX( RiuPlotAxis plotAxis );
    RiuPlotAxis axisX() const;

    // Other
    bool isEnsembleCurve() const;
    void setIsEnsembleCurve( bool isEnsembleCurve );

    void updatePlotAxis();
    void applyCurveAutoNameSettings( const RimSummaryCurveAutoName& autoNameSettings );

    QString curveExportDescription( const RifEclipseSummaryAddress& address ) const override;
    void    setCurveAppearanceFromCaseType();
    void    setDefaultCurveAppearance();

    void setAsTopZWithinCategory( bool enable );
    void setZIndexFromCurveInfo();

    RiaDefines::PhaseType phaseType() const override;

    virtual bool isRegressionCurve() const;
    void         updateLegendEntryVisibilityNoPlotUpdate() override;

protected:
    // RimPlotCurve overrides
    QString createCurveAutoName() override;
    void    updateZoomInParentPlot() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    void loadAndUpdateDataAndPlot();

    void updateLegendsInPlot() override;

    void   defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void   initAfterRead() override;
    double computeCurveZValue() override;

    virtual std::vector<time_t> timeStepsX() const;

    virtual void updateTimeAnnotations();
    bool         canCurveBeAttached() const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void hideXAxisGroup();

private:
    RifSummaryReaderInterface* valuesSummaryReaderX() const;
    RifSummaryReaderInterface* valuesSummaryReaderY() const;

    void calculateCurveInterpolationFromAddress();

    static void appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options, RimSummaryCase* summaryCase );

private:
    // Y values
    caf::PdmPtrField<RimSummaryCase*>                 m_yValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>            m_yValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>           m_yValuesSummaryAddressUiField;
    caf::PdmField<bool>                               m_yPushButtonSelectSummaryAddress;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_yPlotAxisProperties;
    caf::PdmField<RiaDefines::DateTimePeriodEnum>     m_yValuesResampling;

    // X values
    caf::PdmField<caf::AppEnum<RiaDefines::HorizontalAxisType>> m_xAxisType;
    caf::PdmPtrField<RimSummaryCase*>                           m_xValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>                      m_xValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>                     m_xValuesSummaryAddressUiField;
    caf::PdmField<bool>                                         m_xPushButtonSelectSummaryAddress;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*>           m_xPlotAxisProperties;

    // Other fields
    caf::PdmField<caf::Tristate> m_isEnsembleCurve;

    caf::PdmChildField<RimSummaryCurveAutoName*>      m_curveNameConfig;
    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis_OBSOLETE;
    caf::PdmField<bool>                               m_isTopZWithinCategory;

    bool m_showXAxisGroup = true;
};
