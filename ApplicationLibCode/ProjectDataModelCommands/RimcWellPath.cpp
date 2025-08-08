/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimcWellPath.h"

#include "FractureCommands/RicNewWellPathFractureFeature.h"
#include "FractureCommands/RicPlaceThermalFractureUsingTemplateDataFeature.h"

#include "RiaApplication.h"
#include "RiaKeyValueStoreUtil.h"
#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseTools.h"
#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesDefines.h"
#include "RimMswCompletionParameters.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimStimPlanModel.h"
#include "RimThermalFractureTemplate.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathFracture.h"

#include "RigDoglegTools.h"
#include "RigStimPlanModelTools.h"
#include "Well/RigWellPathGeometryExporter.h"
#include "Well/RigWellPathGeometryTools.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <numbers>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_addFracture, "AddFracture" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_addFracture::RimcWellPath_addFracture( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add StimPlan Fracture", "", "", "Add StimPlan Fracture" );

    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_stimPlanFractureTemplate, "StimPlanFractureTemplate", "", "", "", "StimPlan Fracture Template" );
    CAF_PDM_InitScriptableField( &m_alignDip, "AlignDip", false, "Align Dip" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "", "", "", "Eclipse Case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_addFracture::execute()
{
    auto wellPath = self<RimWellPath>();

    RimWellPathFracture* wellPathFracture = RicNewWellPathFractureFeature::addFracture( wellPath, m_md() );

    if ( m_stimPlanFractureTemplate ) wellPathFracture->setFractureTemplate( m_stimPlanFractureTemplate() );

    if ( m_alignDip )
    {
        if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() )
        {
            RiaLogging::info( "Computing formation dip for fracture alignment" );

            double boundingBoxHorizontal = 50.0;
            double boundingBoxVertical   = 100.0;

            cvf::Vec3d position  = wellPathFracture->anchorPosition();
            cvf::Vec3d direction = RigStimPlanModelTools::calculateTSTDirection( m_eclipseCase->eclipseCaseData(),
                                                                                 position,
                                                                                 boundingBoxHorizontal,
                                                                                 boundingBoxVertical );
            RiaLogging::info( QString( "Direction: %1 %2 %3" ).arg( direction.x() ).arg( direction.y() ).arg( direction.z() ) );
            cvf::Vec3d fractureDirectionNormal = wellPathFracture->computeFractureDirectionNormal();

            cvf::Vec3d formationDirection = RimStimPlanModel::projectVectorIntoFracturePlane( position, fractureDirectionNormal, direction );
            if ( !formationDirection.isUndefined() )
            {
                double formationDip = RigStimPlanModelTools::calculateFormationDip( formationDirection ) - 90.0;
                RiaLogging::info( QString( "Computed formation dip: %1" ).arg( formationDip ) );

                wellPathFracture->setDip( formationDip );
            }
        }
        else
        {
            RiaLogging::error( "No eclipse case found. Fracture not aligned with formation dip." );
        }
    }

    return wellPathFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPath_addFracture::classKeywordReturnedType() const
{
    return RimWellPathFracture::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_addThermalFracture, "AddThermalFracture" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_addThermalFracture::RimcWellPath_addThermalFracture( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Thermal Fracture", "", "", "Add Thermal Fracture" );

    CAF_PDM_InitScriptableField( &m_md, "MeasuredDepth", 0.0, "Measured Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fractureTemplate, "FractureTemplate", "", "", "", "Thermal Fracture Template" );
    CAF_PDM_InitScriptableField( &m_placeUsingTemplateData, "PlaceUsingTemplateData", true, "Place using template data" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_addThermalFracture::execute()
{
    auto wellPath = self<RimWellPath>();

    RimWellPathFracture* wellPathFracture = RicNewWellPathFractureFeature::addFracture( wellPath, m_md() );

    if ( m_fractureTemplate )
    {
        wellPathFracture->setFractureTemplate( m_fractureTemplate() );
    }

    if ( m_placeUsingTemplateData )
    {
        RicPlaceThermalFractureUsingTemplateDataFeature::placeUsingTemplateData( wellPathFracture );
    }

    return wellPathFracture;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPath_addThermalFracture::classKeywordReturnedType() const
{
    return RimWellPathFracture::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_appendPerforationInterval, "AppendPerforationInterval" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_appendPerforationInterval::RimcWellPath_appendPerforationInterval( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Append Perforation Interval", "", "", "Append Perforation Interval" );

    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "", "", "", "Start Measured Depth" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "", "", "", "End Measured Depth" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.0, "", "", "", "Diameter" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "SkinFactor", 0.0, "", "", "", "Skin Factor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_appendPerforationInterval::execute()
{
    auto wellPath = self<RimWellPath>();

    auto perforationInterval = new RimPerforationInterval;
    perforationInterval->setStartAndEndMD( m_startMD, m_endMD );
    perforationInterval->setSkinFactor( m_skinFactor );
    perforationInterval->setDiameter( m_diameter );

    wellPath->perforationIntervalCollection()->appendPerforation( perforationInterval );

    auto* wellPathCollection = RimTools::wellPathCollection();

    wellPathCollection->uiCapability()->updateConnectedEditors();
    wellPathCollection->scheduleRedrawAffectedViews();

    return perforationInterval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPath_appendPerforationInterval::classKeywordReturnedType() const
{
    return RimPerforationInterval::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_multiSegmentWellSettings, "MswSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_multiSegmentWellSettings::RimcWellPath_multiSegmentWellSettings( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "MSW Settings", "", "", "Multi Segment Well Settings" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_multiSegmentWellSettings::execute()
{
    auto wellPath = self<RimWellPath>();

    // RimMswCompletionParameters is a child object of RimWellPathCompletionSettings. To simplify the Python API, we return
    // RimMswCompletionParameters directly from a well path object in Python. Two parameters are already exposed as part of the completion
    // settings object, see RimWellPathCompletionSettings and the proxy fields liner_diameter and roughness. These fields are kept to
    // ensure backward compatibility with existing scripts.
    //
    // https://github.com/OPM/ResInsight/issues/11901

    if ( auto completionSettings = wellPath->completionSettings() )
    {
        return completionSettings->mswCompletionParameters();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPath_multiSegmentWellSettings::classKeywordReturnedType() const
{
    return RimMswCompletionParameters::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_appendFishbones, "AppendFishbones" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_appendFishbones::RimcWellPath_appendFishbones( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_VALID, PdmObjectMethod::ResultType::PERSISTENT_TRUE )
{
    CAF_PDM_InitObject( "Append Fishbones", "", "", "Append Fishbones" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_subLocations, "SubLocations", "SubLocations" );
    auto defaultDrillingType = RimFishbonesDefines::DrillingType::STANDARD;
    CAF_PDM_InitScriptableField( &m_drillingType, "DrillingType", defaultDrillingType, "DrillingType" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_appendFishbones::execute()
{
    auto wellPath = self<RimWellPath>();

    if ( m_subLocations().empty() )
    {
        RiaLogging::error(
            "Sub locations are empty, expected list of float values defining measured depths. Cannot create fishbones object." );
        return nullptr;
    }

    if ( auto fishbonesCollection = wellPath->fishbonesCollection() )
    {
        auto* fishbonesObject = fishbonesCollection->appendFishbonesSubsAtLocations( m_subLocations(), m_drillingType() );

        wellPath->updateAllRequiredEditors();

        return fishbonesObject;
    }

    RiaLogging::error( "No fishbones collection object found, cannot create fishbones object." );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellPath_appendFishbones::classKeywordReturnedType() const
{
    return RimFishbones::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellPath, RimcWellPath_extractWellPathPropertiesInternal, "ExtractWellPathPropertiesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellPath_extractWellPathPropertiesInternal::RimcWellPath_extractWellPathPropertiesInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Extract Well Path Properties", "", "", "Extract Well Path Properties" );

    CAF_PDM_InitScriptableField( &m_resamplingInterval, "ResamplingInterval", 10.0, "ResamplingInterval" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateX, "CoordinateX", "CoordinateX" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateY, "CoordinateY", "CoordinateY" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinateZ, "CoordinateZ", "CoordinateZ" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_measuredDepth, "MeasuredDepth", "MeasuredDepth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_azimuth, "Azimuth", "Azimuth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_inclination, "Inclination", "Inclination" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_dogleg, "Dogleg", "Dogleg" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellPath_extractWellPathPropertiesInternal::execute()
{
    auto wellPath = self<RimWellPath>();

    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> tvdValues;
    std::vector<double> mdValues;

    auto wellPathGeom = wellPath->wellPathGeometry();

    double mdStepSize = m_resamplingInterval();
    CAF_ASSERT( mdStepSize > 0.0 );
    double rkbOffset = 0.0;
    RigWellPathGeometryExporter::computeWellPathDataForExport( *wellPathGeom, mdStepSize, rkbOffset, xValues, yValues, tvdValues, mdValues );

    auto convertToCharViaFloat = []( const std::vector<double>& doubles ) -> std::vector<char>
    {
        std::vector<float> f;
        for ( const double& v : doubles )
            f.push_back( static_cast<float>( v ) );
        return RiaKeyValueStoreUtil::convertToByteVector( f );
    };

    RiaApplication::instance()->keyValueStore()->set( m_coordinateX().toStdString(), convertToCharViaFloat( xValues ) );
    RiaApplication::instance()->keyValueStore()->set( m_coordinateY().toStdString(), convertToCharViaFloat( yValues ) );
    RiaApplication::instance()->keyValueStore()->set( m_coordinateZ().toStdString(), convertToCharViaFloat( tvdValues ) );
    RiaApplication::instance()->keyValueStore()->set( m_measuredDepth().toStdString(), convertToCharViaFloat( mdValues ) );

    std::vector<double> azimuthValues;
    std::vector<double> inclinationValues;
    for ( const double& md : mdValues )
    {
        auto [azimuth, inclination] = RigWellPathGeometryTools::calculateAzimuthAndInclinationAtMd( md, wellPathGeom );

        if ( azimuth < 0.0 )
        {
            // Straight atan2 gives angle from -PI to PI yielding angles from -180 to 180
            // where the negative angles are counter clockwise.
            // To get all positive clockwise angles, we add 360 degrees to negative angles.
            azimuth = azimuth + std::numbers::pi * 2.0;
        }

        azimuthValues.push_back( cvf::Math::toDegrees( azimuth ) );
        inclinationValues.push_back( cvf::Math::toDegrees( inclination ) );
    }

    RiaApplication::instance()->keyValueStore()->set( m_azimuth().toStdString(), convertToCharViaFloat( azimuthValues ) );
    RiaApplication::instance()->keyValueStore()->set( m_inclination().toStdString(), convertToCharViaFloat( inclinationValues ) );

    std::vector<RigDoglegTools::WellSurveyPoint> surveyPoints;
    for ( size_t i = 0; i < xValues.size(); i++ )
    {
        surveyPoints.push_back( { .inclination = inclinationValues[i], .azimuth = azimuthValues[i], .measuredDepth = mdValues[i] } );
    }

    // TODO: normalization length is meter. Maybe make it settable?
    std::vector<double> doglegs = RigDoglegTools::calculateTrajectoryDogleg( surveyPoints, 30 );
    RiaApplication::instance()->keyValueStore()->set( m_dogleg().toStdString(), convertToCharViaFloat( doglegs ) );

    return nullptr;
}
