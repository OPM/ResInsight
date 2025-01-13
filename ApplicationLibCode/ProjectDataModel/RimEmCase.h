/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Ceetron Solutions AS
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

#include "RimEclipseCase.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include <array>

struct RimEmData
{
    std::array<double, 3>                     originNED;
    std::array<double, 3>                     cellSizes;
    std::array<int, 3>                        ijkNumCells;
    std::map<std::string, std::vector<float>> resultData;
};

//==================================================================================================
//
//
//==================================================================================================
class RimEmCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEmCase();
    ~RimEmCase() override;

    bool openEclipseGridFile() override;

    QString locationOnDisc() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    RimEmData readDataFromFile();
};
