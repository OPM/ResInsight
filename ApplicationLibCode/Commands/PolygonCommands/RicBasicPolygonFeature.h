////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafCmdFeature.h"

#include <vector>

class RimPolygon;

//==================================================================================================
///
//==================================================================================================
class RicBasicPolygonFeature : public caf::CmdFeature
{
public:
    RicBasicPolygonFeature( bool multiSelectSupported );

protected:
    std::vector<RimPolygon*> selectedPolygons() const;
    bool                     isCommandEnabled() const override;

private:
    bool m_multiSelectSupported;
};
