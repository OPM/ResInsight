/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 - Equinor ASA
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
#include <QStringList>

#include <expected>
#include <vector>

class RimEclipseCase;
class RimFormationNames;

class RimFormationTools
{
public:
    static QStringList        formationFoldersFromCaseFileName( const QString caseFileName );
    static RimFormationNames* loadFormationNamesFromFolder( const QStringList& folderNames );

    // Formation indices are the position of a formation in the formation list. A name may occur more
    // than once (a formation split over several K ranges), so all indices with a matching name are
    // returned, in ascending order. An empty request selects all formations. Unknown names give an error.
    static std::expected<std::vector<int>, QString> formationIndicesFromNames( const std::vector<QString>& availableNames,
                                                                               const std::vector<QString>& requestedNames );

    // Names of the given formation indices, in formation list order, without duplicates
    static std::vector<QString> formationNamesFromIndices( const std::vector<QString>& availableNames, const std::vector<int>& indices );

    // Names in the order of first appearance: the first list, followed by names not seen before
    static std::vector<QString> unionOfFormationNames( const std::vector<std::vector<QString>>& formationNameLists );

    // The ordered formation names of a case, from its own formations or the formations of its ensemble.
    // Does not require the grid to be loaded.
    static std::vector<QString> formationNames( const RimEclipseCase* eclipseCase );

private:
    RimFormationTools() {};
};