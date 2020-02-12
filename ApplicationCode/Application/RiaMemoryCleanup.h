/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiItem.h"

class RimCase;
class Rim3dView;
class RigFemResultAddress;
class RigEclipseResultAddress;

class RiaMemoryCleanup : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaMemoryCleanup();

    void setPropertiesFromView( Rim3dView* view );
    void clearSelectedResultsFromMemory();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    std::vector<RigFemResultAddress>     selectedGeoMechResults() const;
    std::vector<RigEclipseResultAddress> selectedEclipseResults() const;
    std::set<RigFemResultAddress>        findGeoMechCaseResultsInUse() const;
    std::set<RigEclipseResultAddress>    findEclipseResultsInUse() const;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmPtrField<RimCase*>           m_case;
    caf::PdmField<std::vector<size_t>>   m_resultsToDelete;
    std::vector<RigFemResultAddress>     m_geomResultAddresses;
    std::vector<RigEclipseResultAddress> m_eclipseResultAddresses;
    caf::PdmField<bool>                  m_performDelete;
};