/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RiaCurveDataTools.h"
#include "RiaDefines.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtSymbol.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"

#include <QPointer>
#include <Qt>

class QwtPlot;
class QwtPlotCurve;
class QwtPlotIntervalCurve;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotCurve : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<>        appearanceChanged;
    caf::Signal<bool>    visibilityChanged;
    caf::Signal<>        dataChanged;
    caf::Signal<QString> nameChanged;
    caf::Signal<bool>    stackingChanged;
    caf::Signal<bool>    stackingColorsChanged;

public:
    typedef caf::AppEnum<RiuQwtPlotCurve::CurveInterpolationEnum> CurveInterpolation;
    typedef caf::AppEnum<RiuQwtPlotCurve::LineStyleEnum>          LineStyle;
    typedef caf::AppEnum<RiuQwtSymbol::PointSymbolEnum>           PointSymbol;
    typedef caf::AppEnum<RiuQwtSymbol::LabelPosition>             LabelPosition;
    typedef caf::AppEnum<Qt::BrushStyle>                          FillStyle;

public:
    RimPlotCurve();
    ~RimPlotCurve() override;

    void loadDataAndUpdate( bool updateParentPlot );

    virtual bool xValueRangeInQwt( double* minimumValue, double* maximumValue ) const;
    virtual bool yValueRangeInQwt( double* minimumValue, double* maximumValue ) const;

    void          setParentQwtPlotAndReplot( QwtPlot* plot );
    void          setParentQwtPlotNoReplot( QwtPlot* plot );
    void          detachQwtCurve();
    void          reattachQwtCurve();
    QwtPlotCurve* qwtPlotCurve() const;

    void                          setColor( const cvf::Color3f& color );
    cvf::Color3f                  color() const { return m_curveColor; }
    void                          setLineStyle( RiuQwtPlotCurve::LineStyleEnum lineStyle );
    void                          setSymbol( RiuQwtSymbol::PointSymbolEnum symbolStyle );
    void                          setInterpolation( RiuQwtPlotCurve::CurveInterpolationEnum );
    RiuQwtSymbol::PointSymbolEnum symbol();
    int                           symbolSize() const;
    cvf::Color3f                  symbolEdgeColor() const;
    void                          setSymbolEdgeColor( const cvf::Color3f& edgeColor );
    void                          setSymbolSkipDistance( float distance );
    void                          setSymbolLabel( const QString& label );
    void                          setSymbolLabelPosition( RiuQwtSymbol::LabelPosition labelPosition );
    void                          setSymbolSize( int sizeInPixels );
    void                          setLineThickness( int thickness );
    void                          resetAppearance();
    Qt::BrushStyle                fillStyle() const;
    void                          setFillStyle( Qt::BrushStyle brushStyle );
    void                          setFillColor( const cvf::Color3f& fillColor );

    bool isCurveVisible() const;
    void setCurveVisibility( bool visible );

    void updateCurveName();
    void updateCurveNameAndUpdatePlotLegendAndTitle();
    void updateCurveNameNoLegendUpdate();

    QString         curveName() const { return m_curveName; }
    virtual QString curveExportDescription( const RifEclipseSummaryAddress& address = RifEclipseSummaryAddress() ) const
    {
        return m_curveName;
    }
    void    setCustomName( const QString& customName );
    QString legendEntryText() const;
    void    setLegendEntryText( const QString& legendEntryText );

    void updateCurveVisibility();
    void updateLegendEntryVisibilityAndPlotLegend();
    void updateLegendEntryVisibilityNoPlotUpdate();

    void showLegend( bool show );

    void setZOrder( double z );

    virtual void updateCurveAppearance();
    bool         isCrossPlotCurve() const;
    void         updateUiIconFromPlotSymbol();

    virtual RiaDefines::PhaseType phaseType() const;
    void                          assignStackColor( size_t index, size_t count );
    bool                          stacked() const;
    bool                          stackWithPhaseColors() const;

protected:
    virtual QString createCurveAutoName()                        = 0;
    virtual void    updateZoomInParentPlot()                     = 0;
    virtual void    onLoadDataAndUpdate( bool updateParentPlot ) = 0;
    void            initAfterRead() override;
    void            updateCurvePresentation( bool updatePlotLegendAndTitle );

    void         updateOptionSensitivity();
    void         updatePlotTitle();
    virtual void updateLegendsInPlot();

    void setSamplesFromXYErrorValues( const std::vector<double>& xValues,
                                      const std::vector<double>& yValues,
                                      const std::vector<double>& errorValues,
                                      bool                       keepOnlyPositiveValues,
                                      RiaCurveDataTools::ErrorAxis errorAxis = RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS );
    void setSamplesFromXYValues( const std::vector<double>& xValues,
                                 const std::vector<double>& yValues,
                                 bool                       keepOnlyPositiveValues );
    void setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                        const std::vector<double>&    yValues,
                                        bool                          keepOnlyPositiveValues );

    void setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                        const std::vector<double>& yValues,
                                        bool                       keepOnlyPositiveValues );

protected:
    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle*          objectToggleField() override;
    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          appearanceUiOrdering( caf::PdmUiOrdering& uiOrdering );
    void                          curveNameUiOrdering( caf::PdmUiOrdering& uiOrdering );

private:
    bool canCurveBeAttached() const;
    void attachCurveAndErrorBars();
    void checkAndApplyDefaultFillColor();

protected:
    QPointer<QwtPlot> m_parentQwtPlot;

    RiuQwtPlotCurve*      m_qwtPlotCurve;
    QwtPlotIntervalCurve* m_qwtCurveErrorBars;

    caf::PdmField<bool>    m_showCurve;
    caf::PdmField<QString> m_curveName;
    caf::PdmField<QString> m_customCurveName;
    caf::PdmField<bool>    m_showLegend;
    caf::PdmField<QString> m_symbolLabel;
    caf::PdmField<int>     m_symbolSize;
    caf::PdmField<QString> m_legendEntryText;

    caf::PdmField<bool>         m_isUsingAutoName;
    caf::PdmField<cvf::Color3f> m_curveColor;
    caf::PdmField<int>          m_curveThickness;
    caf::PdmField<float>        m_symbolSkipPixelDistance;
    caf::PdmField<bool>         m_showErrorBars;

    caf::PdmField<PointSymbol>        m_pointSymbol;
    caf::PdmField<LineStyle>          m_lineStyle;
    caf::PdmField<FillStyle>          m_fillStyle;
    caf::PdmField<cvf::Color3f>       m_fillColor;
    caf::PdmField<CurveInterpolation> m_curveInterpolation;
    caf::PdmField<LabelPosition>      m_symbolLabelPosition;
    caf::PdmField<cvf::Color3f>       m_symbolEdgeColor;

    caf::PdmField<bool> m_stackCurve;
    caf::PdmField<bool> m_stackWithPhaseColors;
};
