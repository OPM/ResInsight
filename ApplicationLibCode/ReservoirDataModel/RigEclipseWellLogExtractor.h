/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigWellLogExtractor.h"

class RigEclipseCaseData;
class RigWellPath;
class RigResultAccessor;
class RimEclipseResultDefinition;

namespace cvf
{
class BoundingBox;
}

class RigEclipseExtractorResultAddress : public RigExtractorResultAddress
{
public:
    RigEclipseExtractorResultAddress( const RimEclipseResultDefinition* resultDefinition, size_t gridIndex, size_t timeStepIndex )
        : m_resultDefinition( resultDefinition )
        , m_gridIndex( gridIndex )
        , m_timeStepIndex( timeStepIndex )
    {
    }

    const RimEclipseResultDefinition* resultDefinition() const { return m_resultDefinition; }
    size_t                            gridIndex() const { return m_gridIndex; }
    size_t                            timeStepIndex() const { return m_timeStepIndex; }

private:
    const RimEclipseResultDefinition* m_resultDefinition;
    size_t                            m_gridIndex;
    size_t                            m_timeStepIndex;
};

//==================================================================================================
///
//==================================================================================================
class RigEclipseWellLogExtractor : public RigWellLogExtractor
{
public:
    RigEclipseWellLogExtractor( gsl::not_null<const RigEclipseCaseData*> aCase,
                                gsl::not_null<const RigWellPath*>        wellpath,
                                const std::string&                       wellCaseErrorMsgName );

    std::vector<double> curveData( const RigExtractorResultAddress* resultAddress ) override;

    void                      curveData( const RigResultAccessor* resultAccessor, std::vector<double>* values );
    const RigEclipseCaseData* caseData() { return m_caseData.p(); }

private:
    void                calculateIntersection();
    std::vector<size_t> findCloseCellIndices( const cvf::BoundingBox& bb );
    cvf::Vec3d
        calculateLengthInCell( size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint ) const override;

    double computeLengthThreshold() const;

private:
    cvf::cref<RigEclipseCaseData> m_caseData;
};
