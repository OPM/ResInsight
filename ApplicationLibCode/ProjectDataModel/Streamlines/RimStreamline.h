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

#include "RigTracer.h"

#include "RiaDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmProxyValueField.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RimStreamline : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimStreamline( QString simwellname, size_t startCellIdx, cvf::StructGridInterface::FaceType faceIdx );
    ~RimStreamline() override;

    const RigTracer& tracer() const;
    const QString    simWellName() const;
    size_t           size() const;

    void addTracerPoint( cvf::Vec3d position, cvf::Vec3d direction, RiaDefines::PhaseType dominantPhase );
    void reverse();
    void generateStatistics();

    size_t                             m_startCellIdx;
    cvf::StructGridInterface::FaceType m_faceIdx;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    RigTracer              m_tracer;
    caf::PdmField<QString> m_simWellName;

    // caf::PdmField<QString> m_propertiesTable;
};
