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

class RimColorLegend;
class RimStimPlanModelTemplate;

namespace cvf
{
class Color3f;
}

class QString;

//==================================================================================================
///
//==================================================================================================
class RicFaciesPropertiesImportTools
{
public:
    static void importFaciesPropertiesFromFile( const QString&            filePath,
                                                RimStimPlanModelTemplate* stimPlanModelTemplate,
                                                bool                      createColorLegend = false );

private:
    static int  computeEditDistance( const QString& a, const QString& b );
    static bool matchByName( const QString& name, RimColorLegend* colorLegend, cvf::Color3f& color );
    static bool predefinedColorMatch( const QString& name, RimColorLegend* colorLegend, cvf::Color3f& color );
};
