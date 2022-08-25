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

#pragma once

#include "RiaDefines.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimWellPath;
class RimWellLogTrack;
class RimWellLogExtractionCurve;

//==================================================================================================
///
//==================================================================================================
class RimcWellLogTrack_addExtractionCurve : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellLogTrack_addExtractionCurve( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

    static RimWellLogExtractionCurve* addExtractionCurve( RimWellLogTrack*          wellLogTrack,
                                                          RimEclipseCase*           eclipseCase,
                                                          RimWellPath*              wellPath,
                                                          const QString&            propertyName,
                                                          RiaDefines::ResultCatType resultCategoryType,
                                                          int                       timeStep );

private:
    caf::PdmPtrField<RimEclipseCase*> m_case;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
    caf::PdmField<QString>            m_propertyType;
    caf::PdmField<QString>            m_propertyName;
    caf::PdmField<int>                m_timeStep;
};
