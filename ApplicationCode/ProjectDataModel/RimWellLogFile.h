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

#include "RigWellLogFile.h"

#include "cvfBase.h"

class RimWellLogFileChannel;

class QString;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    enum WellFlowCondition
    {
        WELL_FLOW_COND_RESERVOIR,
        WELL_FLOW_COND_STANDARD
    };

public:
    RimWellLogFile();
    virtual ~RimWellLogFile();

    static RimWellLogFile*               readWellLogFile(const QString& logFilePath);
                                         
    void                                 setFileName(const QString& fileName);
    QString                              fileName() const        { return m_fileName; }
                                         
    bool                                 readFile(QString* errorMessage);
                                         
    QString                              wellName() const;
    QString                              date() const;
                                         
    RigWellLogFile*                      wellLogFile()           { return m_wellLogDataFile.p(); }
    std::vector<RimWellLogFileChannel*>  wellLogChannels() const;

private:
    virtual void                         defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual caf::PdmFieldHandle*         userDescriptionField()  { return &m_name; }

    static QString                              formatDate(const QString dateString);

    caf::PdmChildArrayField<RimWellLogFileChannel*>  m_wellLogChannelNames;

private:
    cvf::ref<RigWellLogFile>             m_wellLogDataFile;
    caf::PdmField<QString>               m_wellName;
    caf::PdmField<QString>               m_fileName;
    caf::PdmField<QString>               m_name;
    caf::PdmField<QString>               m_date;
    caf::PdmField<caf::AppEnum<WellFlowCondition>>  m_wellFlowCondition;
};
