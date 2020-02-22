/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimEclipseCase.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include "cvfObject.h"

class RifReaderInterface;
class RimEclipseInputProperty;
class RimEclipseInputPropertyCollection;

//==================================================================================================
//
//
//
//==================================================================================================
class RimEclipseInputCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseInputCase();
    ~RimEclipseInputCase() override;

    // File open methods
    bool openDataFileSet( const QStringList& fileNames );
    bool importAsciiInputProperties( const QStringList& fileNames ) override;
    void loadAndSyncronizeInputProperties();

    // RimCase overrides
    bool openEclipseGridFile() override;
    void reloadEclipseGridFile() override;

    // Overrides from RimCase
    QString locationOnDisc() const override;
    QString gridFileName() const override { return m_gridFileName().path(); }

    void updateFilePathsFromProjectPath( const QString& projectPath, const QString& oldProjectPath ) override;

    void updateAdditionalFileFolder( const QString& newFolder );

private:
    std::vector<QString> additionalFiles() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    cvf::ref<RifReaderInterface> createMockModel( QString modelName );

private:
    // Fields
    caf::PdmField<caf::FilePath>                  m_gridFileName;
    caf::PdmProxyValueField<std::vector<QString>> m_additionalFiles;

    // Obsolete fields
    caf::PdmField<std::vector<QString>> m_additionalFilenames_OBSOLETE;
};
