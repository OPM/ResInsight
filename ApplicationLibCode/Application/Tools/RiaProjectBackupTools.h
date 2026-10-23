/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

//==================================================================================================
//
//==================================================================================================
namespace RiaProjectBackupTools
{
bool appendTextToDatabase( const QString& databaseFilePath, int maximumRecordCount, const QString& content );

// Returns the paths of the restored files, or an error message
std::expected<QStringList, QString> extractVersionsToFolder( const QString& databaseFilePath, const QString& destinationFolder );
} // namespace RiaProjectBackupTools
