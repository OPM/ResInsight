/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaStatisticsTools.h"

#include "RifEclipseSummaryAddress.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString RiaStatisticsTools::replacePercentileByPValueText(const QString& percentile)
{
    QString result = percentile;

    if (result == ENSEMBLE_STAT_P10_QUANTITY_NAME)
    {
        result = ENSEMBLE_STAT_P90_QUANTITY_NAME;
    }
    else if (result == ENSEMBLE_STAT_P90_QUANTITY_NAME)
    {
        result = ENSEMBLE_STAT_P10_QUANTITY_NAME;
    }
    else if (percentile.contains(QString("%1:").arg(ENSEMBLE_STAT_P10_QUANTITY_NAME)))
    {
        result.replace(ENSEMBLE_STAT_P10_QUANTITY_NAME, ENSEMBLE_STAT_P90_QUANTITY_NAME);
    }
    else if (percentile.contains(QString("%1:").arg(ENSEMBLE_STAT_P90_QUANTITY_NAME)))
    {
        result.replace(ENSEMBLE_STAT_P90_QUANTITY_NAME, ENSEMBLE_STAT_P10_QUANTITY_NAME);
    }
    return result;
}
