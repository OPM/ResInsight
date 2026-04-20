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

#include <QString>

//==================================================================================================
/// Abstract data source for a tabbed text dialog. One instance feeds one dialog.
/// The owning dialog queries these methods lazily as the user switches tabs.
//==================================================================================================
class RimTabbedTextProvider
{
public:
    virtual ~RimTabbedTextProvider() = default;

    virtual bool    isValid() const                = 0;
    virtual QString description() const            = 0;
    virtual QString tabTitle( int tabIndex ) const = 0;
    virtual QString tabText( int tabIndex ) const  = 0;
    virtual int     tabCount() const               = 0;
};
