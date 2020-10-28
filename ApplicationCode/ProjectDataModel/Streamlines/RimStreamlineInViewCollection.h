/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"

#include <list>
#include <vector>

class RimStreamline;
class RimEclipseCase;
class RigTracer;
class RigCell;
class RigResultAccessor;
class RigGridBase;

class RimStreamlineInViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimStreamlineInViewCollection();
    ~RimStreamlineInViewCollection() override;

    void            setEclipseCase( RimEclipseCase* reservoir );
    RimEclipseCase* eclipseCase() const;

    RiaDefines::PhaseType phase() const;

    void goForIt();

    const std::list<RigTracer>& tracers();

protected:
    caf::PdmFieldHandle* objectToggleField() override;

private:
    void generateTracer( RigCell cell, double direction );
    void loadDataIfMissing( RiaDefines::PhaseType phase, int timeIdx );

    cvf::ref<RigResultAccessor>
        getDataAccessor( cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase, int timeIdx );

    bool setupDataAccessors( RiaDefines::PhaseType phase, int timeIdx );

    QString gridResultNameFromPhase( RiaDefines::PhaseType phase, cvf::StructGridInterface::FaceType faceIdx ) const;

    std::vector<double> getFaceValues( RigCell cell, RigGridBase* grid );

    RigCell* findNeighborCell( RigCell cell, RigGridBase* grid, cvf::StructGridInterface::FaceType face );

    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<QString>                             m_collectionName;
    caf::PdmField<double>                              m_flowThreshold;
    caf::PdmField<double>                              m_resolution;
    caf::PdmField<double>                              m_maxDays;
    caf::PdmPointer<RimEclipseCase>                    m_eclipseCase;
    caf::PdmChildArrayField<RimStreamline*>            m_streamlines;
    caf::PdmField<caf::AppEnum<RiaDefines::PhaseType>> m_phase;

    std::list<RigTracer> m_activeTracers;

    std::vector<cvf::ref<RigResultAccessor>> m_dataAccess;

    // cvf::ref<RigResultAccessor> m_dataI;
    // cvf::ref<RigResultAccessor> m_dataJ;
    // cvf::ref<RigResultAccessor> m_dataK;
};
