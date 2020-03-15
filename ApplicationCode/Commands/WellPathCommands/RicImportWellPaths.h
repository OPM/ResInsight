/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "CommandFileInterface/Core/RicfCommandObject.h"

#include "cafCmdFeature.h"
#include "cafPdmField.h"

#include <QString>
#include <QStringList>

#include <vector>

class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RicImportWellPaths : public caf::CmdFeature, public RicfCommandObject
{
    RICF_HEADER_INIT;

public:
    RicImportWellPaths();
    caf::PdmScriptResponse execute() override;

protected:
    static std::vector<RimWellPath*> importWellPaths( const QStringList& wellPathFilePaths, QStringList* errorMessages );
    static QStringList               wellPathNameFilters();

    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    caf::PdmField<QString>              m_wellPathFolder;
    caf::PdmField<std::vector<QString>> m_wellPathFiles;
};
