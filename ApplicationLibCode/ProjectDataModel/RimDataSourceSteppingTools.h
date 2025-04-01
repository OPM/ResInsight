/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RifEclipseSummaryAddressDefines.h"

#include "cafPdmUiItem.h"
#include "cafPdmValueField.h"

class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RimDataSourceSteppingTools
{
public:
    static void modifyCurrentIndex( caf::PdmValueField*                  valueField,
                                    const QList<caf::PdmOptionItemInfo>& options,
                                    int                                  indexOffset,
                                    bool                                 notifyChange = true );

    static bool updateAddressIfMatching( const QVariant&                                  oldValue,
                                         const QVariant&                                  newValue,
                                         RifEclipseSummaryAddressDefines::SummaryCategory category,
                                         RifEclipseSummaryAddress&                        adr );

    static bool updateQuantityIfMatching( const QVariant& previousName, const QVariant& newName, RifEclipseSummaryAddress& adr );
    static bool updateQuantityIfMatching( const std::string& previousName, const std::string& newName, RifEclipseSummaryAddress& adr );
};
