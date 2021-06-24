/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RimMultipleLocations.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimModeledWellPath;

namespace caf
{
class PdmUiPropertyViewDialog;
}

//==================================================================================================
///
//==================================================================================================
class RicCreateMultipleWellPathLateralsUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicCreateMultipleWellPathLateralsUi();

    void setSourceLateral( RimModeledWellPath* lateral );
    void setDefaultValues( double start, double end );

    RimModeledWellPath*   sourceLateral() const;
    RimMultipleLocations* locationConfig() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmPtrField<RimModeledWellPath*> m_sourceLateral;

    caf::PdmChildField<RimMultipleLocations*> m_locations;
};
