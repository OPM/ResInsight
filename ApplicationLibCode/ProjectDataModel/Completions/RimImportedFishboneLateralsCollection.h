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

#include "RimCheckableNamedObject.h"
#include "RimFishbonesPipeProperties.h"
#include "RimImportedFishboneLaterals.h"

#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RimImportedFishboneLateralsCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimImportedFishboneLateralsCollection();

    void importCompletionsFromFile( const QStringList& filePaths );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    std::vector<const RimImportedFishboneLaterals*> wellPaths() const;
    double                                          holeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const
    {
        return m_pipeProperties->holeDiameter( unitSystem );
    }
    double skinFactor() const { return m_pipeProperties->skinFactor(); }

    void setUnitSystemSpecificDefaults();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void appendCompletion( RimImportedFishboneLaterals* completion );

private:
    caf::PdmChildArrayField<RimImportedFishboneLaterals*> m_wellPaths;
    caf::PdmChildField<RimFishbonesPipeProperties*>       m_pipeProperties;
};
