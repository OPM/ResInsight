/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include <QString>

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Central, discoverable registry of experimental features.
///
/// To add a new experimental feature, append an entry to the list returned by availableFeatures()
/// (see RiaExperimentalFeatures.cpp) and gate the code path behind
/// RiaPreferencesSystem::isFeatureEnabled( "<keyword>" ).
//--------------------------------------------------------------------------------------------------
class RiaExperimentalFeatures
{
public:
    struct Feature
    {
        QString keyword; ///< Stable identifier, passed to RiaPreferencesSystem::isFeatureEnabled().
        QString displayName; ///< Human readable name shown as the checkbox label.
        QString description; ///< Short explanation, appended to the checkbox label.
    };

    /// The single, discoverable list of all experimental features.
    static const std::vector<Feature>& availableFeatures();

    /// Special keyword that enables every experimental feature at once.
    static QString enableAllKeyword();
};
