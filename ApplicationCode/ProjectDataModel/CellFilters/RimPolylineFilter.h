/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCellFilter.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"

#include <memory>

class RicPolylineCellPickEventHandler;
class RimPolylineTarget;

//==================================================================================================
///
///
//==================================================================================================
class RimPolylineFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolylineFilter();
    ~RimPolylineFilter() override;

    void updateVisualization();

    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert );

protected:
private:
    // caf::PdmField<QString>                      m_name;
    caf::PdmField<bool>                         m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;

    std::shared_ptr<RicPolylineCellPickEventHandler> m_pickTargetsEventHandler;
};
