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

#include "RimWellLogPlot.h"

#include "RigWellPathFormations.h"
#include "RiuPlotAnnotationTool.h"

#include "RimPlot.h"
#include "RimRegularLegendConfig.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "qwt_plot.h"

#include <QPointer>

#include <map>
#include <memory>
#include <vector>

class RigWellPath;
class RimCase;
class RimWellPathAttributeCollection;
class RimWellFlowRateCurve;
class RimWellLogCurve;
class RimWellPath;
class RimDepthTrackPlot;
class RiuWellPathComponentPlotItem;
class RiuWellLogTrack;
class RigEclipseWellLogExtractor;
class RimWellLogPlotCollection;
class RigGeoMechWellLogExtractor;
class RigResultAccessor;
class RigFemResultAddress;
class RigWellLogExtractor;
class RimEclipseResultDefinition;
class RimColorLegend;

class QwtPlotCurve;

struct CurveSamplingPointData
{
    std::vector<double> data;
    std::vector<double> md;
    std::vector<double> tvd;

    double rkbDiff;
};

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogTrack : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum TrajectoryType
    {
        WELL_PATH,
        SIMULATION_WELL
    };
    enum FormationSource
    {
        CASE,
        WELL_PICK_FILTER
    };

    using RegionAnnotationTypeEnum    = caf::AppEnum<RiuPlotAnnotationTool::RegionAnnotationType>;
    using RegionAnnotationDisplayEnum = caf::AppEnum<RiuPlotAnnotationTool::RegionDisplay>;

