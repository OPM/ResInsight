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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RimCase.h"
#include "RimInputPropertyCollection.h"


class QString;

class RifReaderInterface;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimInputCase : public RimCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimInputCase();
    virtual ~RimInputCase();

    // Fields
    caf::PdmField<std::vector<QString> >       m_additionalFileNames;
    caf::PdmField<QString>                     m_gridFileName;
    
    caf::PdmField<RimInputPropertyCollection*> m_inputPropertyCollection;

    // File open methods
    void                        openDataFileSet(const QStringList& filenames);
    void                        loadAndSyncronizeInputProperties();

    void                        removeProperty(RimInputProperty* inputProperty);

    // RimCase overrides
    virtual bool                openEclipseGridFile(); // Find grid file among file set. Read, Find read and validate property date. Syncronize child property sets.

    // PdmObject overrides
    virtual void                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    virtual QString             locationOnDisc() const;

protected:
    virtual void                initAfterRead();

private:
    void                        addFiles(const QStringList& newFileNames);
    void                        removeFiles(const QStringList& obsoleteFileNames);


    cvf::ref<RifReaderInterface> createMockModel(QString modelName);
};
