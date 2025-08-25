/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "RiaPlotDefines.h"

#include "RimAbstractPlotCollection.h"
#include "RimEnsembleWellLogStatistics.h"
#include "RimPlot.h"
#include "RimPlotWindow.h"

#include "RiuPlotCurveInfoTextProvider.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <set>

class RimWellLogCurveCommonDataSource;
class RiuWellLogPlot;
class RimPlot;
class RimEnsembleCurveSet;
class RiuPlotAxis;
class RimWellLogTrack;
class RimWellLogPlotNameConfig;
class RimPlotAxisAnnotation;

class QKeyEvent;

//==================================================================================================
///
///
//==================================================================================================
class RimDepthTrackPlot : public RimTypedPlotCollection<RimPlot>, public RimPlotWindow, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum AxisGridVisibility
    {
        AXIS_GRID_NONE            = 0x00,
        AXIS_GRID_MAJOR           = 0x01,
        AXIS_GRID_MINOR           = 0x02,
        AXIS_GRID_MAJOR_AND_MINOR = 0x03
    };

    using AxisGridEnum  = caf::AppEnum<AxisGridVisibility>;
    using DepthTypeEnum = RiaDefines::DepthTypeEnum;

    enum class DepthOrientation_OBSOLETE
    {
        HORIZONTAL,
        VERTICAL
    };

