/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RigGroupNetwork.h"

#include <QString>

#include <expected>

namespace Opm
{
class Deck;
}

//==================================================================================================
/// Reads the dated GRUPTREE and BRANPROP network trees from a deck, following the accumulation rules
/// of res2df (gruptree.py):
///
/// - START, DATES and TSTEP advance the current date. When the date advances and a network keyword
///   was seen since the last date, the complete current tree is stored with the previous date.
/// - GRUPTREE, BRANPROP and WELSPECS accumulate edges. Unlike res2df, edges are keyed by the child,
///   so a repeated GRUPTREE or WELSPECS moves the child to the new parent, as the simulator does.
///   A BRANPROP record with VFP table 9999 removes the branch.
/// - GRUPNET and NODEPROP give the node property TERMINAL_PRESSURE. BRANPROP gives the edge
///   properties VFP_TABLE and ALQ.
/// - GRUPTREE trees: all wells are added, and groups from WELSPECS that are not in the tree are
///   connected to FIELD. BRANPROP trees: only wells whose group is a node in the tree are added.
//==================================================================================================
namespace RifGroupNetworkReader
{
// Warnings about ignored keywords are logged if logWarnings is true
std::expected<RigGroupNetwork, QString> readFromFile( const QString& deckFileName, bool logWarnings = true );
RigGroupNetwork                         readFromDeck( const Opm::Deck& deck, bool logWarnings = true );
}; // namespace RifGroupNetworkReader
