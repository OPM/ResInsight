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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafSignal.h"

#include <QString>

class RimCustomObjectiveFunction;

//==================================================================================================
///
//==================================================================================================
class RimCustomObjectiveFunctionCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<RimCustomObjectiveFunction*> objectiveFunctionAdded;
    caf::Signal<RimCustomObjectiveFunction*> objectiveFunctionChanged;
    caf::Signal<RimCustomObjectiveFunction*> objectiveFunctionAboutToBeDeleted;
    caf::Signal<>                            objectiveFunctionDeleted;

public:
    RimCustomObjectiveFunctionCollection();

    RimCustomObjectiveFunction* addObjectiveFunction();
    void                        onObjectiveFunctionChanged( RimCustomObjectiveFunction* objectiveFunction );
    void                        removeObjectiveFunction( RimCustomObjectiveFunction* objectiveFunction );
    std::vector<RimCustomObjectiveFunction*> objectiveFunctions() const;

private:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /* = "" */ ) override;

private:
    caf::PdmChildArrayField<RimCustomObjectiveFunction*> m_objectiveFunctions;
};
