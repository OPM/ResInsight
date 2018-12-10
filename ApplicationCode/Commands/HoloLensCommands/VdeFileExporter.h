/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "VdeVizDataExtractor.h"

#include <QString>

class VdeArrayDataPacket;
class VdePacketDirectory;

class RimGridView;



//==================================================================================================
//
//
//
//==================================================================================================
class VdeFileExporter
{
public:
    VdeFileExporter(QString absOutputFolder);

    bool exportToFile(const QString& modelMetaJsonStr, const VdePacketDirectory& packetDirectory, const std::vector<int>& packetIdsToExport);
    bool exportViewContents(const RimGridView& view);

private:
    static bool writeModelMetaJsonFile(const QString& modelMetaJsonStr, QString fileName);
    bool        writeDataPacketToFile(int arrayId, const VdeArrayDataPacket& packet) const;

private:
    QString  m_absOutputFolder;

};
