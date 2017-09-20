/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

class QString;
class RifSummaryReaderInterface;

class RigSummaryCaseData: public cvf::Object
{
public:
    explicit RigSummaryCaseData(const QString& summaryHeaderFileName );
    ~RigSummaryCaseData();

    void openOrReloadCase(const QString& summaryHeaderFileName);

    RifSummaryReaderInterface* summaryReader();

private:
    cvf::ref<RifSummaryReaderInterface> m_summaryFileReader;
};
