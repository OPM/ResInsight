/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include <QString>

#include "RimWellPath.h"

class RivWellPathCollectionPartMgr;
class RimWellPathAsciiFileReader;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWellPathCollection();
    virtual ~RimWellPathCollection();

    void                                setProject(RimProject* project);

    enum WellVisibilityType
    {
        FORCE_ALL_OFF,
        ALL_ON,
        FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimWellPathCollection::WellVisibilityType> WellVisibilityEnum;

    caf::PdmField<bool>                 showWellPathLabel;
    caf::PdmField<cvf::Color3f>         wellPathLabelColor;

    caf::PdmField<WellVisibilityEnum>   wellPathVisibility;
    caf::PdmField<double>               wellPathRadiusScaleFactor;
    caf::PdmField<int>                  wellPathCrossSectionVertexCount;
    caf::PdmField<bool>                 wellPathClip;
    caf::PdmField<int>                  wellPathClipZDistance;

    caf::PdmPointersField<RimWellPath*> wellPaths;

   
    RivWellPathCollectionPartMgr*       wellPathCollectionPartMgr() { return m_wellPathCollectionPartManager.p(); }

    void                                readWellPathFiles();
    void                                addWellPaths(QStringList filePaths);
    RimWellPathAsciiFileReader*         asciiFileReader() {return m_asciiFileReader;}

    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

private:
    caf::PdmPointer<RimProject>         m_project;
    cvf::ref<RivWellPathCollectionPartMgr> m_wellPathCollectionPartManager;

    RimWellPathAsciiFileReader*         m_asciiFileReader;
};


class RimWellPathAsciiFileReader
{
public:
    struct WellData
    {
        QString                 m_name;
        cvf::ref<RigWellPath>   m_wellPathGeometry;
    };

    WellData readWellData(QString filePath, int indexInFile);
    size_t   wellDataCount(QString filePath);

private:

    void readAllWellData(QString filePath);

    std::map<QString, std::vector<WellData> > m_fileNameToWellDataGroupMap;
};
