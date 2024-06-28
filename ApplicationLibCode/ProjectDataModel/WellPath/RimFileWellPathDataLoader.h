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

#pragma once

#include "RimFileWellPath.h"

#include "cafDataLoader.h"
#include "cafProgressInfo.h"

#include <QString>

#include <memory>

#include "RifWellPathImporter.h"

//==================================================================================================
///
///
//==================================================================================================
class RimFileWellPathDataLoader : public caf::DataLoader
{
public:
    RimFileWellPathDataLoader();
    void loadData( caf::PdmObject& pdmObject, const QString& dataType, int taskId, caf::ProgressInfo& progressInfo ) override;
    bool isRunnable() const override;

private:
    std::unique_ptr<RifWellPathImporter> m_wellPathImporter;
};
