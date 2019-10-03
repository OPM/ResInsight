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

#include "RimRegularLegendConfig.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

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
class RiuWellPathComponentPlotItem;
class RiuWellLogTrack;
class RigEclipseWellLogExtractor;
class RimWellLogPlotCollection;
class RigGeoMechWellLogExtractor;
class RigResultAccessor;
class RigFemResultAddress;

class QwtPlotCurve;

struct CurveSamplingPointData
{
    std::vector<double> data;
    std::vector<double> md;
    std::vector<double> tvd;
};

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogTrack : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogTrack();
    ~RimWellLogTrack() override;

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
    enum WidthScaleFactor
    {
        EXTRA_NARROW_TRACK = 3,
        NARROW_TRACK       = 4,
        NORMAL_TRACK       = 5,
        WIDE_TRACK         = 7,
        EXTRA_WIDE_TRACK   = 10
    };

    typedef caf::AppEnum<RiuPlotAnnotationTool::RegionAnnotationType> RegionAnnotationTypeEnum;
    typedef caf::AppEnum<RiuPlotAnnotationTool::RegionDisplay>        RegionAnnotationDisplayEnum;

    void setDescription( const QString& description );
    bool isVisible();
    void setVisible( bool visible );
    void addCurve( RimWellLogCurve* curve );
    void insertCurve( RimWellLogCurve* curve, size_t index );
    void takeOutCurve( RimWellLogCurve* curve );
    void deleteAllCurves();

    size_t curveIndex( RimWellLogCurve* curve );
    size_t curveCount()
    {
        return curves.size();
    }

    void    setXAxisTitle( const QString& text );
    QString depthPlotTitle() const;
    int     widthScaleFactor() const;
    void    setWidthScaleFactor( WidthScaleFactor scaleFactor );

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

    void recreateViewer();
    void detachAllCurves();
    void reattachAllCurves();

    void loadDataAndUpdate( bool updateParentPlotAndToolbars = false );

    void setAndUpdateWellPathFormationNamesData( RimCase* rimCase, RimWellPath* wellPath );

    void setAndUpdateSimWellFormationNamesAndBranchData( RimCase*       rimCase,
                                                         const QString& simWellName,
                                                         int            branchIndex,
                                                         bool           useBranchDetection );
    void setAndUpdateSimWellFormationNamesData( RimCase* rimCase, const QString& simWellName );

    void setAutoScaleXEnabled( bool enabled );
    void availableDepthRange( double* minimumDepth, double* maximumDepth );
    void updateParentPlotZoom();
    void calculateXZoomRangeAndUpdateQwt();
    void applyXZoomFromVisibleRange();
    void calculateXZoomRange();
    void updateEditors();
    void setVisibleXRange( double minValue, double maxValue );
    void setTickIntervals( double majorTickInterval, double minorTickInterval );
    void setXAxisGridVisibility( RimWellLogPlot::AxisGridVisibility gridLines );

    void setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType annotationType );
    void setAnnotationDisplay( RiuPlotAnnotationTool::RegionDisplay annotationDisplay );

    RiuPlotAnnotationTool::RegionAnnotationType annotationType() const;
    RiuPlotAnnotationTool::RegionDisplay        annotationDisplay() const;

    bool showFormations() const;

    void setShowRegionLabels( bool on );

    bool showWellPathAttributes() const;
    void setShowWellPathAttributes( bool on );
    void setWellPathAttributesSource( RimWellPath* wellPath );

    RimWellPath*     wellPathAttributeSource() const;
    RiuWellLogTrack* viewer();

    RimWellLogCurve* curveDefinitionFromCurve( const QwtPlotCurve* curve ) const;

    void setLogarithmicScale( bool enable );

    std::map<int, std::vector<RimWellFlowRateCurve*>> visibleStackedCurves();

    QString                       description();
    std::vector<RimWellLogCurve*> curvesVector();
    std::vector<RimWellLogCurve*> visibleCurvesVector();

    void uiOrderingForRftPltFormations( caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForXAxisSettings( caf::PdmUiOrdering& uiOrdering );

    void setFormationsForCaseWithSimWellOnly( bool caseWithSimWellOnly );
    void updateAxisAndGridTickIntervals();

    void updateAllLegendItems();

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;
    void                          updateParentPlotLayout();
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    caf::PdmFieldHandle* objectToggleField() override;
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 initAfterRead() override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

    void computeAndSetXRangeMinForLogarithmicScale();

    static void simWellOptionItems( QList<caf::PdmOptionItemInfo>* options, RimCase* eclCase );

    static RigEclipseWellLogExtractor* createSimWellExtractor( RimWellLogPlotCollection* wellLogCollection,
                                                               RimCase*                  rimCase,
                                                               const QString&            simWellName,
                                                               int                       branchIndex,
                                                               bool                      useBranchDetection );

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

    void setFormationFieldsUiReadOnly( bool readOnly = true );

    void updateRegionAnnotationsOnPlot();
    void updateFormationNamesOnPlot();
    void updateCurveDataRegionsOnPlot();
    void updateWellPathAttributesOnPlot();
    void removeRegionAnnotations();
    void updateAxisScaleEngine();
    bool isFirstVisibleTrackInPlot() const;

    std::pair<double, double> adjustXRange( double minValue, double maxValue, double tickInterval );

    void updateWellPathAttributesCollection();

private:
    QString m_xAxisTitle;

    caf::PdmField<bool>                         m_show;
    caf::PdmField<QString>                      m_userName;
    caf::PdmChildArrayField<RimWellLogCurve*>   curves;
    caf::PdmField<double>                       m_visibleXRangeMin;
    caf::PdmField<double>                       m_visibleXRangeMax;
    caf::PdmField<bool>                         m_isAutoScaleXEnabled;
    caf::PdmField<bool>                         m_isLogarithmicScaleEnabled;
    caf::PdmField<RimWellLogPlot::AxisGridEnum> m_xAxisGridVisibility;
    caf::PdmField<bool>                         m_explicitTickIntervals;
    caf::PdmField<double>                       m_majorTickInterval;
    caf::PdmField<double>                       m_minorTickInterval;

    caf::PdmField<RegionAnnotationTypeEnum>                            m_regionAnnotationType;
    caf::PdmField<RegionAnnotationDisplayEnum>                         m_regionAnnotationDisplay;
    caf::PdmField<RimRegularLegendConfig::ColorRangeEnum>              m_colorShadingPalette;
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
    caf::PdmField<caf::AppEnum<WidthScaleFactor>>                      m_widthScaleFactor;
    caf::PdmField<bool>                                                m_formationBranchDetection;
    caf::PdmField<bool>                                                m_showWellPathAttributes;
    caf::PdmField<bool>                                                m_showWellPathCompletions;
    caf::PdmField<bool>                                                m_showWellPathComponentsBothSides;
    caf::PdmField<bool>                                                m_showWellPathComponentLabels;
    caf::PdmField<bool>                                                m_wellPathAttributesInLegend;
    caf::PdmField<bool>                                                m_wellPathCompletionsInLegend;
    caf::PdmPtrField<RimWellPath*>                                     m_wellPathComponentSource;
    caf::PdmPtrField<RimWellPathAttributeCollection*>                  m_wellPathAttributeCollection;

    caf::PdmField<bool> m_showFormations_OBSOLETE;

    std::vector<std::unique_ptr<RiuWellPathComponentPlotItem>> m_wellPathAttributePlotObjects;

    bool m_formationsForCaseWithSimWellOnly;

    QPointer<RiuWellLogTrack> m_wellLogTrackPlotWidget;

    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
};
