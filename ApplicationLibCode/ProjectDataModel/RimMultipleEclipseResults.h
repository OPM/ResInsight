/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RimCheckableObject.h"

#include <QString>

class RigEclipseResultAddress;
class RimEclipseView;

//==================================================================================================
///
///
//==================================================================================================
class RimMultipleEclipseResults : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimMultipleEclipseResults();

    void setEclipseView( RimEclipseView* eclipseView );

    std::vector<RigEclipseResultAddress> additionalResultAddresses() const;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<std::vector<QString>> m_selectedKeywords;
    caf::PdmPointer<RimEclipseView>     m_eclipseView;
};
