/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfColor3.h"

class RimEclipseWell;
class RigFlowDiagResults;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFlowDiagSolution : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;
public:

    RimFlowDiagSolution();
    virtual ~RimFlowDiagSolution();

    QString                 userDescription() const; 
    RigFlowDiagResults*     flowDiagResults();
    std::vector<QString>    tracerNames() const;

    std::map<std::string, std::vector<int> > allInjectorTracerActiveCellIndices(size_t timeStepIndex) const;
    std::map<std::string, std::vector<int> > allProducerTracerActiveCellIndices(size_t timeStepIndex) const;

    enum TracerStatusType
    {
        CLOSED,    ///< Tracer has no active cells, and does not contribute
        PRODUCER,  ///< Tracer with producing cells
        INJECTOR,  ///< Tracer with injecting cells
        VARYING,   ///< Tracer is producing and injecting at different time steps. Only used as a timestep-overall type
        UNDEFINED  ///< Used as "Any" or "not set"
    };

    TracerStatusType tracerStatusOverall(const QString& tracerName) const;
    TracerStatusType tracerStatusInTimeStep(const QString& tracerName, size_t timeStepIndex) const;
    cvf::Color3f     tracerColor(const QString& tracerName) const;

    static bool    hasCrossFlowEnding(const QString& tracerName);
    static QString removeCrossFlowEnding(const QString& tracerName);
    static QString addCrossFlowEnding(const QString& wellName);
private:
    std::map<std::string, std::vector<int> > allTracerActiveCellIndices(size_t timeStepIndex, bool useInjectors) const;

    virtual caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmField<QString> m_userDescription;

    cvf::ref<RigFlowDiagResults> m_flowDiagResults;
};
