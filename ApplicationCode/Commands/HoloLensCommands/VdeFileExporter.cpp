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

#include "VdeFileExporter.h"
#include "VdeArrayDataPacket.h"
#include "VdePacketDirectory.h"

#include "cvfTrace.h"

#include <QDir>
#include <QFile>



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VdeFileExporter::VdeFileExporter(QString absOutputFolder)
:   m_absOutputFolder(absOutputFolder)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::exportToFile(const QString& modelMetaJsonStr, const VdePacketDirectory& packetDirectory, const std::vector<int>& packetIdsToExport)
{
    cvf::Trace::show("Exporting to folder: %s", m_absOutputFolder.toLatin1().constData());

    QString jsonFileName = m_absOutputFolder + "/modelMeta.json";
    if (!writeModelMetaJsonFile(modelMetaJsonStr, jsonFileName))
    {
        cvf::Trace::show("Error writing: %s", jsonFileName.toLatin1().constData());
        return false;
    }

    for (const int packetArrayId : packetIdsToExport)
    {
        const VdeArrayDataPacket* dataPacket = packetDirectory.lookupPacket(packetArrayId);
        if (!dataPacket)
        {
            cvf::Trace::show("Error during export, no data for arrayId %d", packetArrayId);
            return false;
        }

        CVF_ASSERT(packetArrayId == dataPacket->arrayId());
        if (!writeDataPacketToFile(dataPacket->arrayId(), *dataPacket))
        {
            cvf::Trace::show("Error writing packet data to file, arrayId %d", packetArrayId);
            return false;
        }
    }

    cvf::Trace::show("Data exported to folder: %s", m_absOutputFolder.toLatin1().constData());

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::exportViewContents(const RimGridView& view)
{
    QString modelMetaJsonStr;
    std::vector<int> allReferencedArrayIds;
    VdePacketDirectory packetDirectory;

    VdeVizDataExtractor extractor(view);
    extractor.extractViewContents(&modelMetaJsonStr, &allReferencedArrayIds, &packetDirectory);

    if (!exportToFile(modelMetaJsonStr, packetDirectory, allReferencedArrayIds))
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::writeDataPacketToFile(int arrayId, const VdeArrayDataPacket& packet) const
{
    const QString fileName = m_absOutputFolder + QString("/arrayData_%1.bin").arg(arrayId);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    if (file.write(packet.fullPacketRawPtr(), packet.fullPacketSize()) == -1)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool VdeFileExporter::writeModelMetaJsonFile(const QString& modelMetaJsonStr, QString fileName)
{
    const QByteArray jsonByteArr = modelMetaJsonStr.toLatin1();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if (file.write(jsonByteArr) == -1)
    {
        return false;
    }

    return true;
}

