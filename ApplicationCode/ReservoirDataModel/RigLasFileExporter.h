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
#include "cvfCollection.h"

#include <QString>

#include <vector>

class RimWellLogCurve;
class RigWellLogCurveData;
class SingleLasFileMetaData;

class RigLasFileExporter
{
public:
    RigLasFileExporter(const std::vector<RimWellLogCurve*>& curves);

    void setResamplingInterval(double interval);

    bool writeToFolder(const QString& exportFolder);

private:
    std::vector<SingleLasFileMetaData>  createLasFileDescriptions(const std::vector<RimWellLogCurve*>& curves);
    void                                appendLasFileDescriptions(const std::vector<RimWellLogCurve*>& curves, 
                                                                  std::vector<SingleLasFileMetaData>* lasFileDescriptions);
    QString                             caseNameFromCurve(RimWellLogCurve* curve);

private:
    std::vector<RimWellLogCurve*> m_curves;

    bool                                 m_isResampleActive;
    double                               m_resamplingInterval;
    cvf::Collection<RigWellLogCurveData> m_resampledCurveDatas;
};
