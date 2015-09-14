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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"


class RimWellLasLog;
class QString;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLasFileInfo : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLasFileInfo();
    virtual ~RimWellLasFileInfo();

    void setFileName(const QString& fileName);
    bool readFile();
    void close();
    
    QString wellName() const;

    virtual caf::PdmFieldHandle* userDescriptionField()  { return &m_name; }

private:
    caf::PdmChildArrayField<RimWellLasLog*>  m_lasFileLogs;

private:
    caf::PdmField<QString> m_wellName;
    caf::PdmField<QString> m_fileName;
    caf::PdmField<QString> m_name;
};
