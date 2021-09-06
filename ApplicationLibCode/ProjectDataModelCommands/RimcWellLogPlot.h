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

#include "RimWellLogPlot.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimWellPath;
class RimWellLogTrack;

//==================================================================================================
///
//==================================================================================================
class RimcWellLogPlot_newWellLogTrack : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellLogPlot_newWellLogTrack( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

    static RimWellLogTrack* createWellLogTrack( RimWellLogPlot* wellLogPlot,
                                                RimEclipseCase* eclipseCase,
                                                RimWellPath*    wellPath,
                                                const QString&  title );

private:
    caf::PdmField<QString>            m_title;
    caf::PdmPtrField<RimEclipseCase*> m_case;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
};
