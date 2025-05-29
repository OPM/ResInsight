/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "Summary/RiaSummaryDefines.h"

#include "cafPdmUiItem.h"

#include <QStringList>

#include <vector>

class RimSummaryEnsemble;
class RimEnsembleFileSet;

namespace RimEnsembleFileSetTools
{
std::vector<RimSummaryEnsemble*> createSummaryEnsemblesFromFileSets( const std::vector<RimEnsembleFileSet*> fileSets );
std::vector<RimEnsembleFileSet*> createEnsembleFileSets( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode groupingMode );

RimEnsembleFileSet* createEnsembleFileSetFromOpm( const QString& pathPattern, const QString& name );

QList<caf::PdmOptionItemInfo> ensembleFileSetOptions();

}; // namespace RimEnsembleFileSetTools
