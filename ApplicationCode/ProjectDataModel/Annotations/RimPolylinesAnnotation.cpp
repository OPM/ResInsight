/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimPolylinesAnnotation.h"

#include "RimAnnotationInViewCollection.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "QFile"
#include "RimAnnotationCollection.h"
#include "QFileInfo"

CAF_PDM_ABSTRACT_SOURCE_INIT(RimPolylinesAnnotation, "RimPolylinesAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::RimPolylinesAnnotation()
{
    CAF_PDM_InitObject("PolylineAnnotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_isActive, "IsActive", true, "Is Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::~RimPolylinesAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPolylinesAnnotation::isActive()
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPolylinesAnnotation::objectToggleField()
{
    return &m_isActive;
}

CAF_PDM_SOURCE_INIT(RimUserDefinedPolylinesAnnotation, "UserDefinedPolylinesAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::RimUserDefinedPolylinesAnnotation()
{
    CAF_PDM_InitObject("PolyLines Annotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitField(&m_points, "Points", {}, "", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimUserDefinedPolylinesAnnotation::~RimUserDefinedPolylinesAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimUserDefinedPolylinesAnnotation::polyLinesData()
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData; 
    std::vector<std::vector<cvf::Vec3d> > lines; 
    lines.push_back(m_points());
    pld->setPolyLines(lines);

    return pld;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUserDefinedPolylinesAnnotation::isEmpty()
{
    return m_points().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_points);

    auto appearanceGroup = uiOrdering.addNewGroup("Line Appearance");
    appearance()->uiOrdering(uiConfigName, *appearanceGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUserDefinedPolylinesAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                         const QVariant&            oldValue,
                                                         const QVariant&            newValue)
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);

    annColl->scheduleRedrawOfRelevantViews();
}


#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RimPolylinesFromFileAnnotation, "PolylinesFromFileAnnotation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylinesFromFileAnnotation::RimPolylinesFromFileAnnotation()
{
    CAF_PDM_InitObject("PolyLines Annotation", ":/WellCollection.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_polyLinesFileName, "PolyLineFilePath", "File Path", "", "", "");
    m_polyLinesFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
    CAF_PDM_InitField(&m_userDescription, "PolyLineDescription", QString(""), "Name", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPolylinesFromFileAnnotation::~RimPolylinesFromFileAnnotation()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylinesFromFileAnnotation::setFileName(const QString& fileName)
{
    m_polyLinesFileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimPolylinesFromFileAnnotation::fileName() const
{
    return m_polyLinesFileName().path();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylinesFromFileAnnotation::readPolyLinesFile(QString * errorMessage)
{
    QFile dataFile(m_polyLinesFileName().path());

    if (!dataFile.open(QFile::ReadOnly)) 
    { 
        if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_polyLinesFileName().path()) + "\n"; 
        return;
    }

    m_polyLinesData = new RigPolyLinesData;
    
    std::vector< std::vector< cvf::Vec3d > > polylines(1);

    QTextStream stream(&dataFile);
    int lineNumber = 1;
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QStringList commentLineSegs = line.split("#", QString::KeepEmptyParts);
        if(commentLineSegs.size() == 0) continue; // Empty line


        QStringList lineSegs = commentLineSegs[0].split(QRegExp("\\s+"), QString::SkipEmptyParts);

        if(lineSegs.size() == 0) continue; // No data
        if(lineSegs.size() != 3) 
        { 
            if (errorMessage) (*errorMessage) += "Unexpected number of words on line: " + QString::number(lineNumber) + "\n";
            continue;
        }

        if (lineSegs.size() == 3) // Normal case
        {
            bool isNumberParsingOk = true;
            bool isOk = true;
            double x = lineSegs[0].toDouble(&isOk); isNumberParsingOk &= isOk;
            double y = lineSegs[1].toDouble(&isOk); isNumberParsingOk &= isOk;
            double z = lineSegs[2].toDouble(&isOk); isNumberParsingOk &= isOk;

            if (!isNumberParsingOk)
            {
                if (errorMessage) (*errorMessage) += "Could not read the point at line: " + QString::number(lineNumber) + "\n";
                continue;
            }

            if (x == 999.0 && y == 999.0 && z == 999.0) // New PolyLine
            {
                polylines.push_back(std::vector<cvf::Vec3d>());
                continue;
            }

            cvf::Vec3d point(x, y, -z);
            polylines.back().push_back(point);

        }

        ++lineNumber;
    }  

    if ( polylines.back().empty() )
    {
        polylines.pop_back();
    }

    m_polyLinesData->setPolyLines(polylines);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimPolylinesFromFileAnnotation::isEmpty()
{
    if (m_polyLinesData.isNull()) return true; 

    for (const std::vector<cvf::Vec3d> & line :m_polyLinesData->polyLines())
    {
        if (!line.empty()) return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimPolylinesFromFileAnnotation::setDescriptionFromFileName()
{
    QFileInfo fileInfo(m_polyLinesFileName().path());
    m_userDescription =  fileInfo.fileName();

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylinesFromFileAnnotation::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_polyLinesFileName);
    auto appearanceGroup = uiOrdering.addNewGroup("Line Appearance");
    appearance()->uiOrdering(uiConfigName, *appearanceGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylinesFromFileAnnotation::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue)
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(annColl);
    
    annColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPolylinesFromFileAnnotation::userDescriptionField()
{
    return &m_userDescription;
}
