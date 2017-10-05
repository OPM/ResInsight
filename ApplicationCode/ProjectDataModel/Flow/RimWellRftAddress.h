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

#include <QPointer>
#include <QDate>
#include <QMetaType>

class RimEclipseResultCase;
class RimEclipseWell;
class RimFlowDiagSolution;
class RimTotalWellAllocationPlot;
class RimWellLogPlot;
class RiuWellRftPlot;
class RimWellLogTrack;
class RimTofAccumulatedPhaseFractionsPlot;
class RigSingleWellResultsData;
class RimWellLogFileChannel;
class RimWellLogFile;



//==================================================================================================
///  
///  
//==================================================================================================
enum class RftSourceType
{
    NONE,
    RFT,
    GRID,
    OBSERVED
};


class RimWellRftAddress
{
public:
    RimWellRftAddress() : m_sourceType(RftSourceType::NONE), m_caseId(-1)
    {
    }

    RimWellRftAddress(RftSourceType sourceType, int caseId = -1);

    RftSourceType   sourceType() const;
    int             caseId() const;

    QString uiText() const;
    static QString sourceTypeUiText(RftSourceType sourceType);

    friend QTextStream& operator >> (QTextStream& str, RimWellRftAddress& addr);
    friend bool operator<(const RimWellRftAddress& addr1, const RimWellRftAddress& addr2);

private:
    RftSourceType   m_sourceType;
    int             m_caseId;
};

Q_DECLARE_METATYPE(RimWellRftAddress);

bool operator==(const RimWellRftAddress& addr1, const RimWellRftAddress& addr2);
QTextStream& operator <<(QTextStream& str, const RimWellRftAddress& addr);
QTextStream& operator >> (QTextStream& str, RimWellRftAddress& addr);
bool operator<(const RimWellRftAddress& addr1, const RimWellRftAddress& addr2);