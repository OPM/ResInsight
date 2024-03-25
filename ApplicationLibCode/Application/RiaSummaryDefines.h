/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include <QString>

namespace RiaDefines
{
enum class FileType
{
    SMSPEC,
    REVEAL_SUMMARY,
    STIMPLAN_SUMMARY
};

enum class HorizontalAxisType
{
    TIME,
    SUMMARY_VECTOR
};

QString summaryField();
QString summaryAquifer();
QString summaryNetwork();
QString summaryMisc();
QString summaryRegion();
QString summaryRegion2Region();
QString summaryWell();
QString summaryWellGroup();
QString summaryWellSegment();
QString summaryCompletion();
QString summarySegment();
QString summaryBlock();
QString summaryLgrCompletion();
QString summaryLgrWell();
QString summaryLgrBlock();
QString summaryCalculated();

QString summaryRealizationNumber();

}; // namespace RiaDefines
