/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswCompletions.h"

#include "RicMswSegmentCellIntersection.h"

#include "RimWellPathValve.h"
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswCompletion::RicMswCompletion( const QString&     label,
                                    const RimWellPath* wellPath,
                                    double             startMD,
                                    double             startTVD,
                                    size_t             index /* = cvf::UNDEFINED_SIZE_T */ )
    : RicMswBranch( label, wellPath, startMD, startTVD )
    , m_index( index )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswCompletion::index() const
{
    return m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswFracture::RicMswFracture( const QString&     label,
                                const RimWellPath* wellPath,
                                double             startMD,
                                double             startTVD,
                                size_t             index /*= cvf::UNDEFINED_SIZE_T*/ )
    : RicMswCompletion( label, wellPath, startMD, startTVD, index )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswFracture::completionType() const
{
    return RigCompletionData::FRACTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforation::RicMswPerforation( const QString&     label,
                                      const RimWellPath* wellPath,
                                      double             startMD,
                                      double             startTVD,
                                      size_t             index /*= cvf::UNDEFINED_SIZE_T*/ )
    : RicMswCompletion( label, wellPath, startMD, startTVD, index )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforation::completionType() const
{
    return RigCompletionData::PERFORATION;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswValve::RicMswValve( const QString&          label,
                          const RimWellPath*      wellPath,
                          double                  startMD,
                          double                  startTVD,
                          const RimWellPathValve* wellPathValve )
    : RicMswCompletion( label, wellPath, startMD, startTVD )
    , m_wellPathValve( wellPathValve )
    , m_valid( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathValve* RicMswValve::wellPathValve() const
{
    return m_wellPathValve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswValve::isValid() const
{
    return m_valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswValve::setIsValid( bool valid )
{
    m_valid = valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RicMswValve> RicMswValve::createExportValve( const QString&          label,
                                                             const RimWellPath*      wellPath,
                                                             double                  startMD,
                                                             double                  startTVD,
                                                             const RimWellPathValve* wellPathValve )
{
    std::unique_ptr<RicMswValve> outletValve;
    if ( wellPathValve->componentType() == RiaDefines::WellPathComponentType::ICD )
    {
        outletValve = std::make_unique<RicMswPerforationICD>( label, wellPath, startMD, startTVD, wellPathValve );
    }
    else if ( wellPathValve->componentType() == RiaDefines::WellPathComponentType::ICV )
    {
        outletValve = std::make_unique<RicMswPerforationICV>( label, wellPath, startMD, startTVD, wellPathValve );
    }
    else if ( wellPathValve->componentType() == RiaDefines::WellPathComponentType::AICD )
    {
        outletValve = std::make_unique<RicMswPerforationAICD>( label, wellPath, startMD, startTVD, wellPathValve );
    }
    else
    {
        CAF_ASSERT( false && "Valve needs to be either an ICD, ICVF or AICD" );
    }
    return outletValve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswWsegValve::RicMswWsegValve( const QString&          label,
                                  const RimWellPath*      wellPath,
                                  double                  startMD,
                                  double                  startTVD,
                                  const RimWellPathValve* wellPathValve )
    : RicMswValve( label, wellPath, startMD, startTVD, wellPathValve )
    , m_flowCoefficient( 0.0 )
    , m_area( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswWsegValve::flowCoefficient() const
{
    return m_flowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswWsegValve::area() const
{
    return m_area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswWsegValve::setFlowCoefficient( double icdFlowCoefficient )
{
    m_flowCoefficient = icdFlowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswWsegValve::setArea( double icdArea )
{
    m_area = icdArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswFishbonesICD::RicMswFishbonesICD( const QString&          label,
                                        const RimWellPath*      wellPath,
                                        double                  startMD,
                                        double                  startTVD,
                                        const RimWellPathValve* wellPathValve )
    : RicMswWsegValve( label, wellPath, startMD, startTVD, wellPathValve )
{
    setIsValid( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswFishbonesICD::completionType() const
{
    return RigCompletionData::FISHBONES_ICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforationICD::RicMswPerforationICD( const QString&          label,
                                            const RimWellPath*      wellPath,
                                            double                  startMD,
                                            double                  startTVD,
                                            const RimWellPathValve* wellPathValve )
    : RicMswWsegValve( label, wellPath, startMD, startTVD, wellPathValve )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforationICD::completionType() const
{
    return RigCompletionData::PERFORATION_ICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforationICV::RicMswPerforationICV( const QString&          label,
                                            const RimWellPath*      wellPath,
                                            double                  startMD,
                                            double                  startTVD,
                                            const RimWellPathValve* wellPathValve )
    : RicMswWsegValve( label, wellPath, startMD, startTVD, wellPathValve )
{
    setIsValid( true );
}

//-------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforationICV::completionType() const
{
    return RigCompletionData::PERFORATION_ICV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforationAICD::RicMswPerforationAICD( const QString&          label,
                                              const RimWellPath*      wellPath,
                                              double                  startMD,
                                              double                  startTVD,
                                              const RimWellPathValve* wellPathValve )
    : RicMswValve( label, wellPath, startMD, startTVD, wellPathValve )
    , m_deviceOpen( false )
    , m_length( 0.0 )
    , m_flowScalingFactor( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforationAICD::completionType() const
{
    return RigCompletionData::PERFORATION_AICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswPerforationAICD::isOpen() const
{
    return m_deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setIsOpen( bool deviceOpen )
{
    m_deviceOpen = deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswPerforationAICD::length() const
{
    return m_length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setLength( double length )
{
    m_length = length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswPerforationAICD::flowScalingFactor() const
{
    return m_flowScalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setflowScalingFactor( double scalingFactor )
{
    m_flowScalingFactor = scalingFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::array<double, AICD_NUM_PARAMS>& RicMswPerforationAICD::values() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, AICD_NUM_PARAMS>& RicMswPerforationAICD::values()
{
    return m_parameters;
}
