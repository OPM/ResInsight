/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimStimPlanFractureTemplate.h"

#include "RigTesselatorTools.h"

#include "RimFracture.h"
#include "RimProject.h"
#include "RigStimPlanFractureDefinition.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"
#include "cafPdmUiFilePathEditor.h"
#include <QMessageBox>
#include <QFileInfo>



CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_StimPlanFileName, "StimPlanFileName", QString(""), "File Name", "", "", "");

    m_StimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::~RimStimPlanFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

    if (&m_StimPlanFileName == changedField)
    {
        updateUiTreeName();
        QString errorMessage;
        readStimPlanXMLFile(&errorMessage);
        if (!errorMessage.isEmpty())
        {
            QMessageBox::warning(nullptr, "StimPlanFile", errorMessage);
        }
    }


// 
//     if (changedField == &halfLength || changedField == &height || changedField == &azimuthAngle || changedField == &perforationLength || changedField == &orientation)
//     {
//         //Changes to one of these parameters should change all fractures with this fracture template attached. 
//         RimProject* proj;
//         this->firstAncestorOrThisOfType(proj);
//         if (proj)
//         {
//             //Regenerate geometry
//             std::vector<RimFracture*> fractures;
//             proj->descendantsIncludingThisOfType(fractures);
// 
//             for (RimFracture* fracture : fractures)
//             {
//                 if (fracture->attachedFractureDefinition() == this)
//                 {
//                     if (changedField == &halfLength || changedField == &height)
//                     {
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &azimuthAngle && (abs(oldValue.toDouble() - fracture->azimuth()) < 1e-5))
//                     {
//                         fracture->azimuth = azimuthAngle;
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &orientation)
//                     {
//                         fracture->setAzimuth();
//                         if (orientation() == FracOrientationEnum::AZIMUTH)
//                         {
//                             fracture->azimuth = azimuthAngle;
//                         }
// 
//                         fracture->setRecomputeGeometryFlag();
//                     }
// 
//                     if (changedField == &perforationLength && (abs(oldValue.toDouble() - fracture->perforationLength()) < 1e-5))
//                     {
//                         fracture->perforationLength = perforationLength;
//                     }
// 
//                 }
//             }
// 
//             proj->createDisplayModelAndRedrawAllViews();
//         }
//     }
// 
// 



}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::updateUiTreeName()
{
    this->uiCapability()->setUiName(fileNameWoPath());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setFileName(const QString& fileName)
{
    m_StimPlanFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RimStimPlanFractureTemplate::fileName()
{
    return m_StimPlanFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::fileNameWoPath()
{
    QFileInfo stimplanfileFileInfo(m_StimPlanFileName());
    return stimplanfileFileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::readStimPlanXMLFile(QString * errorMessage)
{
//     QFile dataFile(m_formationNamesFileName());
// 
//     if (!dataFile.open(QFile::ReadOnly))
//     {
//         if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_formationNamesFileName()) + "\n";
//         return;
//     }
// 
//     m_formationNamesData = new RigFormationNames;
// 
//     QTextStream stream(&dataFile);
//     int lineNumber = 1;
//     while (!stream.atEnd())
//     {
// 
//     }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* polygonIndices)
{
//     RigEllipsisTesselator tesselator(20);
// 
//     float a = halfLength;
//     float b = height / 2.0f;
// 
//     tesselator.tesselateEllipsis(a, b, polygonIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon()
{
     std::vector<cvf::Vec3f> polygon;
// 
//     std::vector<cvf::Vec3f> nodeCoords;
//     std::vector<cvf::uint>  polygonIndices;
// 
//     fractureGeometry(&nodeCoords, &polygonIndices);
// 
//     for (size_t i = 1; i < nodeCoords.size(); i++)
//     {
//         polygon.push_back(nodeCoords[i]);
//     }
// 
     return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

}

