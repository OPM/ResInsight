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

#include "RimExtrudedCurveIntersection.h"

#include "RiaVec3Tools.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEnsembleSurface.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCollection.h"
#include "RimSurfaceIntersectionCurve.h"
#include "RimTools.h"
#include "RimWellPath.h"

#include "RiuViewer.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfPlane.h"

namespace caf
{
template <>
void caf::AppEnum<RimExtrudedCurveIntersection::CrossSectionEnum>::setUp()
{
    addItem( RimExtrudedCurveIntersection::CrossSectionEnum::CS_WELL_PATH, "CS_WELL_PATH", "Well Path" );
    addItem( RimExtrudedCurveIntersection::CrossSectionEnum::CS_SIMULATION_WELL, "CS_SIMULATION_WELL", "Simulation Well" );
    addItem( RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE, "CS_POLYLINE", "Polyline" );
    addItem( RimExtrudedCurveIntersection::CrossSectionEnum::CS_AZIMUTHLINE, "CS_AZIMUTHLINE", "Azimuth and Dip" );
    setDefault( RimExtrudedCurveIntersection::CrossSectionEnum::CS_POLYLINE );
}

template <>
void caf::AppEnum<RimExtrudedCurveIntersection::CrossSectionDirEnum>::setUp()
{
    addItem( RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_VERTICAL, "CS_VERTICAL", "Vertical" );
    addItem( RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_HORIZONTAL, "CS_HORIZONTAL", "Horizontal" );
    addItem( RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_TWO_POINTS, "CS_TWO_POINTS", "Defined by Two Points" );
    setDefault( RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_VERTICAL );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimExtrudedCurveIntersection, "CurveIntersection", "CrossSection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RimExtrudedCurveIntersection::intersectionGeometryGenerator() const
{
    if ( m_crossSectionPartMgr.notNull() ) return m_crossSectionPartMgr->intersectionGeometryGenerator();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection::CrossSectionEnum RimExtrudedCurveIntersection::type() const
{
    return m_type();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection::CrossSectionDirEnum RimExtrudedCurveIntersection::direction() const
{
    return m_direction();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimExtrudedCurveIntersection::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RimExtrudedCurveIntersection::simulationWell() const
{
    return m_simulationWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimExtrudedCurveIntersection::inputPolyLineFromViewerEnabled() const
{
    return m_inputPolylineFromViewerEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimExtrudedCurveIntersection::inputExtrusionPointsFromViewerEnabled() const
{
    return m_inputExtrusionPointsFromViewerEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimExtrudedCurveIntersection::inputTwoAzimuthPointsFromViewerEnabled() const
{
    return m_inputTwoAzimuthPointsFromViewerEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::configureForSimulationWell( RimSimWellInView* simWell )
{
    m_type           = CrossSectionEnum::CS_SIMULATION_WELL;
    m_simulationWell = simWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::configureForWellPath( RimWellPath* wellPath )
{
    m_type     = CrossSectionEnum::CS_WELL_PATH;
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::configureForPolyLine()
{
    m_type                           = CrossSectionEnum::CS_POLYLINE;
    m_inputPolylineFromViewerEnabled = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::configureForAzimuthLine()
{
    m_type                                   = CrossSectionEnum::CS_AZIMUTHLINE;
    m_inputTwoAzimuthPointsFromViewerEnabled = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection::RimExtrudedCurveIntersection()
{
    CAF_PDM_InitScriptableObject( "Intersection", ":/CrossSection16x16.png" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_name, "UserDescription", "Name", QString( "Intersection Name" ), "Name" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_type, "Type", "Type" );
    CAF_PDM_InitFieldNoDefault( &m_direction, "Direction", "Direction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "Well Path        " );
    CAF_PDM_InitScriptableFieldNoDefault( &m_simulationWell, "SimulationWell", "Simulation Well" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_userPolylineXyz, "Points", "Points", "", "Use Ctrl-C for copy and Ctrl-V for paste", "" );

    CAF_PDM_InitFieldNoDefault( &m_userPolylineXydForUi, "PointsUi", "Points", "", "Use Ctrl-C for copy and Ctrl-V for paste", "" );
    m_userPolylineXydForUi.registerSetMethod( this, &RimExtrudedCurveIntersection::setPointsFromXYD );
    m_userPolylineXydForUi.registerGetMethod( this, &RimExtrudedCurveIntersection::pointsXYD );

    CAF_PDM_InitField( &m_azimuthAngle, "AzimuthAngle", 0.0, "Azimuth" );
    m_azimuthAngle.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_dipAngle, "DipAngle", 90.0, "Dip" );
    m_dipAngle.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_customExtrusionPoints, "CustomExtrusionPoints", "" );
    CAF_PDM_InitFieldNoDefault( &m_twoAzimuthPoints,
                                "TwoAzimuthPoints",
                                "Points",
                                "",
                                "Pick two points to define a line.\nUse Ctrl-C for copy and Ctrl-V for paste",
                                "" );

    CAF_PDM_InitField( &m_branchIndex, "Branch", -1, "Branch" );
    CAF_PDM_InitField( &m_extentLength, "ExtentLength", 200.0, "Extent Length" );
    CAF_PDM_InitField( &m_lengthUp, "lengthUp", 1000.0, "Length Up" );
    CAF_PDM_InitField( &m_lengthDown, "lengthDown", 1000.0, "Length Down" );

    CAF_PDM_InitFieldNoDefault( &m_inputPolylineFromViewerEnabled, "m_activateUiAppendPointsCommand", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_inputPolylineFromViewerEnabled );
    m_inputPolylineFromViewerEnabled = false;

    CAF_PDM_InitFieldNoDefault( &m_inputExtrusionPointsFromViewerEnabled, "inputExtrusionPointsFromViewerEnabled", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_inputExtrusionPointsFromViewerEnabled );
    m_inputExtrusionPointsFromViewerEnabled = false;

    CAF_PDM_InitFieldNoDefault( &m_inputTwoAzimuthPointsFromViewerEnabled, "inputTwoAzimuthPointsFromViewerEnabled", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_inputTwoAzimuthPointsFromViewerEnabled );
    m_inputTwoAzimuthPointsFromViewerEnabled = false;

    CAF_PDM_InitFieldNoDefault( &m_surfaceIntersections, "SurfaceIntersections", "Surface Intersections" );
    m_surfaceIntersections = new RimSurfaceIntersectionCollection;
    m_surfaceIntersections->objectChanged.connect( this, &RimExtrudedCurveIntersection::onSurfaceIntersectionsChanged );

    CAF_PDM_InitField( &m_depthUpperThreshold, "UpperThreshold", -300000.0, "Upper Threshold" );
    m_depthUpperThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_depthLowerThreshold, "LowerThreshold", 300000.0, "Lower Threshold" );
    m_depthLowerThreshold.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_depthFilterType, "DepthFilter", "Depth Filter" );

    CAF_PDM_InitFieldNoDefault( &m_collectionUpperThreshold, "CollectionUpperThreshold", "Collection Upper Threshold" );
    m_collectionUpperThreshold.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_collectionLowerThreshold, "CollectionLowerThreshold", "Collection Lower Threshold" );
    m_collectionLowerThreshold.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_depthThresholdOverridden, "ThresholdOverridden", false, "Depth Filter is Controlled by Intersection Collection" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_depthThresholdOverridden );

    CAF_PDM_InitFieldNoDefault( &m_collectionDepthFilterType, "CollectionDepthFilterType", "Collection Controlled Filter Type" );
    m_collectionDepthFilterType.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_enableKFilter, "EnableKFilter", false, "Enable K Range Filter" );
    CAF_PDM_InitFieldNoDefault( &m_kFilterText, "KRangeFilter", "K Range Filter", "", "Example: 2,4-6,10-30:2", "" );

    CAF_PDM_InitField( &m_kFilterCollectionOverride, "KFilterCollectionOverride", false, "K Range Filter is Controlled by Intersection Collection" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_kFilterCollectionOverride );

    CAF_PDM_InitFieldNoDefault( &m_kFilterCollectionText, "KRangeCollectionFilter", "Collection K Range Filter", "", "Example: 2,4-6,10-30:2", "" );
    m_kFilterCollectionText.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimExtrudedCurveIntersection::~RimExtrudedCurveIntersection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimExtrudedCurveIntersection::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimExtrudedCurveIntersection::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setName( const QString& newName )
{
    m_name = newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::upperFilterDepth( double upperGridLimit ) const
{
    if ( m_depthThresholdOverridden )
    {
        switch ( m_collectionDepthFilterType() )
        {
            case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
            case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
                return -m_collectionUpperThreshold;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
            case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
            default:
                return upperGridLimit;
        }
    }

    switch ( depthFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            return -m_depthUpperThreshold;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            return upperGridLimit;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionFilterEnum RimExtrudedCurveIntersection::depthFilterType() const
{
    if ( m_depthThresholdOverridden )
    {
        return m_collectionDepthFilterType();
    }
    return m_depthFilterType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::lowerFilterDepth( double lowerGridLimit ) const
{
    if ( m_depthThresholdOverridden )
    {
        switch ( m_collectionDepthFilterType() )
        {
            case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
            case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
                return -m_collectionLowerThreshold;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
            case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
            default:
                return lowerGridLimit;
        }
    }

    switch ( depthFilterType() )
    {
        case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
            return -m_depthLowerThreshold;

        case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
        case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
        default:
            return lowerGridLimit;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setDepthOverride( bool collectionOverride )
{
    m_depthThresholdOverridden = collectionOverride;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setDepthOverrideParameters( double upperThreshold, double lowerThreshold, RimIntersectionFilterEnum filterType )
{
    m_collectionUpperThreshold  = upperThreshold;
    m_collectionLowerThreshold  = lowerThreshold;
    m_collectionDepthFilterType = filterType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimExtrudedCurveIntersection::kLayerFilterEnabled() const
{
    return m_enableKFilter() || m_kFilterCollectionOverride();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimExtrudedCurveIntersection::kFilterText() const
{
    if ( m_kFilterCollectionOverride() ) return m_kFilterCollectionText();
    return m_kFilterText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setKFilterOverride( bool collectionOverride, QString kFilterText )
{
    m_kFilterCollectionOverride = collectionOverride;
    m_kFilterCollectionText     = kFilterText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isActive || changedField == &m_type || changedField == &m_direction || changedField == &m_wellPath ||
         changedField == &m_simulationWell || changedField == &m_branchIndex || changedField == &m_extentLength ||
         changedField == &m_lengthUp || changedField == &m_lengthDown || changedField == &m_showInactiveCells ||
         changedField == &m_useSeparateDataSource || changedField == &m_separateDataSource || changedField == &m_depthUpperThreshold ||
         changedField == &m_depthLowerThreshold || changedField == &m_depthThresholdOverridden || changedField == &m_depthFilterType ||
         changedField == &m_enableKFilter || changedField == &m_kFilterText || changedField == &m_kFilterCollectionOverride )
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if ( changedField == &m_simulationWell || changedField == &m_isActive || changedField == &m_type )
    {
        recomputeSimulationWellBranchData();
    }

    if ( changedField == &m_simulationWell || changedField == &m_wellPath || changedField == &m_branchIndex )
    {
        updateName();
    }

    if ( changedField == &m_name )
    {
        Rim2dIntersectionView* iView = correspondingIntersectionView();
        if ( iView )
        {
            iView->updateName();
            iView->updateConnectedEditors();
        }
    }

    if ( changedField == &m_inputPolylineFromViewerEnabled || changedField == &m_userPolylineXyz || changedField == &m_userPolylineXydForUi )
    {
        if ( m_inputPolylineFromViewerEnabled )
        {
            m_inputExtrusionPointsFromViewerEnabled  = false;
            m_inputTwoAzimuthPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if ( changedField == &m_inputExtrusionPointsFromViewerEnabled || changedField == &m_customExtrusionPoints )
    {
        if ( m_inputExtrusionPointsFromViewerEnabled )
        {
            m_inputPolylineFromViewerEnabled         = false;
            m_inputTwoAzimuthPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if ( changedField == &m_inputTwoAzimuthPointsFromViewerEnabled || changedField == &m_twoAzimuthPoints )
    {
        if ( m_inputTwoAzimuthPointsFromViewerEnabled )
        {
            m_inputPolylineFromViewerEnabled        = false;
            m_inputExtrusionPointsFromViewerEnabled = false;
        }

        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if ( changedField == &m_azimuthAngle )
    {
        updateAzimuthLine();
        rebuildGeometryAndScheduleCreateDisplayModel();
    }

    if ( changedField == &m_dipAngle )
    {
        rebuildGeometryAndScheduleCreateDisplayModel();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup( "Intersecting Geometry" );
    geometryGroup->add( &m_type );

    if ( m_type() == CrossSectionEnum::CS_WELL_PATH )
    {
        geometryGroup->add( &m_wellPath );
    }
    else if ( type() == CrossSectionEnum::CS_SIMULATION_WELL )
    {
        geometryGroup->add( &m_simulationWell );
        updateSimulationWellCenterline();
        if ( m_simulationWell() && m_simulationWellBranchCenterlines.size() > 1 )
        {
            geometryGroup->add( &m_branchIndex );
        }
    }
    else if ( type() == CrossSectionEnum::CS_POLYLINE )
    {
        geometryGroup->add( &m_userPolylineXydForUi );
        geometryGroup->add( &m_inputPolylineFromViewerEnabled );
    }
    else if ( type() == CrossSectionEnum::CS_AZIMUTHLINE )
    {
        geometryGroup->add( &m_twoAzimuthPoints );
        geometryGroup->add( &m_inputTwoAzimuthPointsFromViewerEnabled );
        geometryGroup->add( &m_azimuthAngle );
        geometryGroup->add( &m_dipAngle );
    }

    caf::PdmUiGroup* optionsGroup = uiOrdering.addNewGroup( "Options" );
    optionsGroup->setCollapsedByDefault();

    if ( type() == CrossSectionEnum::CS_AZIMUTHLINE )
    {
        optionsGroup->add( &m_lengthUp );
        optionsGroup->add( &m_lengthDown );
    }
    else
    {
        optionsGroup->add( &m_direction );
        optionsGroup->add( &m_extentLength );
    }

    if ( direction() == CrossSectionDirEnum::CS_TWO_POINTS )
    {
        optionsGroup->add( &m_customExtrusionPoints );
        optionsGroup->add( &m_inputExtrusionPointsFromViewerEnabled );
    }

    optionsGroup->add( &m_showInactiveCells );

    if ( type() == CrossSectionEnum::CS_POLYLINE )
    {
        m_extentLength.uiCapability()->setUiReadOnly( true );
    }
    else
    {
        m_extentLength.uiCapability()->setUiReadOnly( false );
    }

    auto filterGroup = uiOrdering.addNewGroup( "Depth Filter" );
    if ( m_depthThresholdOverridden() )
    {
        filterGroup->add( &m_depthThresholdOverridden );
    }
    else
    {
        filterGroup->add( &m_depthFilterType );

        switch ( m_depthFilterType() )
        {
            case RimIntersectionFilterEnum::INTERSECT_FILTER_BELOW:
                m_depthUpperThreshold.uiCapability()->setUiName( "Depth" );
                filterGroup->add( &m_depthUpperThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_BETWEEN:
                m_depthUpperThreshold.uiCapability()->setUiName( "Upper Depth" );
                filterGroup->add( &m_depthUpperThreshold );
                m_depthLowerThreshold.uiCapability()->setUiName( "Lower Depth" );
                filterGroup->add( &m_depthLowerThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_ABOVE:
                m_depthLowerThreshold.uiCapability()->setUiName( "Depth" );
                filterGroup->add( &m_depthLowerThreshold );
                break;

            case RimIntersectionFilterEnum::INTERSECT_FILTER_NONE:
            default:
                break;
        }

        if ( eclipseView() != nullptr )
        {
            auto kgroup = uiOrdering.addNewGroup( "K Range Filter" );

            if ( m_kFilterCollectionOverride() )
            {
                kgroup->add( &m_kFilterCollectionOverride );
            }
            else
            {
                kgroup->add( &m_enableKFilter );
                kgroup->add( &m_kFilterText );
            }
        }
    }

    this->defineSeparateDataSourceUi( uiConfigName, uiOrdering );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimExtrudedCurveIntersection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );

        if ( !options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_simulationWell )
    {
        RimSimWellInViewCollection* coll = simulationWellCollection();
        if ( coll )
        {
            caf::PdmChildArrayField<RimSimWellInView*>& simWells = coll->wells;

            caf::IconProvider simWellIcon( ":/Well.svg" );
            for ( RimSimWellInView* eclWell : simWells )
            {
                options.push_back( caf::PdmOptionItemInfo( eclWell->name(), eclWell, false, simWellIcon ) );
            }
        }

        if ( options.empty() )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        updateSimulationWellCenterline();

        size_t branchCount = m_simulationWellBranchCenterlines.size();

        options.push_back( caf::PdmOptionItemInfo( "All", -1 ) );

        for ( size_t bIdx = 0; bIdx < branchCount; ++bIdx )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( bIdx + 1 ), QVariant::fromValue( bIdx ) ) );
        }
    }
    else
    {
        options = RimIntersection::calculateValueOptions( fieldNeedingOptions );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( auto c : m_surfaceIntersections->surfaceIntersectionCurves() )
    {
        uiTreeOrdering.add( c );
    }
    for ( auto c : m_surfaceIntersections->surfaceIntersectionBands() )
    {
        uiTreeOrdering.add( c );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInViewCollection* RimExtrudedCurveIntersection::simulationWellCollection() const
{
    RimEclipseView* eclView = eclipseView();

    if ( eclView )
    {
        return eclView->wellCollection();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::updateAzimuthLine()
{
    if ( m_twoAzimuthPoints().size() == 2 )
    {
        double currentAzimuth = azimuthInRadians( m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0] );
        double newAzimuth     = cvf::Math::toRadians( m_azimuthAngle );
        double rotAngle       = newAzimuth - currentAzimuth;

        cvf::Mat4d rotMat             = cvf::Mat4d::fromRotation( -cvf::Vec3d::Z_AXIS, rotAngle );
        cvf::Mat4d transFromOriginMat = cvf::Mat4d::fromTranslation( m_twoAzimuthPoints()[0] );
        cvf::Mat4d transToOriginMat   = cvf::Mat4d::fromTranslation( -m_twoAzimuthPoints()[0] );

        m_twoAzimuthPoints.v()[1] = m_twoAzimuthPoints()[1].getTransformedPoint( transFromOriginMat * rotMat * transToOriginMat );

        m_twoAzimuthPoints.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<cvf::Vec3d>> RimExtrudedCurveIntersection::polyLines( cvf::Vec3d* flattenedPolylineStartPoint ) const
{
    if ( flattenedPolylineStartPoint ) *flattenedPolylineStartPoint = cvf::Vec3d::ZERO;

    std::vector<std::vector<cvf::Vec3d>> lines;

    double horizontalProjectedLengthAlongWellPathToClipPoint = 0.0;

    if ( type() == CrossSectionEnum::CS_WELL_PATH )
    {
        if ( m_wellPath() && wellPath()->wellPathGeometry() )
        {
            lines.push_back( wellPath()->wellPathGeometry()->wellPathPoints() );
            auto ownerCase = firstAncestorOrThisOfType<RimCase>();
            if ( ownerCase && ownerCase->activeCellsBoundingBox().isValid() )
            {
                size_t dummy;
                lines[0] = RigWellPath::clipPolylineStartAboveZ( lines[0],
                                                                 ownerCase->activeCellsBoundingBox().max().z(),
                                                                 &horizontalProjectedLengthAlongWellPathToClipPoint,
                                                                 &dummy );
            }
        }
    }
    else if ( type() == CrossSectionEnum::CS_SIMULATION_WELL )
    {
        if ( m_simulationWell() )
        {
            updateSimulationWellCenterline();

            int branchIndexToUse = branchIndex();

            if ( 0 <= branchIndexToUse && branchIndexToUse < static_cast<int>( m_simulationWellBranchCenterlines.size() ) )
            {
                lines.push_back( m_simulationWellBranchCenterlines[branchIndexToUse] );
            }

            if ( branchIndexToUse == -1 )
            {
                lines = m_simulationWellBranchCenterlines;
            }
        }
    }
    else if ( type() == CrossSectionEnum::CS_POLYLINE )
    {
        lines.push_back( m_userPolylineXyz );
    }
    else if ( type() == CrossSectionEnum::CS_AZIMUTHLINE )
    {
        lines.push_back( m_twoAzimuthPoints );
    }

    if ( type() == CrossSectionEnum::CS_WELL_PATH || type() == CrossSectionEnum::CS_SIMULATION_WELL )
    {
        if ( type() == CrossSectionEnum::CS_SIMULATION_WELL && m_simulationWell() )
        {
            cvf::Vec3d top, bottom;

            m_simulationWell->wellHeadTopBottomPosition( -1, &top, &bottom );

            for ( std::vector<cvf::Vec3d>& polyLine : lines )
            {
                polyLine.insert( polyLine.begin(), top );
            }
        }

        for ( std::vector<cvf::Vec3d>& polyLine : lines )
        {
            addExtents( polyLine );
        }

        if ( flattenedPolylineStartPoint && !lines.empty() && lines[0].size() > 1 )
        {
            ( *flattenedPolylineStartPoint )[0] = horizontalProjectedLengthAlongWellPathToClipPoint - m_extentLength;
            ( *flattenedPolylineStartPoint )[2] = lines[0][1].z(); // Depth of first point in first polyline
        }
    }
    else
    {
        if ( flattenedPolylineStartPoint && !lines.empty() && !( lines[0] ).empty() )
        {
            ( *flattenedPolylineStartPoint )[2] = lines[0][0].z(); // Depth of first point in first polyline
        }
    }
    return lines;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivExtrudedCurveIntersectionPartMgr* RimExtrudedCurveIntersection::intersectionPartMgr()
{
    if ( m_crossSectionPartMgr.isNull() ) m_crossSectionPartMgr = new RivExtrudedCurveIntersectionPartMgr( this );

    return m_crossSectionPartMgr.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::rebuildGeometry()
{
    m_crossSectionPartMgr = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimExtrudedCurveIntersection::polyLinesForExtrusionDirection() const
{
    return m_customExtrusionPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::updateSimulationWellCenterline() const
{
    if ( m_isActive() && type() == CrossSectionEnum::CS_SIMULATION_WELL && m_simulationWell() )
    {
        if ( m_simulationWellBranchCenterlines.empty() )
        {
            auto branches = m_simulationWell->wellPipeBranches();
            for ( const auto& branch : branches )
            {
                m_simulationWellBranchCenterlines.push_back( branch->wellPathPoints() );
            }
        }
    }
    else
    {
        m_simulationWellBranchCenterlines.clear();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::addExtents( std::vector<cvf::Vec3d>& polyLine ) const
{
    size_t lineVxCount = polyLine.size();

    if ( lineVxCount == 0 ) return;

    // Add extent at end of well
    {
        size_t     endIdxOffset = lineVxCount > 3 ? 3 : lineVxCount;
        cvf::Vec3d endDirection = ( polyLine[lineVxCount - 1] - polyLine[lineVxCount - endIdxOffset] );
        endDirection[2]         = 0; // Remove z. make extent length be horizontally
        if ( endDirection.length() < 1e-2 )
        {
            endDirection    = polyLine.back() - polyLine.front();
            endDirection[2] = 0;

            if ( endDirection.length() < 1e-2 )
            {
                endDirection = cvf::Vec3d::X_AXIS;
            }
        }

        endDirection.normalize();

        cvf::Vec3d newEnd = polyLine.back() + endDirection * m_extentLength();

        polyLine.push_back( newEnd );
    }

    // Add extent at start
    {
        size_t     endIdxOffset   = lineVxCount > 3 ? 3 : lineVxCount - 1;
        cvf::Vec3d startDirection = ( polyLine[0] - polyLine[endIdxOffset] );
        startDirection[2]         = 0; // Remove z. make extent length be horizontally
        if ( startDirection.length() < 1e-2 )
        {
            startDirection    = polyLine.front() - polyLine.back();
            startDirection[2] = 0;

            if ( startDirection.length() < 1e-2 )
            {
                startDirection = -cvf::Vec3d::X_AXIS;
            }
        }

        startDirection.normalize();

        cvf::Vec3d newStart = polyLine.front() + startDirection * m_extentLength();

        polyLine.insert( polyLine.begin(), newStart );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::updateName()
{
    if ( type() == CrossSectionEnum::CS_SIMULATION_WELL && m_simulationWell() )
    {
        m_name = m_simulationWell()->name();
        if ( branchIndex() != -1 )
        {
            m_name = m_name() + " Branch " + QString::number( branchIndex() + 1 );
        }
    }
    else if ( m_type() == CrossSectionEnum::CS_WELL_PATH && m_wellPath() )
    {
        m_name = m_wellPath()->name();
    }

    Rim2dIntersectionView* iView = correspondingIntersectionView();
    if ( iView )
    {
        iView->updateName();
        iView->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimExtrudedCurveIntersection::branchIndex() const
{
    RimSimWellInViewCollection* coll = simulationWellCollection();

    if ( coll && !coll->isAutoDetectingBranches() )
    {
        return -1;
    }

    if ( m_branchIndex >= static_cast<int>( m_simulationWellBranchCenterlines.size() ) )
    {
        return -1;
    }

    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setPushButtonText( bool buttonEnable, caf::PdmUiPushButtonEditorAttribute* attribute )
{
    if ( attribute )
    {
        if ( buttonEnable )
        {
            attribute->m_buttonText = "Stop picking points";
        }
        else
        {
            attribute->m_buttonText = "Start picking points";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setBaseColor( bool enable, caf::PdmUiListEditorAttribute* attribute )
{
    if ( attribute )
    {
        attribute->qssState = enable ? "ExternalInput" : QString();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute )
{
    auto* doubleSliderAttrib = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );
    if ( doubleSliderAttrib )
    {
        if ( field == &m_azimuthAngle )
        {
            doubleSliderAttrib->m_minimum         = 0;
            doubleSliderAttrib->m_maximum         = 360;
            doubleSliderAttrib->m_sliderTickCount = 360;
        }
        else if ( field == &m_dipAngle )
        {
            doubleSliderAttrib->m_minimum         = 0;
            doubleSliderAttrib->m_maximum         = 180;
            doubleSliderAttrib->m_sliderTickCount = 180;
        }
        else if ( ( field == &m_depthUpperThreshold ) || ( field == &m_depthLowerThreshold ) )
        {
            RimEclipseView* eclView = eclipseView();

            if ( eclView )
            {
                const cvf::BoundingBox bb = eclView->mainGrid()->boundingBox();

                doubleSliderAttrib->m_minimum = -1.0 * bb.max().z();
                doubleSliderAttrib->m_maximum = -1.0 * bb.min().z();
            }

            RimGeoMechView* geomView = firstAncestorOrThisOfType<RimGeoMechView>();

            if ( geomView )
            {
                const cvf::BoundingBox bb = geomView->domainBoundingBox();

                doubleSliderAttrib->m_minimum = -1.0 * bb.max().z();
                doubleSliderAttrib->m_maximum = -1.0 * bb.min().z();
            }
        }
    }
    else if ( field == &m_inputPolylineFromViewerEnabled )
    {
        setPushButtonText( m_inputPolylineFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) );
    }
    else if ( field == &m_userPolylineXydForUi )
    {
        setBaseColor( m_inputPolylineFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute ) );
    }
    else if ( field == &m_inputTwoAzimuthPointsFromViewerEnabled )
    {
        setPushButtonText( m_inputTwoAzimuthPointsFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) );
    }
    else if ( field == &m_twoAzimuthPoints )
    {
        setBaseColor( m_inputTwoAzimuthPointsFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute ) );
    }
    else if ( field == &m_inputExtrusionPointsFromViewerEnabled )
    {
        setPushButtonText( m_inputExtrusionPointsFromViewerEnabled, dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute ) );
    }
    else if ( field == &m_customExtrusionPoints )
    {
        setBaseColor( m_inputExtrusionPointsFromViewerEnabled, dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::appendPointToPolyLine( const cvf::Vec3d& pointXyz )
{
    m_userPolylineXyz.v().push_back( pointXyz );

    m_userPolylineXyz.uiCapability()->updateConnectedEditors();
    m_userPolylineXydForUi.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim2dIntersectionView* RimExtrudedCurveIntersection::correspondingIntersectionView() const
{
    std::vector<Rim2dIntersectionView*> objects = objectsWithReferringPtrFieldsOfType<Rim2dIntersectionView>();
    for ( auto isectView : objects )
    {
        if ( isectView )
        {
            return isectView;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::appendPointToExtrusionDirection( const cvf::Vec3d& point )
{
    if ( m_customExtrusionPoints().size() > 1 ) m_customExtrusionPoints.v().clear();

    m_customExtrusionPoints.v().push_back( point );

    m_customExtrusionPoints.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::appendPointToAzimuthLine( const cvf::Vec3d& point )
{
    if ( m_twoAzimuthPoints().empty() )
    {
        m_twoAzimuthPoints.v().push_back( point );
    }
    else if ( m_twoAzimuthPoints().size() == 1 )
    {
        cvf::Vec3d projectedPoint = cvf::Vec3d( point[0], point[1], m_twoAzimuthPoints.v()[0][2] );
        m_twoAzimuthPoints.v().push_back( projectedPoint );

        m_azimuthAngle = cvf::Math::toDegrees( azimuthInRadians( m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0] ) );
        m_azimuthAngle.uiCapability()->updateConnectedEditors();
    }
    else if ( m_twoAzimuthPoints().size() > 1 )
    {
        m_twoAzimuthPoints.v().clear();
        m_twoAzimuthPoints.v().push_back( point );
    }

    m_twoAzimuthPoints.uiCapability()->updateConnectedEditors();

    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimExtrudedCurveIntersection::extrusionDirection() const
{
    cvf::Vec3d dir = cvf::Vec3d::Z_AXIS;

    if ( m_direction() == RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_HORIZONTAL )
    {
        std::vector<std::vector<cvf::Vec3d>> lines = this->polyLines();
        if ( !lines.empty() && lines[0].size() > 1 )
        {
            std::vector<cvf::Vec3d> firstLine = lines[0];

            // Use first and last point of polyline to approximate orientation of polyline
            // Then cross with Z axis to find extrusion direction

            cvf::Vec3d polyLineDir = firstLine[firstLine.size() - 1] - firstLine[0];
            cvf::Vec3d up          = cvf::Vec3d::Z_AXIS;
            dir                    = polyLineDir ^ up;
        }
    }
    else if ( m_direction() == RimExtrudedCurveIntersection::CrossSectionDirEnum::CS_TWO_POINTS && m_customExtrusionPoints().size() > 1 )
    {
        dir = m_customExtrusionPoints()[m_customExtrusionPoints().size() - 1] - m_customExtrusionPoints()[0];
    }
    else if ( m_twoAzimuthPoints().size() == 2 )
    {
        double dipInRad = cvf::Math::toRadians( m_dipAngle );

        cvf::Vec3d azimutDirection = m_twoAzimuthPoints()[1] - m_twoAzimuthPoints()[0];

        cvf::Mat3d rotMat                           = cvf::Mat3d::fromRotation( azimutDirection, dipInRad );
        cvf::Vec3d vecPerpToRotVecInHorizontalPlane = azimutDirection ^ cvf::Vec3d::Z_AXIS;

        dir = vecPerpToRotVecInHorizontalPlane.getTransformedVector( rotMat );
    }

    return dir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::lengthUp() const
{
    return m_lengthUp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::lengthDown() const
{
    return m_lengthDown;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setLengthDown( double lengthDown )
{
    m_lengthDown = lengthDown;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::extentLength()
{
    return m_extentLength();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::recomputeSimulationWellBranchData()
{
    if ( m_type() == CrossSectionEnum::CS_SIMULATION_WELL )
    {
        m_simulationWellBranchCenterlines.clear();
        updateSimulationWellCenterline();

        m_crossSectionPartMgr = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimExtrudedCurveIntersection::hasDefiningPoints() const
{
    return m_type() == CrossSectionEnum::CS_POLYLINE || m_type() == CrossSectionEnum::CS_AZIMUTHLINE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionCurve*> RimExtrudedCurveIntersection::surfaceIntersectionCurves() const
{
    return m_surfaceIntersections->surfaceIntersectionCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionBand*> RimExtrudedCurveIntersection::surfaceIntersectionBands() const
{
    return m_surfaceIntersections->surfaceIntersectionBands();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCurve* RimExtrudedCurveIntersection::addIntersectionCurve()
{
    return m_surfaceIntersections->addIntersectionCurve();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionBand* RimExtrudedCurveIntersection::addIntersectionBand()
{
    return m_surfaceIntersections->addIntersectionBand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setLengthUp( double lengthUp )
{
    m_lengthUp = lengthUp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::rebuildGeometryAndScheduleCreateDisplayModel()
{
    m_crossSectionPartMgr = nullptr;

    auto rimView = firstAncestorOrThisOfType<Rim3dView>();
    if ( rimView )
    {
        rimView->scheduleCreateDisplayModelAndRedraw();
    }

    Rim2dIntersectionView* iview = correspondingIntersectionView();
    if ( iview )
    {
        iview->scheduleGeometryRegen( RivCellSetEnum::ALL_CELLS );
        iview->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimExtrudedCurveIntersection::azimuthInRadians( cvf::Vec3d vec )
{
    return cvf::GeometryTools::getAngle( -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS, vec );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::appendOptionItemsForSources( int                            currentLevel,
                                                                RimSurfaceCollection*          currentCollection,
                                                                QList<caf::PdmOptionItemInfo>& options ) const
{
    caf::IconProvider surfaceIcon( ":/ReservoirSurface16x16.png" );

    options.push_back( caf::PdmOptionItemInfo::createHeader( currentCollection->collectionName(), true ) );

    for ( auto surf : currentCollection->surfaces() )
    {
        auto itemInfo = caf::PdmOptionItemInfo( surf->userDescription(), surf, false, surfaceIcon );
        itemInfo.setLevel( currentLevel + 1 );
        options.push_back( itemInfo );
    }

    for ( auto subColl : currentCollection->subCollections() )
    {
        appendOptionItemsForSources( currentLevel, subColl, options );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::onSurfaceIntersectionsChanged( const caf::SignalEmitter* emitter )
{
    updateAllRequiredEditors();
    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimExtrudedCurveIntersection::pointsXYD() const
{
    return RiaVec3Tools::invertZSign( m_userPolylineXyz() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimExtrudedCurveIntersection::setPointsFromXYD( const std::vector<cvf::Vec3d>& pointsXYD )
{
    m_userPolylineXyz = RiaVec3Tools::invertZSign( pointsXYD );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimExtrudedCurveIntersection::eclipseView() const
{
    return firstAncestorOrThisOfType<RimEclipseView>();
}
