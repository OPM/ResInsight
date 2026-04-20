/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include <memory>

class RimTabbedTextProvider;

//==================================================================================================
/// Capability mixin: a plot type that can produce a tabbed text representation
/// for the "Show Plot Data" feature. Only opt-in plot types inherit this.
//==================================================================================================
class RimTabbedTextProducer
{
public:
    virtual ~RimTabbedTextProducer() = default;

    virtual std::unique_ptr<RimTabbedTextProvider> createTabbedTextProvider() const = 0;
};
