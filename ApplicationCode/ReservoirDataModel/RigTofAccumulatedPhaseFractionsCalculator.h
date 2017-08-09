/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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
#include "cafPdmPtrField.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <array>
#include "RimFlowDiagSolution.h"

class RimEclipseCase;

//==================================================================================================
/// 
//==================================================================================================

class RigTofAccumulatedPhaseFractionsCalculator
{

public:
    void computeTOFaccumulations();

    void sortTofAndCalculateAccPhaseFraction(const std::vector<double>* tofData, const std::vector<double>* fractionData, std::vector<double>& porvResults, std::vector<double>& swatResults, std::vector<double>& soilResults, std::vector<double>& sgasResults);

private:
    RimEclipseCase*                             m_case;
    QString                                     m_wellName;

    caf::PdmField<int>                          m_timeStep;
    caf::PdmPtrField<RimFlowDiagSolution*>      m_flowDiagSolution;

};

