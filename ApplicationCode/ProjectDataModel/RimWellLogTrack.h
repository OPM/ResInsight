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

#include "RimPlotInterface.h"
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
class RimWellLogTrack : public caf::PdmObject, public RimPlotInterface
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

    typedef caf::AppEnum<RiuPlotAnnotationTool::RegionAnnotationType> RegionAnnotationTypeEnum;
    typedef caf::AppEnum<RiuPlotAnnotationTool::RegionDisplay>        RegionAnnotationDisplayEnum;

    bool    isChecked() const override;
    void    setChecked( bool checked ) override;
    QString description() const override;
    void    setDescription( const QString& description );
    int     widthScaleFactor() const override;
    void    setWidthScaleFactor( WidthScaleFactor scaleFactor ) override;

    void addCurve( RimWellLogCurve* curve );
    void insertCurve( RimWellLogCurve* curve, size_t index );
    void takeOutCurve( RimWellLogCurve* curve );
    void deleteAllCurves();

    size_t curveIndex( RimWellLogCurve* curve );
    size_t curveCount()
    {
        return m_curves.size();
    }

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

    void createPlotWidget();
    void detachAllCurves();
    void reattachAllCurves() override;

    void loadDataAndUpdate() override;

    void setAndUpdateWellPathFormationNamesData( RimCase* rimCase, RimWellPath* wellPath );

    void setAndUpdateSimWellFormationNamesAndBranchData( RimCase*       rimCase,
                                                         const QString& simWellName,
                                                         int            branchIndex,
                                                         bool           useBranchDetection );
    void setAndUpdateSimWellFormationNamesData( RimCase* rimCase, const QString& simWellName );

    void setAutoScaleXEnabled( bool enabled ) override;
    void setAutoScaleYEnabled( bool enabled ) override;

    void availableXAxisRange( double* minX, double* maxX );
    void availableDepthRange( double* minimumDepth, double* maximumDepth );

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

    RiuPlotAnnotationTool::RegionAnnotationType annotationType() const;
    RiuPlotAnnotationTool::RegionDisplay        annotationDisplay() const;

    bool showFormations() const;

    void setShowRegionLabels( bool on );

    bool showWellPathAttributes() const;
    void setShowWellPathAttributes( bool on );
    void setWellPathAttributesSource( RimWellPath* wellPath );

    RimWellPath*      wellPathAttributeSource() const;
    RiuQwtPlotWidget* viewer() override;

    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

    void setLogarithmicScale( bool enable );

    std::map<int, std::vector<RimWellFlowRateCurve*>> visibleStackedCurves();

    std::vector<RimWellLogCurve*> curves() const;
    std::vector<RimWellLogCurve*> visibleCurves() const;

    void uiOrderingForRftPltFormations( caf::PdmUiOrdering& uiOrdering );
    void uiOrderingForXAxisSettings( caf::PdmUiOrdering& uiOrdering );

    void setFormationsForCaseWithSimWellOnly( bool caseWithSimWellOnly );
    void updateXAxisAndGridTickIntervals();

    void updateAllLegendItems();

    QString asciiDataForPlotExport() const;

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

    void onAxisSelected( int axis, bool toggle ) override;

    void updateAxes() override;

private:
    void calculateXZoomRange();
    void calculateYZoomRange();

    void updateXZoom();
    void updateYZoom();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* objectToggleField() override;
    caf::PdmFieldHandle* userDescriptionField() override;

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

    std::pair<double, double> adjustXRange( double minValue, double maxValue, double tickInterval );

    void updateWellPathAttributesCollection();

    void onWidthScaleFactorChange() override;

    RimWellLogPlot* parentWellLogPlot() const;

private:
    QString m_xAxisTitle;

    caf::PdmField<bool>                                   m_show;
    caf::PdmField<QString>                                m_description;
    caf::PdmField<RimPlotInterface::WidthScaleFactorEnum> m_widthScaleFactor;

    caf::PdmChildArrayField<RimWellLogCurve*> m_curves;
    caf::PdmField<double>                     m_visibleXRangeMin;
    caf::PdmField<double>                     m_visibleXRangeMax;
    caf::PdmField<double>                     m_visibleYRangeMin;
    caf::PdmField<double>                     m_visibleYRangeMax;

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

    QPointer<RiuWellLogTrack>              m_plotWidget;
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
};
