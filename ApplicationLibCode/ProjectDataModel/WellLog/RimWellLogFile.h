/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RimWellLogFileChannel;
class RimWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogFile : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogFile();
    ~RimWellLogFile() override;

    virtual void                                setFileName( const QString& fileName );
    virtual QString                             fileName() const;
    virtual std::vector<RimWellLogFileChannel*> wellLogChannels() const;

    virtual std::vector<std::pair<double, double>>
        findMdAndChannelValuesForWellPath( const RimWellPath& wellPath, const QString& channelName, QString* unitString = nullptr ) = 0;

protected:
    caf::PdmChildArrayField<RimWellLogFileChannel*> m_wellLogChannelNames;
    caf::PdmField<caf::FilePath>                    m_fileName;
};
