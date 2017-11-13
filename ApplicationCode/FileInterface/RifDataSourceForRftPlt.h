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
class RifDataSourceForRftPlt
{
public:
    enum SourceType
    {
        NONE,
        OBSERVED,
        RFT,
        GRID
    };

    RifDataSourceForRftPlt();
    RifDataSourceForRftPlt(SourceType sourceType, RimEclipseCase* eclCase);
    RifDataSourceForRftPlt(SourceType sourceType, RimWellLogFile* wellLogFile = nullptr);

    SourceType   sourceType() const;
    RimEclipseCase* eclCase() const;
    RimWellLogFile* wellLogFile() const;

    static QString sourceTypeUiText(SourceType sourceType);

    friend QTextStream& operator >> (QTextStream& str, RifDataSourceForRftPlt& addr);
    friend bool operator<(const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2);

private:
    SourceType                      m_sourceType;
    caf::PdmPointer<RimEclipseCase> m_eclCase;
    caf::PdmPointer<RimWellLogFile> m_wellLogFile;
};

bool operator==(const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2);
QTextStream& operator <<(QTextStream& str, const RifDataSourceForRftPlt& addr);
QTextStream& operator >> (QTextStream& str, RifDataSourceForRftPlt& addr);
bool operator<(const RifDataSourceForRftPlt& addr1, const RifDataSourceForRftPlt& addr2);