public:
    RimWellLogTrack();
    ~RimWellLogTrack() override;

    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer() override;
    QImage            snapshotWindowContent() override;
    void              zoomAll() override;

    QString description() const override;
    void    setDescription( const QString& description );

    void addCurve( RimWellLogCurve* curve );
    void insertCurve( RimWellLogCurve* curve, size_t index );
    void removeCurve( RimWellLogCurve* curve );
    void deleteAllCurves();

    size_t curveIndex( RimWellLogCurve* curve );
    size_t curveCount() { return m_curves.size(); }

    void    setXAxisTitle( const QString& text );
    QString yAxisTitle() const;

    void           setFormationWellPath( RimWellPath* wellPath );
    RimWellPath*   formationWellPath() const;
    void           setFormationSimWellName( const QString& simWellName );
    QString        formationSimWellName() const;
    void           setFormationBranchDetection( bool branchDetection );
    bool           formationBranchDetection() const;
    void           setFormationBranchIndex( int branchIndex );
    int            formationBranchIndex() const;
    void           setFormationCase( RimCase* rimCase );
    RimCase*       formationNamesCase() const;
    void           setFormationTrajectoryType( TrajectoryType trajectoryType );
    TrajectoryType formationTrajectoryType() const;
    void setRegionPropertyResultType( RiaDefines::ResultCatType resultCatType, const QString& resultVariable );

    void detachAllCurves() override;
    void reattachAllCurves() override;

    void setAndUpdateWellPathFormationNamesData( RimCase* rimCase, RimWellPath* wellPath );

    void setAndUpdateSimWellFormationNamesAndBranchData( RimCase*       rimCase,
                                                         const QString& simWellName,
                                                         int            branchIndex,
                                                         bool           useBranchDetection );
    void setAndUpdateSimWellFormationNamesData( RimCase* rimCase, const QString& simWellName );

    void setAutoScaleXEnabled( bool enabled ) override;
    void setAutoScaleYEnabled( bool enabled ) override;
    void setAutoScaleXIfNecessary();

    void availableXAxisRange( double* minX, double* maxX );
    void availableDepthRange( double* minimumDepth, double* maximumDepth );

    void visibleXAxisRange( double* minX, double* maxX );
    void visibleDepthRange( double* minimumDepth, double* maximumDepth );

    void setVisibleXRange( double minValue, double maxValue );
    void setVisibleYRange( double minValue, double maxValue );

    void updateZoomInQwt() override;
    void updateZoomFromQwt() override;

    void updateParentPlotZoom();

    void updateEditors();

    void setTickIntervals( double majorTickInterval, double minorTickInterval );
    void setXAxisGridVisibility( RimWellLogPlot::AxisGridVisibility gridLines );

    void setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType annotationType );
    void setAnnotationDisplay( RiuPlotAnnotationTool::RegionDisplay annotationDisplay );
    void setAnnotationTransparency( int percent );
    void setColorShadingLegend( RimColorLegend* colorLegend );

    RiuPlotAnnotationTool::RegionAnnotationType annotationType() const;
    RiuPlotAnnotationTool::RegionDisplay        annotationDisplay() const;

    bool showFormations() const;

    void setShowRegionLabels( bool on );

    bool showWellPathAttributes() const;
    void setShowWellPathAttributes( bool on );
    void setShowWellPathAttributesInLegend( bool on );
    void setShowWellPathCompletionsInLegend( bool on );
    void setShowBothSidesOfWell( bool on );
    void setWellPathAttributesSource( RimWellPath* wellPath );

    void setOverburdenHeight( double overburdenHeight );
    void setUnderburdenHeight( double underburdenHeight );

    RimWellPath* wellPathAttributeSource() const;

    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

    void setLogarithmicScale( bool enable );

    std::map<int, std::vector<RimWellLogCurve*>> visibleStackedCurves();

    std::vector<RimWellLogCurve*> curves() const;
    std::vector<RimWellLogCurve*> visibleCurves() const;

    void uiOrderingForRftPltFormations( caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForXAxisSettings( caf::PdmUiOrdering& uiOrdering );

    void setFormationsForCaseWithSimWellOnly( bool caseWithSimWellOnly );
    void updateXAxisAndGridTickIntervals();

    void updateLegend() override;

    QString asciiDataForPlotExport() const override;

    void onAxisSelected( int axis, bool toggle ) override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void updateAxes() override;

    static CurveSamplingPointData curveSamplingPointData( RigEclipseWellLogExtractor* extractor,
                                                          RigResultAccessor*          resultAccessor );
    static CurveSamplingPointData curveSamplingPointData( RigGeoMechWellLogExtractor* extractor,
                                                          const RigFemResultAddress&  resultAddress );

    static void findRegionNamesToPlot( const CurveSamplingPointData&           curveData,
                                       const std::vector<QString>&             formationNamesVector,
                                       RimWellLogPlot::DepthTypeEnum           depthType,
                                       std::vector<QString>*                   formationNamesToPlot,
                                       std::vector<std::pair<double, double>>* yValues );

    static std::vector<QString> formationNamesVector( RimCase* rimCase );
    void                        updateStackedCurveData();

    static void addOverburden( std::vector<QString>& namesVector, CurveSamplingPointData& curveData, double height );
    static void addUnderburden( std::vector<QString>& namesVector, CurveSamplingPointData& curveData, double height );

protected:
    // RimViewWindow overrides
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void cleanupBeforeClose();
    void detachAllPlotItems();
    void calculateXZoomRange();
    void calculateYZoomRange();

    void updateXZoom();
    void updateYZoom();

    int axisFontSize() const;

    void doRemoveFromCollection() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void curveDataChanged( const caf::SignalEmitter* emitter );
    void curveVisibilityChanged( const caf::SignalEmitter* emitter, bool visible );
    void curveAppearanceChanged( const caf::SignalEmitter* emitter );
    void curveStackingChanged( const caf::SignalEmitter* emitter, bool stacked );
    void curveStackingColorsChanged( const caf::SignalEmitter* emitter, bool stackWithPhaseColors );

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void computeAndSetXRangeMinForLogarithmicScale();

    static void simWellOptionItems( QList<caf::PdmOptionItemInfo>* options, RimCase* eclCase );

    static RigEclipseWellLogExtractor* createSimWellExtractor( RimWellLogPlotCollection* wellLogCollection,
                                                               RimCase*                  rimCase,
                                                               const QString&            simWellName,
                                                               int                       branchIndex,
                                                               bool                      useBranchDetection );

    void setFormationFieldsUiReadOnly( bool readOnly = true );

    void updateRegionAnnotationsOnPlot();
    void updateFormationNamesOnPlot();
    void updateResultPropertyNamesOnPlot();
    void updateCurveDataRegionsOnPlot();
    void updateWellPathAttributesOnPlot();
    void removeRegionAnnotations();
    void updateAxisScaleEngine();

    std::pair<double, double> adjustXRange( double minValue, double maxValue, double tickInterval );

    void updateWellPathAttributesCollection();

    RimDepthTrackPlot* parentWellLogPlot() const;

    void handleWheelEvent( QWheelEvent* event ) override;
    void doUpdateLayout() override;

    std::vector<std::pair<double, double>> waterAndRockRegions( RiaDefines::DepthTypeEnum  depthType,
                                                                const RigWellLogExtractor* extractor ) const;

    void connectCurveSignals( RimWellLogCurve* curve );
    void disconnectCurveSignals( RimWellLogCurve* curve );

private:
    QString m_xAxisTitle;

    caf::PdmField<QString> m_description;

    caf::PdmChildArrayField<RimWellLogCurve*> m_curves;
    caf::PdmField<double>                     m_visibleXRangeMin;
    caf::PdmField<double>                     m_visibleXRangeMax;
    caf::PdmField<double>                     m_visibleDepthRangeMin;
    caf::PdmField<double>                     m_visibleDepthRangeMax;

    caf::PdmField<bool>                         m_isAutoScaleXEnabled;
    caf::PdmField<bool>                         m_isLogarithmicScaleEnabled;
    caf::PdmField<RimWellLogPlot::AxisGridEnum> m_xAxisGridVisibility;
    caf::PdmField<bool>                         m_explicitTickIntervals;
    caf::PdmField<double>                       m_majorTickInterval;
    caf::PdmField<double>                       m_minorTickInterval;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisFontSize;

    caf::PdmField<RegionAnnotationTypeEnum>                            m_regionAnnotationType;
    caf::PdmField<RegionAnnotationDisplayEnum>                         m_regionAnnotationDisplay;
    caf::PdmPtrField<RimColorLegend*>                                  m_colorShadingLegend;
    caf::PdmField<int>                                                 m_colorShadingTransparency;
    caf::PdmField<bool>                                                m_showRegionLabels;
    caf::PdmField<caf::AppEnum<FormationSource>>                       m_formationSource;
    caf::PdmPtrField<RimCase*>                                         m_formationCase;
    caf::PdmField<caf::AppEnum<TrajectoryType>>                        m_formationTrajectoryType;
    caf::PdmPtrField<RimWellPath*>                                     m_formationWellPathForSourceCase;
    caf::PdmPtrField<RimWellPath*>                                     m_formationWellPathForSourceWellPath;
    caf::PdmField<QString>                                             m_formationSimWellName;
    caf::PdmField<int>                                                 m_formationBranchIndex;
    caf::PdmField<caf::AppEnum<RigWellPathFormations::FormationLevel>> m_formationLevel;
    caf::PdmField<bool>                                                m_showformationFluids;
    caf::PdmField<bool>                                                m_formationBranchDetection;
    caf::PdmField<bool>                                                m_showWellPathAttributes;
    caf::PdmField<bool>                                                m_showWellPathCompletions;
    caf::PdmField<bool>                                                m_showWellPathComponentsBothSides;
    caf::PdmField<bool>                                                m_showWellPathComponentLabels;
    caf::PdmField<bool>                                                m_wellPathAttributesInLegend;
    caf::PdmField<bool>                                                m_wellPathCompletionsInLegend;
    caf::PdmPtrField<RimWellPath*>                                     m_wellPathComponentSource;
    caf::PdmPtrField<RimWellPathAttributeCollection*>                  m_wellPathAttributeCollection;
    caf::PdmChildField<RimEclipseResultDefinition*>                    m_resultDefinition;
    caf::PdmField<double>                                              m_overburdenHeight;
    caf::PdmField<double>                                              m_underburdenHeight;

    caf::PdmField<bool>                                   m_showFormations_OBSOLETE;
    caf::PdmField<bool>                                   m_show_OBSOLETE;
    caf::PdmField<RimRegularLegendConfig::ColorRangeEnum> m_colorShadingPalette_OBSOLETE;

    std::vector<std::unique_ptr<RiuWellPathComponentPlotItem>> m_wellPathAttributePlotObjects;

    bool m_formationsForCaseWithSimWellOnly;

    QPointer<RiuWellLogTrack>              m_plotWidget;
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;

    double m_availableXRangeMin;
    double m_availableXRangeMax;
    double m_availableDepthRangeMin;
    double m_availableDepthRangeMax;
};
