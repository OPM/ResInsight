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

#include "RimPlotCurveAppearance.h"

#include "RiaCurveDataTools.h"
#include "RiaPlotDefines.h"

#include "RiuPlotAxis.h"
#include "RiuQwtPlotCurveDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

#include <QPointer>

class RiuPlotCurve;
class RiuPlotWidget;
class RimPlotRectAnnotation;
class RifEclipseSummaryAddress;

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

public:
    RimPlotCurve();
    ~RimPlotCurve() override;

    void loadDataAndUpdate( bool updateParentPlot );

    void                                setColor( const cvf::Color3f& color );
    cvf::Color3f                        color() const;
    void                                setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum lineStyle );
    void                                setSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle );
    void                                setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum );
    RiuPlotCurveSymbol::PointSymbolEnum symbol();
    int                                 symbolSize() const;
    cvf::Color3f                        symbolEdgeColor() const;
    void                                setSymbolEdgeColor( const cvf::Color3f& edgeColor );
    void                                setSymbolSkipDistance( float distance );
    void                                setSymbolLabel( const QString& label );
    void                                setSymbolLabelPosition( RiuPlotCurveSymbol::LabelPosition labelPosition );
    void                                setSymbolSize( int sizeInPixels );
    void                                setLineThickness( int thickness );
    void                                resetAppearance();
    virtual Qt::BrushStyle              fillStyle() const;
    void                                setFillStyle( Qt::BrushStyle brushStyle );
    void                                setFillColor( const cvf::Color3f& fillColor );
    void                                setFillColorOpacity( float opacity );
    void                                setCurveColorOpacity( float opacity );

    bool isChecked() const;
    void setCheckState( bool isChecked );

    // The check state of the curve (m_showCurve) can automatically be updated based on presens of curve data. The virtual method
    // isAnyCurveDataPresent() can be overridden. Similar concept is used in RimWellLogTrack
    void         setAutoCheckStateBasedOnCurveData( bool enable );
    void         updateCheckStateBasedOnCurveData();
    virtual bool isAnyCurveDataPresent() const;

    void updateCurveName();
    void updateCurveNameAndUpdatePlotLegendAndTitle();
    void updateCurveNameNoLegendUpdate();
    void setCurveNameTemplateText( const QString& templateText );

    void            setNamingMethod( RiaDefines::ObjectNamingMethod namingMethod );
    QString         curveName() const;
    virtual QString curveExportDescription( const RifEclipseSummaryAddress& address ) const;
    virtual QString createCurveNameFromTemplate( const QString& templateText );

    void setCustomName( const QString& customName );
    void setLegendEntryText( const QString& legendEntryText );

    void         updateCurveVisibility( bool updateParentPlot = true );
    void         updateLegendEntryVisibilityAndPlotLegend();
    virtual void updateLegendEntryVisibilityNoPlotUpdate();
    virtual void replotParentPlot();

    bool showInLegend() const;
    bool errorBarsVisible() const;

    void         setShowInLegend( bool show );
    virtual void setZOrder( double z );
    void         setErrorBarsVisible( bool isVisible );

    virtual void updateCurveAppearance();
    virtual void updateUiIconFromPlotSymbol();
    virtual bool hasParentPlot() const;

    void updateCurveAppearanceForFilesOlderThan_2021_06();

    virtual bool xValueRange( double* minimumValue, double* maximumValue ) const;
    virtual bool yValueRange( double* minimumValue, double* maximumValue ) const;

    void setTitle( const QString& title );

    int                        dataSize() const;
    std::pair<double, double>  sample( int index ) const;
    virtual double             closestYValueForX( double xValue ) const;
    std::vector<RimPlotCurve*> additionalDataSources() const;

    void setParentPlotNoReplot( RiuPlotWidget* );
    void setParentPlotAndReplot( RiuPlotWidget* );

    void          attach( RiuPlotWidget* );
    void          detach( bool deletePlotCurve = false );
    void          reattach( bool updateParentPlot = true );
    bool          isSameCurve( const RiuPlotCurve* plotCurve ) const;
    void          deletePlotCurve();
    RiuPlotCurve* plotCurve() const;

    std::vector<RimPlotRectAnnotation*> rectAnnotations() const;

    void setSamplesFromXYValues( const std::vector<double>& xValues, const std::vector<double>& yValues, bool useLogarithmicScale );

