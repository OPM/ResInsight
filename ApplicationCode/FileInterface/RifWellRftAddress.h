/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPdmPointer.h"

#include <QPointer>
#include <QDate>
#include <QMetaType>

class RimWellLogFile;
class RimEclipseCase;


//==================================================================================================
///  
///  
//==================================================================================================
class RifWellRftAddress
{
public:
    enum SourceType
    {
        NONE,
        OBSERVED,
        RFT,
        GRID
    };

    RifWellRftAddress();
    RifWellRftAddress(SourceType sourceType, RimEclipseCase* eclCase);
    RifWellRftAddress(SourceType sourceType, RimWellLogFile* wellLogFile = nullptr);

    SourceType   sourceType() const;
    RimEclipseCase* eclCase() const;
    RimWellLogFile* wellLogFile() const;

    static QString sourceTypeUiText(SourceType sourceType);

    friend QTextStream& operator >> (QTextStream& str, RifWellRftAddress& addr);
    friend bool operator<(const RifWellRftAddress& addr1, const RifWellRftAddress& addr2);

private:
    SourceType                      m_sourceType;
    caf::PdmPointer<RimEclipseCase> m_eclCase;
    caf::PdmPointer<RimWellLogFile> m_wellLogFile;
};

bool operator==(const RifWellRftAddress& addr1, const RifWellRftAddress& addr2);
QTextStream& operator <<(QTextStream& str, const RifWellRftAddress& addr);
QTextStream& operator >> (QTextStream& str, RifWellRftAddress& addr);
bool operator<(const RifWellRftAddress& addr1, const RifWellRftAddress& addr2);