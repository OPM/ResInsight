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
#pragma once

#include "RicMswBranch.h"

#include "RigCompletionData.h"

#include "RimWellPathAicdParameters.h"

#include "cvfMath.h"

#include <QString>
#include <memory>

class RimWellPathValve;

//==================================================================================================
///
//==================================================================================================
class RicMswCompletion : public RicMswBranch
{
public:
    RicMswCompletion( const QString& label, double startMD, double startTVD, size_t index = cvf::UNDEFINED_SIZE_T );

    virtual RigCompletionData::CompletionType completionType() const = 0;
    size_t         index() const;

private:
    size_t  m_index;
};

class RicMswFishbones : public RicMswCompletion
{
public:
    RicMswFishbones( const QString& label, double startMD, double startTVD, size_t index = cvf::UNDEFINED_SIZE_T )
        : RicMswCompletion( label, startMD, startTVD, index )
    {
    }

    RigCompletionData::CompletionType completionType() const override { return RigCompletionData::FISHBONES; }
};

//==================================================================================================
///
//==================================================================================================
class RicMswFracture : public RicMswCompletion
{
public:
    RicMswFracture( const QString& label, double startMD, double startTVD, size_t index = cvf::UNDEFINED_SIZE_T );
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforation : public RicMswCompletion
{
public:
    RicMswPerforation( const QString& label, double startMD, double startTVD, size_t index = cvf::UNDEFINED_SIZE_T );
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswValve : public RicMswCompletion
{
public:
    RicMswValve( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );

    virtual ~RicMswValve() {}

    const RimWellPathValve* wellPathValve() const;

    bool isValid() const;
    void setIsValid( bool valid );

private:
    bool                    m_valid;
    const RimWellPathValve* m_wellPathValve;
};

//==================================================================================================
///
//==================================================================================================
class RicMswWsegValve : public RicMswValve
{
public:
    RicMswWsegValve( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );

    double flowCoefficient() const;
    double area() const;
    void   setFlowCoefficient( double icdFlowCoefficient );
    void   setArea( double icdArea );

private:
    double m_flowCoefficient;
    double m_area;
};

//==================================================================================================
///
//==================================================================================================
class RicMswFishbonesICD : public RicMswWsegValve
{
public:
    RicMswFishbonesICD( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforationICD : public RicMswWsegValve
{
public:
    RicMswPerforationICD( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforationICV : public RicMswWsegValve
{
public:
    RicMswPerforationICV( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );
    RigCompletionData::CompletionType completionType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RicMswPerforationAICD : public RicMswValve
{
public:
    RicMswPerforationAICD( const QString& label, double startMD, double startTVD, const RimWellPathValve* wellPathValve );
    RigCompletionData::CompletionType completionType() const override;

    bool   isOpen() const;
    void   setIsOpen( bool deviceOpen );
    double length() const;
    void   setLength( double length );
    double flowScalingFactor() const;
    void   setflowScalingFactor( double scalingFactor );

    const std::array<double, AICD_NUM_PARAMS>& values() const;
    std::array<double, AICD_NUM_PARAMS>&       values();

private:
    bool                                m_deviceOpen;
    std::array<double, AICD_NUM_PARAMS> m_parameters;
    double                              m_length;
    double                              m_flowScalingFactor;
};
