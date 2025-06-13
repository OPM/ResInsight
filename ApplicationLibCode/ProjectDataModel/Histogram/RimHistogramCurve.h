/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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
#include "cafPdmPtrField.h"

#include "RimStackablePlotCurve.h"

class RimPlotAxisPropertiesInterface;
class RimHistogramDataSource;

//==================================================================================================
///
///
//==================================================================================================
class RimHistogramCurve : public RimStackablePlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimHistogramCurve();
    ~RimHistogramCurve() override;

    // Y Axis functions
    // RimHistogramCase*             histogramCaseY() const;
    // RifEclipseHistogramAddress    histogramAddressY() const;
    std::string                 unitNameY() const;
    virtual std::vector<double> valuesY() const;

    // RifEclipseHistogramAddress    errorHistogramAddressY() const;
    // std::vector<double>         errorValuesY() const;
    void        setLeftOrRightAxisY( RiuPlotAxis plotAxis );
    RiuPlotAxis axisY() const;
    // virtual std::vector<time_t> timeStepsY() const;
    // double                      yValueAtTimeT( time_t time ) const;
    // void                        setOverrideCurveDataY( const std::vector<time_t>& xValues, const std::vector<double>& yValues );

    // RifEclipseHistogramAddressDefines::CurveType curveType() const;

    // // X Axis functions
    //  void setAxisTypeX( RiaDefines::HorizontalAxisType axisType );
    // RiaDefines::HorizontalAxisType axisTypeX() const;
    // RimHistogramCase*                histogramCaseX() const;
    // RifEclipseHistogramAddress       histogramAddressX() const;
    std::string                 unitNameX() const;
    virtual std::vector<double> valuesX() const;

    // void        setHistogramCaseX( RimHistogramCase* sumCase );
    // void        setHistogramAddressX( const RifEclipseHistogramAddress& address );
    // void        setTopOrBottomAxisX( RiuPlotAxis plotAxis );
    RiuPlotAxis axisX() const;

    // // Other
    // bool isEnsembleCurve() const;

    void updatePlotAxis();
    // void enableVectorNameInCurveName( bool enable );

    // QString curveExportDescription( const RifEclipseHistogramAddress& address ) const override;
    // void    setCurveAppearanceFromCaseType();
    // void    setDefaultCurveAppearance();

    // void setAsTopZWithinCategory( bool enable );
    // void setZIndexFromCurveInfo();

    // RiaDefines::PhaseType phaseType() const override;

    // virtual bool isRegressionCurve() const;
    void updateLegendEntryVisibilityNoPlotUpdate() override;

    void setDataSource( RimHistogramDataSource* dataSource );

protected:
    // RimPlotCurve overrides
    QString createCurveAutoName() override;
    void    updateZoomInParentPlot() override;
    void    onLoadDataAndUpdate( bool updateParentPlot ) override;

    void loadAndUpdateDataAndPlot();

    void updateLegendsInPlot() override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    // void   initAfterRead() override;
    double computeCurveZValue() override;

    // virtual std::vector<time_t> timeStepsX() const;

    // virtual void updateTimeAnnotations();
    bool canCurveBeAttached() const override;

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void hideXAxisGroup();

    void onDataSourceChanged( const caf::SignalEmitter* emitter );

private:
    // void calculateCurveInterpolationFromAddress();
    // void calculateCurveTypeFromAddress();

    // static void appendOptionItemsForHistogramAddresses( QList<caf::PdmOptionItemInfo>* options, RimHistogramCase* histogramCase );

private:
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_yPlotAxisProperties;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_xPlotAxisProperties;

    caf::PdmChildField<RimHistogramDataSource*> m_dataSource;
};
