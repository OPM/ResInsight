/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "Polygons/RimPolygon.h"

#include "cafPdmField.h"

//==================================================================================================
///
/// A RimPolygon fetched from Sumo, stamped with the full Sumo identity (case, ensemble,
/// realization, polygon result, name, contact type) it was fetched with -- a self-contained,
/// immutable snapshot that a RimPolygonCloudAddress can hand out for any realization without
/// external bookkeeping or mutation.
///
/// Identity fields are hidden from the UI (traceability/debugging only) -- editing happens on
/// RimPolygonCloudAddress.
///
//==================================================================================================
class RimCloudPolygon : public RimPolygon
{
    CAF_PDM_HEADER_INIT;

public:
    RimCloudPolygon();

    void setSumoIdentity( const QString& caseId,
                          const QString& ensembleName,
                          int            realization,
                          const QString& polygonResult,
                          const QString& sumoName,
                          const QString& contactType );

    QString caseId() const;
    QString ensembleName() const;
    int     realization() const;
    QString polygonResult() const;
    QString sumoName() const;
    QString contactType() const;

private:
    caf::PdmField<QString> m_caseId;
    caf::PdmField<QString> m_ensembleName;
    caf::PdmField<int>     m_realization;
    caf::PdmField<QString> m_polygonResult;
    caf::PdmField<QString> m_sumoName;
    caf::PdmField<QString> m_contactType;
};
