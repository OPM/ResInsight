/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RigFemResultPosEnum.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimWellPath;
class RimWbsParameters;

//==================================================================================================
/// Create a Well Bore Stability plot for a well path in this GeoMech case.
///
/// The plot is created with default parameters. Adjust them through the returned plot's Parameters
/// child object, or pass a RimWbsParameters object from C++ via setParameters().
//==================================================================================================
class RimGeoMechCase_createWellBoreStabilityPlot : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase_createWellBoreStabilityPlot( caf::PdmObjectHandle* self );

    void setWellPath( RimWellPath* wellPath );
    void setTimeStep( int timeStep );
    void setParameters( const RimWbsParameters* parameters );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmPtrField<RimWellPath*> m_wellPath;
    caf::PdmField<int>             m_timeStep;

    const RimWbsParameters* m_parameters = nullptr;
};

//==================================================================================================
/// Get the time step strings of this GeoMech case.
//==================================================================================================
class RimGeoMechCase_timeStepStrings : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase_timeStepStrings( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
/// Get the result names of this GeoMech case for a result position.
///
/// Names are returned as "FIELD:COMPONENT", or "FIELD" for results without components.
//==================================================================================================
class RimGeoMechCase_resultNames : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase_resultNames( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<caf::AppEnum<RigFemResultPosEnum>> m_resultPosition;
};

//==================================================================================================
/// Read the values of a GeoMech result into the key-value store.
///
/// A negative frame index selects the last frame of the time step. The values of all parts are
/// stored as a float array under the given key.
//==================================================================================================
class RimGeoMechCase_readResultValuesInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechCase_readResultValuesInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<caf::AppEnum<RigFemResultPosEnum>> m_resultPosition;
    caf::PdmField<QString>                           m_fieldName;
    caf::PdmField<QString>                           m_componentName;
    caf::PdmField<int>                               m_timeStep;
    caf::PdmField<int>                               m_frameIndex;
    caf::PdmField<QString>                           m_key;
};
