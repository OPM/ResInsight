/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionResultsDefinitionCollection.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfVector3.h"

#include <vector>

class RigFemPartCollection;
class RigFemResultAddress;

class RimModeledWellPath;
class RimWellLogTrack;
class RimWellLogExtractionCurve;

class RimGeoMechFaultReactivationResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechFaultReactivationResult();
    ~RimGeoMechFaultReactivationResult() override;

private:
    void onLoadDataAndUpdate();

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void createWellGeometry();
    void createWellLogCurves();

    int getPartIndexFromPoint( const RigFemPartCollection* const partCollection, const cvf::Vec3d& point ) const;

    RimWellLogExtractionCurve* createWellLogExtractionCurveAndAddToTrack( RimWellLogTrack*           track,
                                                                          const RigFemResultAddress& resultAddress,
                                                                          RimModeledWellPath*        wellPath,
                                                                          int                        partId );

    QString plotDescription() const;

private:
    caf::PdmPtrField<RimExtrudedCurveIntersection*> m_intersection;

    caf::PdmField<bool> m_createFaultReactivationResult;

    caf::PdmField<double> m_distanceFromIntersection; // To move wells to each side of intersection
    caf::PdmField<double> m_widthOutsideIntersection; // To stretch well points outside intersection

    caf::PdmPtrField<RimModeledWellPath*> m_faceAWellPath;
    caf::PdmPtrField<RimModeledWellPath*> m_faceBWellPath;

    caf::PdmField<int> m_faceAWellPathPartIndex;
    caf::PdmField<int> m_faceBWellPathPartIndex;
};