public:
    RimDepthTrackPlot();
    ~RimDepthTrackPlot() override;

    RimDepthTrackPlot& operator=( RimDepthTrackPlot&& rhs );

    QWidget* viewWidget() override;
    QWidget* createPlotWidget( QWidget* mainWindowParent = nullptr );
    QString  description() const override;

    size_t   plotCount() const override;
    size_t   plotIndex( const RimWellLogTrack* plot ) const;
    RimPlot* plotByIndex( size_t index ) const;

    int columnCount() const override;

    std::vector<RimPlot*>         plots() const override;
    std::vector<RimWellLogTrack*> visiblePlots() const;
    void                          insertPlot( RimPlot* plot, size_t index ) final;
    void                          removePlot( RimPlot* plot ) final;

    DepthTypeEnum depthType() const;
    void          setDepthType( DepthTypeEnum depthType );

    RiaDefines::DepthUnitType depthUnit() const;
    void                      setDepthUnit( RiaDefines::DepthUnitType depthUnit );

    QString            depthAxisTitle() const;
    void               enableDepthAxisGridLines( AxisGridVisibility gridVisibility );
    AxisGridVisibility depthAxisGridLinesEnabled() const;

    RiaDefines::Orientation depthOrientation() const;
    void                    setDepthOrientation( RiaDefines::Orientation depthOrientation );

    RiaDefines::MultiPlotAxisVisibility depthAxisVisibility() const;
    void                                setDepthAxisVisibility( RiaDefines::MultiPlotAxisVisibility axisVisibility );

    RiuPlotAxis depthAxis() const;
    RiuPlotAxis valueAxis() const;
    RiuPlotAxis annotationAxis() const;

    void setAutoScalePropertyValuesEnabled( bool enabled );
    void setAutoScaleDepthValuesEnabled( bool enabled );

    void zoomAll() override;
    void updateZoom();
    void setDepthAxisRangeByFactorAndCenter( double zoomFactor, double zoomCenter );
    void setDepthAxisRangeByPanDepth( double panFactor );
    void setDepthAxisRange( double minimumDepth, double maximumDepth );

    void calculateAvailableDepthRange();
    void availableDepthRange( double* minimumDepth, double* maximumDepth ) const;
    void visibleDepthRange( double* minimumDepth, double* maximumDepth ) const;

    void                                enableDepthMarkerLine( bool enable );
    bool                                isDepthMarkerLineEnabled() const;
    void                                setDepthMarkerPosition( double depth );
    void                                clearDepthAnnotations();
    std::vector<RimPlotAxisAnnotation*> depthAxisAnnotations() const;
    void                                setAutoZoomMinimumDepthFactor( double factor );
    void                                setAutoZoomMaximumDepthFactor( double factor );

    void uiOrderingForDepthAxis( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForAutoName( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    QString                   createAutoName() const override;
    RimWellLogPlotNameConfig* nameConfig() const;
    void                      setNameTemplateText( const QString& templateText );
    void                      setNamingMethod( RiaDefines::ObjectNamingMethod namingMethod );

    RimWellLogCurveCommonDataSource* commonDataSource() const;
    void                             updateCommonDataSource();
    void                             setCommonDataSourceEnabled( bool enable );

    void setAvailableDepthUnits( const std::set<RiaDefines::DepthUnitType>& depthUnits );
    void setAvailableDepthTypes( const std::set<DepthTypeEnum>& depthTypes );

    QString asciiDataForPlotExport() const;
    void    handleKeyPressEvent( QKeyEvent* keyEvent );

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    int subTitleFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

    RiaDefines::DepthUnitType caseDepthUnit() const;

    void updateDepthAxisVisibility();

    static RiuPlotAxis depthAxis( RiaDefines::Orientation depthOrientation );
    static RiuPlotAxis valueAxis( RiaDefines::Orientation depthOrientation );
    static RiuPlotAxis annotationAxis( RiaDefines::Orientation depthOrientation );

    void updateTrackVisibility();

protected:
    QImage snapshotWindowContent() override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    void                               performAutoNameUpdate() override;
    QString                            createPlotNameFromTemplate( const QString& templateText ) const override;
    QStringList                        supportedPlotNameVariables() const override;
    virtual std::map<QString, QString> createNameKeyValueMap() const;

    void recreatePlotWidgets();

    // Overridden PDM methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void initAfterRead() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void onLoadDataAndUpdate() override;
    void updatePlots();
    caf::PdmFieldHandle* userDescriptionField() override;

    void uiOrderingForFonts( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering );

protected:
    bool                                                 m_commonDataSourceEnabled;
    caf::PdmChildField<RimWellLogCurveCommonDataSource*> m_commonDataSource;
    caf::PdmChildField<RimWellLogPlotNameConfig*>        m_nameConfig;
    caf::PdmField<caf::AppEnum<DepthTypeEnum>>           m_depthType;
    caf::PdmField<caf::AppEnum<RiaDefines::Orientation>> m_depthOrientation;

private:
    void cleanupBeforeClose();
    void updateSubPlotNames();
    void onPlotAdditionOrRemoval();
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;
    void doUpdateLayout() override;
    void onPlotsReordered( const caf::SignalEmitter* emitter );

    static void                          createPlotWidgetAndAttachCurveTextProvider( RimWellLogTrack* track );
    static RiuPlotCurveInfoTextProvider* curveTextProvider();

private:
    caf::PdmField<QString> m_plotWindowTitle;
    caf::PdmField<QString> m_nameTemplateText;

    caf::PdmField<caf::AppEnum<RiaDefines::ObjectNamingMethod>> m_namingMethod;

    // Depth axis
    caf::PdmField<caf::AppEnum<RiaDefines::DepthUnitType>>           m_depthUnit;
    caf::PdmField<double>                                            m_minVisibleDepth;
    caf::PdmField<double>                                            m_maxVisibleDepth;
    caf::PdmField<AxisGridEnum>                                      m_depthAxisGridVisibility;
    caf::PdmField<bool>                                              m_isAutoScaleDepthEnabled;
    caf::PdmField<caf::AppEnum<RiaDefines::MultiPlotAxisVisibility>> m_depthAxisVisibility;
    caf::PdmField<bool>                                              m_showDepthMarkerLine;
    caf::PdmField<double>                                            m_autoZoomMinDepthFactor;
    caf::PdmField<double>                                            m_autoZoomMaxDepthFactor;
    caf::PdmChildArrayField<RimPlotAxisAnnotation*>                  m_depthAnnotations;

    RimFontSizeField m_subTitleFontSize;
    RimFontSizeField m_axisTitleFontSize;
    RimFontSizeField m_axisValueFontSize;

    caf::PdmChildArrayField<RimWellLogTrack*> m_plots;

    caf::PdmField<caf::AppEnum<RimEnsembleWellLogStatistics::DepthEqualization>> m_depthEqualization;
    caf::PdmPtrField<RimEnsembleCurveSet*>                                       m_ensembleCurveSet;

    QPointer<RiuWellLogPlot>            m_viewer;
    std::set<RiaDefines::DepthUnitType> m_availableDepthUnits;
    std::set<DepthTypeEnum>             m_availableDepthTypes;

    double m_minAvailableDepth;
    double m_maxAvailableDepth;
};
