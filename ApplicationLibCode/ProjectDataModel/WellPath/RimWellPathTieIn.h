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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimWellPath;
class RimWellPathValve;

class RimWellPathTieIn : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathTieIn();

    void         connectWellPaths( RimWellPath* parentWell, RimWellPath* childWell, double tieInMeasuredDepth );
    RimWellPath* parentWell() const;
    double       tieInMeasuredDepth() const;
    void         setTieInMeasuredDepth( double measuredDepth );

    RimWellPath* childWell() const;
    void         updateChildWellGeometry();

    void updateFirstTargetFromParentWell();

    const RimWellPathValve* outletValve() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<QString> m_infoLabel;

    caf::PdmPtrField<RimWellPath*> m_parentWell;
    caf::PdmPtrField<RimWellPath*> m_childWell;
    caf::PdmField<double>          m_tieInMeasuredDepth;

    caf::PdmField<bool>                   m_addValveAtConnection;
    caf::PdmChildField<RimWellPathValve*> m_valve;
};
