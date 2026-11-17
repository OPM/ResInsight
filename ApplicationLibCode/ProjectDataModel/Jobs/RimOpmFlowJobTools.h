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

#include "RimJobWellSettings.h"

#include <QDateTime>
#include <QString>

#include <vector>

class RimEclipseCase;
class RifOpmFlowDeckFile;
class RimWellPath;

namespace RimOpmFlowJobTools
{
RimEclipseCase* findExistingCase( QString filename );

int mergeMswData( RifOpmFlowDeckFile*              deckFile,
                  int                              mergeDeckPosition,
                  RimEclipseCase*                  eclipseCase,
                  RimWellPath*                     wellPath,
                  RimJobWellSettings::WellOpenType wellOpenType,
                  int                              openTimeStep );

int mergeBasicWellSettings( RifOpmFlowDeckFile*              deckFile,
                            int                              mergeDeckPosition,
                            RimEclipseCase*                  eclipseCase,
                            RimWellPath*                     wellPath,
                            RimJobWellSettings::WellOpenType wellOpenType,
                            int                              openTimeStep,
                            QString                          wellGroupName );

} // namespace RimOpmFlowJobTools