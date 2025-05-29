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

enum class EnsembleGroupingMode
{
    FMU_FOLDER_STRUCTURE,
    EVEREST_FOLDER_STRUCTURE,
    RESINSIGHT_OPMFLOW_STRUCTURE,
    NONE
};

enum class HorizontalAxisType
{
    TIME,
    SUMMARY_VECTOR
};

enum class SummaryCurveTypeMode
{
    AUTO,
    CUSTOM
};

QString summaryField();
QString summaryAquifer();
QString summaryNetwork();
QString summaryMisc();
QString summaryRegion();
QString summaryRegion2Region();
QString summaryWell();
QString summaryWellCompletion();
QString summaryWellGroup();
QString summaryWellSegment();
QString summaryWellConnection();
QString summarySegment();
QString summaryBlock();
QString summaryLgrConnection();
QString summaryLgrWell();
QString summaryLgrBlock();
QString summaryCalculated();

QString summaryRealizationNumber();

QString key1VariableName();
QString key2VariableName();

}; // namespace RiaDefines
