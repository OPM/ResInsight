/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimVfpTableExtractor.h"

// #include "RiaApplication.h"

// #include "RimCase.h"
// #include "RimEclipseCase.h"
// #include "RimGeoMechCase.h"
// #include "RimOilField.h"
// #include "RimProject.h"
// #include "RimWellLogFile.h"
// #include "RimWellPath.h"
// #include "RimWellPathCollection.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

// #include <QDateTime>
// #include <QDir>
// #include <QFileInfo>

#include "opm/parser/eclipse/EclipseState/Schedule/VFPInjTable.hpp"
#include "opm/parser/eclipse/EclipseState/Schedule/VFPProdTable.hpp"
#include "opm/parser/eclipse/Parser/Parser.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPInjTable> RimVfpTableExtractor::extractVfpInjectionTables( const std::string& filename )
{
    std::vector<Opm::VFPInjTable> tables;

    Opm::Parser parser;
    auto        deck = parser.parseFile( filename );

    std::string myKeyword   = "VFPINJ";
    auto        keywordList = deck.getKeywordList( myKeyword );

    Opm::UnitSystem unitSystem;

    for ( auto kw : keywordList )
    {
        auto name = kw->name();

        Opm::VFPInjTable table( *kw, unitSystem );
        tables.push_back( table );
    }

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPProdTable> RimVfpTableExtractor::extractVfpProductionTables( const std::string& filename )
{
    std::vector<Opm::VFPProdTable> tables;

    Opm::Parser parser;
    auto        deck = parser.parseFile( filename );

    std::string myKeyword   = "VFPPROD";
    auto        keywordList = deck.getKeywordList( myKeyword );

    Opm::UnitSystem unitSystem;

    for ( auto kw : keywordList )
    {
        auto name = kw->name();

        Opm::VFPProdTable table( *kw, unitSystem );
        tables.push_back( table );
    }

    return tables;
}