protected:
    virtual QString createCurveAutoName();

    virtual QStringList supportedCurveNameVariables() const;

    virtual void updateZoomInParentPlot();
    virtual void onLoadDataAndUpdate( bool updateParentPlot );
    void         initAfterRead() override;
    void         updateCurvePresentation( bool updatePlotLegendAndTitle );

    void         updateFieldUiState();
    void         updatePlotTitle();
    virtual void updateLegendsInPlot();

    void setSamplesFromXYErrorValues( const std::vector<double>&   xValues,
                                      const std::vector<double>&   yValues,
                                      const std::vector<double>&   errorValues,
                                      bool                         useLogarithmicScale,
                                      RiaCurveDataTools::ErrorAxis errorAxis = RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS );

    void setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes, const std::vector<double>& yValues, bool useLogarithmicScale );

    void setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes, const std::vector<double>& yValues, bool useLogarithmicScale );

    virtual double computeCurveZValue();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle*          objectToggleField() override;
    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void appearanceUiOrdering( caf::PdmUiOrdering& uiOrdering );
    void curveNameUiOrdering( caf::PdmUiOrdering& uiOrdering );
    void additionalDataSourcesUiOrdering( caf::PdmUiOrdering& uiOrdering );

    void         onCurveAppearanceChanged( const caf::SignalEmitter* emitter );
    virtual void onFillColorChanged( const caf::SignalEmitter* emitter );

    virtual bool canCurveBeAttached() const;
    virtual void clearErrorBars();
    void         checkAndApplyDefaultFillColor();

    void updateYAxisInPlot( RiuPlotAxis plotAxis );
    void updateXAxisInPlot( RiuPlotAxis plotAxis );

    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void onColorTagClicked( const SignalEmitter* emitter, size_t index );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    bool isCurveNameTemplateSupported() const;

protected:
    caf::PdmField<bool> m_showCurve;
    caf::PdmField<bool> m_autoCheckStateBasedOnCurveData;

    caf::PdmField<QString> m_curveName;
    caf::PdmField<QString> m_curveNameTemplateText;

    caf::PdmField<caf::AppEnum<RiaDefines::ObjectNamingMethod>> m_namingMethod;

    caf::PdmField<QString> m_legendEntryText;

    caf::PdmField<bool> m_showLegend;
    caf::PdmField<bool> m_showErrorBars;

    caf::PdmChildField<RimPlotCurveAppearance*> m_curveAppearance;

    caf::PdmPtrArrayField<RimPlotCurve*>            m_additionalDataSources;
    caf::PdmChildArrayField<RimPlotRectAnnotation*> m_rectAnnotations;

    QPointer<RiuPlotWidget> m_parentPlot;
    RiuPlotCurve*           m_plotCurve;

    caf::PdmField<bool>                                       m_isUsingAutoName_OBSOLETE;
    caf::PdmField<QString>                                    m_symbolLabel_OBSOLETE;
    caf::PdmField<int>                                        m_symbolSize_OBSOLETE;
    caf::PdmField<cvf::Color3f>                               m_curveColor_OBSOLETE;
    caf::PdmField<int>                                        m_curveThickness_OBSOLETE;
    caf::PdmField<float>                                      m_symbolSkipPixelDistance_OBSOLETE;
    caf::PdmField<RimPlotCurveAppearance::PointSymbol>        m_pointSymbol_OBSOLETE;
    caf::PdmField<RimPlotCurveAppearance::LineStyle>          m_lineStyle_OBSOLETE;
    caf::PdmField<RimPlotCurveAppearance::FillStyle>          m_fillStyle_OBSOLETE;
    caf::PdmField<cvf::Color3f>                               m_fillColor_OBSOLETE;
    caf::PdmField<RimPlotCurveAppearance::CurveInterpolation> m_curveInterpolation_OBSOLETE;
    caf::PdmField<RimPlotCurveAppearance::LabelPosition>      m_symbolLabelPosition_OBSOLETE;
    caf::PdmField<cvf::Color3f>                               m_symbolEdgeColor_OBSOLETE;
};
