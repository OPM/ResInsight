/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimFishbones.h"

#include "RiaColorTables.h"
#include "RiaEclipseUnitTools.h"

#include "Well/RigFishbonesGeometry.h"
#include "Well/RigWellPath.h"

#include "RimFishbonesCollection.h"
#include "RimFishbonesPipeProperties.h"
#include "RimMultipleValveLocations.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathValve.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiListEditor.h"

#include "cvfAssert.h"
#include "cvfBoundingBox.h"
#include "cvfMath.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <random>

CAF_PDM_SOURCE_INIT( RimFishbones, "FishbonesMultipleSubs" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbones::RimFishbones()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "FishbonesMultipleSubs",
                                                    ":/FishBoneGroup16x16.png",
                                                    "",
                                                    "",
                                                    "Fishbones",
                                                    "Fishbones is a completion type used to add multiple short laterals to the main well "
                                                    "bore." );

    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );
    m_name.registerGetMethod( this, &RimFishbones::generatedName );
    m_name.uiCapability()->setUiReadOnly( true );
    m_name.xmlCapability()->setIOWritable( false );

    cvf::Color3f defaultColor = RiaColorTables::wellPathComponentColors()[RiaDefines::WellPathComponentType::FISHBONES];
    CAF_PDM_InitField( &fishbonesColor, "Color", defaultColor, "Fishbones Color" );

    CAF_PDM_InitScriptableField( &m_lateralCountPerSub, "LateralCountPerSub", 3, "Laterals Per Sub" );
    CAF_PDM_InitScriptableField( &m_lateralLength,
                                 "LateralLength",
                                 QString( "11.0" ),
                                 "Length(s) [m]",
                                 "",
                                 "Specify multiple length values if the sub lengths differ",
                                 "" );

    CAF_PDM_InitScriptableField( &m_lateralExitAngle, "LateralExitAngle", 35.0, "Exit Angle [deg]" );
    CAF_PDM_InitScriptableField( &m_lateralBuildAngle, "LateralBuildAngle", 6.0, "Build Angle [deg/m]" );

    CAF_PDM_InitScriptableField( &m_lateralTubingDiameter, "LateralTubingDiameter", 8.0, "Tubing Diameter [mm]" );

    CAF_PDM_InitScriptableField( &m_lateralOpenHoleRoghnessFactor, "LateralOpenHoleRoghnessFactor", 0.001, "Open Hole Roghness Factor [m]" );
    CAF_PDM_InitScriptableField( &m_lateralTubingRoghnessFactor, "LateralTubingRoghnessFactor", 1e-5, "Tubing Roghness Factor [m]" );

    CAF_PDM_InitScriptableField( &m_lateralInstallSuccessFraction, "LateralInstallSuccessFraction", 1.0, "Install Success Rate [0..1]" );

    CAF_PDM_InitScriptableField( &m_icdCount, "IcdCount", 2, "ICDs per Sub" );
    CAF_PDM_InitScriptableField( &m_icdOrificeDiameter, "IcdOrificeDiameter", 7.0, "ICD Orifice Diameter [mm]" );
    CAF_PDM_InitScriptableField( &m_icdFlowCoefficient, "IcdFlowCoefficient", 1.5, "ICD Flow Coefficient" );

    initialiseObsoleteFields();
    CAF_PDM_InitFieldNoDefault( &m_valveLocations, "ValveLocations", "Valve Locations" );
    m_valveLocations = new RimMultipleValveLocations();
    m_valveLocations->findField( "RangeValveCount" )->uiCapability()->setUiName( "Number of Subs" );
    m_valveLocations.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitScriptableField( &m_subsOrientationMode, "SubsOrientationMode", RimFishbonesDefines::LateralsOrientationType::RANDOM, "Orientation" );

    CAF_PDM_InitFieldNoDefault( &m_installationRotationAngles, "InstallationRotationAngles", "Installation Rotation Angles [deg]" );
    m_installationRotationAngles.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_installationRotationAngles.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_fixedInstallationRotationAngle, "FixedInstallationRotationAngle", 0.0, "  Fixed Angle [deg]" );

    auto pipeProperties = new RimFishbonesPipeProperties;

    // These proxy fields are created to allow the use from scripting API to work on the pipe properties directly on the fishbones object
    // Note that the set/get methods for the proxy fields are connected to the pipe properties object, which is a bit unusual
    CAF_PDM_InitScriptableFieldNoDefault( &m_lateralDiameter, "LateralDiameter", "Lateral Diameter" );
    m_lateralDiameter.registerSetMethod( pipeProperties, &RimFishbonesPipeProperties::setHoleDiameter );
    m_lateralDiameter.registerGetMethod( pipeProperties, &RimFishbonesPipeProperties::holeDiameter );

    CAF_PDM_InitScriptableFieldNoDefault( &m_lateralSkinFactor, "LateralSkinFactor", "Lateral Skin Factor" );
    m_lateralSkinFactor.registerSetMethod( pipeProperties, &RimFishbonesPipeProperties::setSkinFactor );
    m_lateralSkinFactor.registerGetMethod( pipeProperties, &RimFishbonesPipeProperties::skinFactor );

    CAF_PDM_InitFieldNoDefault( &m_pipeProperties, "PipeProperties", "Pipe Properties" );
    m_pipeProperties.uiCapability()->setUiTreeChildrenHidden( true );
    m_pipeProperties = pipeProperties;

    m_rigFishbonesGeometry = std::make_unique<RigFisbonesGeometry>( this );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbones::~RimFishbones()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFishbones::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFishbones::generatedName() const
{
    caf::PdmChildArrayField<RimFishbones*>* container = dynamic_cast<caf::PdmChildArrayField<RimFishbones*>*>( parentField() );
    CVF_ASSERT( container );

    size_t index = container->indexOf( this ) + 1;
    return QString( "Fishbone %1" ).arg( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::setMeasuredDepthAndCount( double startMD, double spacing, int subCount )
{
    double endMD = startMD + spacing * subCount;
    m_valveLocations->initFields( RimMultipleValveLocations::VALVE_SPACING, startMD, endMD, spacing, subCount, {} );

    computeRangesAndLocations();
    computeRotationAngles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::setValveLocations( const std::vector<double>& measuredDepths )
{
    m_valveLocations->setLocationType( RimMultipleValveLocations::VALVE_CUSTOM );
    m_valveLocations->setValveLocations( measuredDepths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::setSystemParameters( int lateralsPerSub, double lateralLength, double holeDiameter, double buildAngle, int icdsPerSub )
{
    m_lateralCountPerSub = lateralsPerSub;
    m_lateralLength      = QString::number( lateralLength );
    m_pipeProperties->setHoleDiameter( holeDiameter );
    m_lateralBuildAngle = buildAngle;
    m_icdCount          = icdsPerSub;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::measuredDepth( size_t subIndex ) const
{
    return m_valveLocations->measuredDepth( subIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::rotationAngle( size_t index ) const
{
    if ( m_subsOrientationMode == RimFishbonesDefines::LateralsOrientationType::FIXED )
    {
        return m_fixedInstallationRotationAngle;
    }
    else
    {
        CVF_ASSERT( index < m_installationRotationAngles().size() );

        return m_installationRotationAngles()[index];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::exitAngle() const
{
    return m_lateralExitAngle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::buildAngle() const
{
    return m_lateralBuildAngle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::tubingDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            return RiaEclipseUnitTools::inchToMeter( m_lateralTubingDiameter() );
        }
        else
        {
            return m_lateralTubingDiameter() / 1000;
        }
    }
    else if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            return RiaEclipseUnitTools::meterToFeet( m_lateralTubingDiameter() / 1000 );
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet( m_lateralTubingDiameter() );
        }
    }
    CVF_ASSERT( false );
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::holeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    return m_pipeProperties()->holeDiameter( unitSystem );
}

//--------------------------------------------------------------------------------------------------
/// Compute the equivalent diameter based on the area between two cylinders
//
// http://www.fekete.com/san/webhelp/feketeharmony/harmony_webhelp/content/html_files/reference_material/calculations_and_correlations/annular_diameters.htm
//--------------------------------------------------------------------------------------------------
double RimFishbones::equivalentDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    double innerRadius = tubingDiameter( unitSystem ) / 2;
    double outerRadius = holeDiameter( unitSystem ) / 2;

    double innerArea = cvf::PI_D * innerRadius * innerRadius;
    double outerArea = cvf::PI_D * outerRadius * outerRadius;

    double equivalentArea = outerArea - innerArea;

    double effectiveRadius = cvf::Math::sqrt( equivalentArea / cvf::PI_D );
    return effectiveRadius * 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::skinFactor() const
{
    return m_pipeProperties()->skinFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::openHoleRoughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( m_lateralOpenHoleRoghnessFactor() );
    }
    else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC && unitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( m_lateralOpenHoleRoghnessFactor() );
    }
    return m_lateralOpenHoleRoghnessFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::icdOrificeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();
    return RimWellPathValve::convertOrificeDiameter( m_icdOrificeDiameter(), wellPath->unitSystem(), unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::icdFlowCoefficient() const
{
    return m_icdFlowCoefficient();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimFishbones::icdCount() const
{
    return m_icdCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimFishbones::lateralLengths() const
{
    QStringList items         = m_lateralLength().split( ' ' );
    double      currentLength = 0.0;

    std::vector<double> lengths;
    for ( int i = 0; i < static_cast<int>( m_lateralCountPerSub ); i++ )
    {
        if ( i < items.size() )
        {
            bool   conversionOk   = false;
            double candidateValue = items[i].toDouble( &conversionOk );
            if ( conversionOk )
            {
                currentLength = candidateValue;
            }
        }

        lengths.push_back( currentLength );
    }

    return lengths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::geometryUpdated()
{
    computeRotationAngles();
    computeSubLateralIndices();

    RimProject* proj = RimProject::current();
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RimFishbones::SubAndLateralIndex>& RimFishbones::installedLateralIndices() const
{
    return m_subLateralIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimFishbones::coordsForLateral( size_t subIndex, size_t lateralIndex ) const
{
    std::vector<std::pair<cvf::Vec3d, double>> coordsAndMD = m_rigFishbonesGeometry->coordsForLateral( subIndex, lateralIndex );

    std::vector<cvf::Vec3d> domainCoords;
    for ( const auto& coordMD : coordsAndMD )
    {
        domainCoords.push_back( coordMD.first );
    }

    return domainCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<cvf::Vec3d, double>> RimFishbones::coordsAndMDForLateral( size_t subIndex, size_t lateralIndex ) const
{
    return m_rigFishbonesGeometry->coordsForLateral( subIndex, lateralIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::recomputeLateralLocations()
{
    computeRangesAndLocations();
    computeRotationAngles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::setUnitSystemSpecificDefaults()
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_lateralLength                 = "11";
            m_lateralBuildAngle             = 6.0;
            m_lateralTubingDiameter         = 8;
            m_lateralOpenHoleRoghnessFactor = 0.001;
            m_lateralTubingRoghnessFactor   = 1e-05;
            m_icdOrificeDiameter            = 7;
        }
        else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_lateralLength                 = "36";
            m_lateralBuildAngle             = 1.83;
            m_lateralTubingDiameter         = 0.31;
            m_lateralOpenHoleRoghnessFactor = 0.0032;
            m_lateralTubingRoghnessFactor   = 3.28e-05;
            m_icdOrificeDiameter            = 0.28;
        }
        m_pipeProperties->setUnitSystemSpecificDefaults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimFishbones::componentType() const
{
    return RiaDefines::WellPathComponentType::FISHBONES;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFishbones::componentLabel() const
{
    return generatedName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFishbones::componentTypeLabel() const
{
    return "Fishbones";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFishbones::defaultComponentColor() const
{
    return fishbonesColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::startMD() const
{
    double measuredDepth = 0.0;
    if ( !m_valveLocations->valveLocations().empty() )
    {
        measuredDepth = m_valveLocations->valveLocations().front();
    }

    return measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFishbones::endMD() const
{
    double measuredDepth = 0.0;
    if ( !m_valveLocations->valveLocations().empty() )
    {
        measuredDepth = m_valveLocations->valveLocations().back();
    }

    return measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::applyOffset( double offsetMD )
{
    m_valveLocations->applyOffset( offsetMD );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_subsOrientationMode )
    {
        computeRotationAngles();
    }

    if ( changedField == &m_lateralInstallSuccessFraction || changedField == &m_lateralCountPerSub )
    {
        computeSubLateralIndices();
    }

    geometryUpdated();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFishbones::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFishbones::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::computeRangesAndLocations()
{
    m_valveLocations->computeRangesAndLocations();
    geometryUpdated();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
        if ( wellPath )
        {
            if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
            {
                m_lateralLength.uiCapability()->setUiName( "Length(s) [m]" );
                m_lateralBuildAngle.uiCapability()->setUiName( "Build Angle [deg/m]" );
                m_lateralTubingDiameter.uiCapability()->setUiName( "Tubing Diameter [mm]" );
                m_lateralOpenHoleRoghnessFactor.uiCapability()->setUiName( "Open Hole Roughness Factor [m]" );
                m_lateralTubingRoghnessFactor.uiCapability()->setUiName( "Tubing Roughness Factor [m]" );

                m_lateralDiameter.uiCapability()->setUiName( "Lateral Diameter [mm]" );

                m_icdOrificeDiameter.uiCapability()->setUiName( "ICD Orifice Diameter [mm]" );
            }
            else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
            {
                m_lateralLength.uiCapability()->setUiName( "Length(s) [ft]" );
                m_lateralBuildAngle.uiCapability()->setUiName( "Build Angle [deg/ft]" );
                m_lateralTubingDiameter.uiCapability()->setUiName( "Tubing Diameter [in]" );
                m_lateralOpenHoleRoghnessFactor.uiCapability()->setUiName( "Open Hole Roughness Factor [ft]" );
                m_lateralTubingRoghnessFactor.uiCapability()->setUiName( "Tubing Roughness Factor [ft]" );

                m_lateralDiameter.uiCapability()->setUiName( "Lateral Diameter [in]" );

                m_icdOrificeDiameter.uiCapability()->setUiName( "ICD Orifice Diameter [in]" );
            }
        }
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Appearance" );

        group->add( &fishbonesColor );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Location" );
        m_valveLocations->uiOrdering( uiConfigName, *group );
    }

    {
        caf::PdmUiGroup* lateralConfigGroup = uiOrdering.addNewGroup( "Lateral Configuration" );

        lateralConfigGroup->add( &m_lateralCountPerSub );
        lateralConfigGroup->add( &m_lateralLength );

        lateralConfigGroup->add( &m_lateralExitAngle );
        lateralConfigGroup->add( &m_lateralBuildAngle );

        lateralConfigGroup->add( &m_subsOrientationMode );
        if ( m_subsOrientationMode == RimFishbonesDefines::LateralsOrientationType::FIXED )
        {
            lateralConfigGroup->add( &m_fixedInstallationRotationAngle );
        }

        lateralConfigGroup->add( &m_lateralInstallSuccessFraction );
    }

    {
        caf::PdmUiGroup* wellGroup = uiOrdering.addNewGroup( "Lateral Properties" );

        wellGroup->add( &m_lateralDiameter );
        wellGroup->add( &m_lateralSkinFactor );
    }

    {
        caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup( "Lateral Multi Segment Wells" );
        mswGroup->setCollapsedByDefault();
        mswGroup->add( &m_lateralTubingDiameter );
        mswGroup->add( &m_lateralOpenHoleRoghnessFactor );
        mswGroup->add( &m_lateralTubingRoghnessFactor );
        mswGroup->add( &m_icdCount );
        mswGroup->add( &m_icdOrificeDiameter );
        mswGroup->add( &m_icdFlowCoefficient );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::initAfterRead()
{
    initValveLocationFromLegacyData();

    if ( m_valveLocations->valveLocations().size() != m_installationRotationAngles().size() )
    {
        computeRotationAngles();
    }
    computeSubLateralIndices();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimFishbones::boundingBoxInDomainCoords() const
{
    cvf::BoundingBox bb;

    for ( const auto& [subIndex, lateralIndex] : installedLateralIndices() )
    {
        std::vector<cvf::Vec3d> coords = coordsForLateral( subIndex, lateralIndex );

        for ( const auto& c : coords )
        {
            bb.add( c );
        }
    }

    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFishbones::isEnabled() const
{
    auto collection = firstAncestorOrThisOfTypeAsserted<RimFishbonesCollection>();

    return collection->isChecked() && isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::computeRotationAngles()
{
    unsigned                           seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::minstd_rand0                  generator( seed );
    std::uniform_int_distribution<int> distribution( 0, 360 );

    std::vector<double> vals;
    for ( size_t i = 0; i < m_valveLocations->valveLocations().size(); i++ )
    {
        vals.push_back( distribution( generator ) );
    }

    m_installationRotationAngles = vals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::computeSubLateralIndices()
{
    std::vector<SubAndLateralIndex> subLateralCandidates;
    for ( size_t subIndex = 0; subIndex < m_valveLocations->valveLocations().size(); ++subIndex )
    {
        for ( int lateralIndex = 0; lateralIndex < m_lateralCountPerSub(); ++lateralIndex )
        {
            subLateralCandidates.push_back( std::make_pair( subIndex, lateralIndex ) );
        }
    }

    m_subLateralIndices = subLateralCandidates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::initialiseObsoleteFields()
{
    CAF_PDM_InitField( &m_subsLocationMode_OBSOLETE,
                       "SubsLocationMode",
                       caf::AppEnum<RimFishbonesDefines::LocationType_OBSOLETE>( RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_UNDEFINED ),
                       "Location Defined By" );
    m_subsLocationMode_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_rangeStart_OBSOLETE, "RangeStart", std::numeric_limits<double>::infinity(), "Start MD [m]" );
    m_rangeStart_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_rangeEnd_OBSOLETE, "RangeEnd", std::numeric_limits<double>::infinity(), "End MD [m]" );
    m_rangeEnd_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_rangeSubSpacing_OBSOLETE, "RangeSubSpacing", std::numeric_limits<double>::infinity(), "Spacing [m]" );
    m_rangeSubSpacing_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_rangeSubCount_OBSOLETE, "RangeSubCount", -1, "Number of Subs" );
    m_rangeSubCount_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_locationOfSubs_OBSOLETE, "LocationOfSubs", "Measured Depths [m]" );
    m_locationOfSubs_OBSOLETE.xmlCapability()->setIOWritable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFishbones::initValveLocationFromLegacyData()
{
    RimMultipleValveLocations::LocationType locationType = RimMultipleValveLocations::VALVE_UNDEFINED;
    if ( m_subsLocationMode_OBSOLETE() == RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_COUNT_END )
    {
        locationType = RimMultipleValveLocations::VALVE_COUNT;
    }
    else if ( m_subsLocationMode_OBSOLETE() == RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_SPACING_END )
    {
        locationType = RimMultipleValveLocations::VALVE_SPACING;
    }
    else if ( m_subsLocationMode_OBSOLETE() == RimFishbonesDefines::LocationType_OBSOLETE::FB_SUB_USER_DEFINED )
    {
        locationType = RimMultipleValveLocations::VALVE_CUSTOM;
    }

    m_valveLocations->initFields( locationType,
                                  m_rangeStart_OBSOLETE(),
                                  m_rangeEnd_OBSOLETE(),
                                  m_rangeSubSpacing_OBSOLETE(),
                                  m_rangeSubCount_OBSOLETE(),
                                  m_locationOfSubs_OBSOLETE() );
}
