/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cvfBase.h"
#include "cvfObject.h"

class RimEclipseInputProperty;
class RimEclipseInputPropertyCollection;

//==================================================================================================
//
// This class is intended to replace RimEclipseInputCase when the opm-parser is considered stable
//
//==================================================================================================
class RimEclipseInputCaseOpm : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseInputCaseOpm();
    virtual ~RimEclipseInputCaseOpm();

    void                        importNewEclipseGridAndProperties(const QString& fileName);

    void                        appendPropertiesFromStandaloneFiles(const QStringList& fileNames);

    // RimCase overrides
    virtual bool                openEclipseGridFile(); // Find grid file among file set. Read, Find read and validate property date. Syncronize child property sets.

    // Overrides from RimCase
    virtual QString             locationOnDisc() const;
    virtual QString             gridFileName() const { return m_gridFileName();}

    virtual void                updateFilePathsFromProjectPath(const QString& projectPath, const QString& oldProjectPath);

private:
    void                        importEclipseGridAndProperties(const QString& fileName);
    void                        loadAndSyncronizeInputProperties();


private:
    caf::PdmChildField<RimEclipseInputPropertyCollection*> m_inputPropertyCollection;
    caf::PdmField<std::vector<QString> >       m_additionalFileNames;

    caf::PdmField<QString>      m_gridFileName;
};
