/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimFileWellPathDataLoader.h"

#include "RiaLogging.h"
#include "RimFileWellPath.h"

#include "RigWellPath.h"

#include "cafProgressInfo.h"

#include <QApplication>
#include <QObject>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFileWellPathDataLoader::RimFileWellPathDataLoader()
    : caf::DataLoader()
{
    m_wellPathImporter = std::make_unique<RifWellPathImporter>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFileWellPathDataLoader::loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo )
{
    auto* fWPath = dynamic_cast<RimFileWellPath*>( &pdmObject );
    if ( fWPath && !fWPath->filePath().isEmpty() )
    {
        QString errorMessage;
        if ( !fWPath->readWellPathFile( &errorMessage, m_wellPathImporter.get(), false ) )
        {
            RiaLogging::warning( errorMessage );
        }
    }
    taskDone.send( dataType, taskId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFileWellPathDataLoader::isRunnable() const
{
    return true;
}
