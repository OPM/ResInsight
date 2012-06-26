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

#include "RIStdInclude.h"


#include "RimScriptCollection.h"
#include "cafPdmField.h"
#include "cafUtils.h"
#include "RIMainWindow.h"
#include "RimUiTreeModelPdm.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT(RimScriptCollection, "ScriptLocation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimScriptCollection::RimScriptCollection()
{
    CAF_PDM_InitObject("ScriptLocation", ":/Folder.png", "", "");

    CAF_PDM_InitFieldNoDefault(&directory, "ScriptDirectory", "Dir",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&calcScripts, "CalcScripts", "",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&subDirectories, "SubDirectories", "",  "", "", "");

    directory.setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimScriptCollection::~RimScriptCollection()
{
   calcScripts.deleteAllChildObjects();
   subDirectories.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::readContentFromDisc()
{
    calcScripts.deleteAllChildObjects();

    if (directory().isEmpty())
    {
        for (size_t i = 0; i < subDirectories.size(); ++i)
        {
            if (subDirectories[i]) subDirectories[i]->readContentFromDisc();
        }
        return;
    }

    // Build a list of all scripts in the specified directory
    {
        QString filter = "*.m";
        QStringList fileList = caf::Utils::getFilesInDirectory(directory, filter, true);

        int i;
        for (i = 0; i < fileList.size(); i++)
        {
            QString fileName = fileList.at(i);

            QFileInfo fi(fileName);
            if (fi.exists())
            {
                RimCalcScript* calcScript = new RimCalcScript;
                calcScript->absolutePath = fileName;
                calcScript->setUiName(fi.baseName());
                calcScript->readContentFromFile();

                calcScripts.push_back(calcScript);
            }
        }
    }

    // Add subfolders
    {
        QDir dir(directory);
        QFileInfoList fileInfoList = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
        subDirectories.deleteAllChildObjects();

        QStringList retFileNames;

        QListIterator<QFileInfo> it(fileInfoList);
        while (it.hasNext())
        {
            QFileInfo fi = it.next();

            RimScriptCollection* scriptLocation = new RimScriptCollection;
            scriptLocation->directory = fi.absoluteFilePath();
            scriptLocation->setUiName(fi.baseName());
            scriptLocation->readContentFromDisc();
        
            subDirectories.push_back(scriptLocation);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::pathsAndSubPaths(QStringList& pathList)
{
    if (!this->directory().isEmpty())
    {
        pathList.append(this->directory());
    }

    for (size_t i= 0; i < this->subDirectories.size(); ++i)
    {
        if (this->subDirectories[i]) this->subDirectories[i]->pathsAndSubPaths(pathList);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimScriptCollection * RimScriptCollection::findScriptCollection(const QString& path)
{
    if (!this->directory().isEmpty())
    {
        QFileInfo otherPath(path);
        QFileInfo thisPath(directory());
        if (otherPath == thisPath) return this;
    }

    for (size_t i = 0; i < this->subDirectories.size(); ++i)
    {
         RimScriptCollection* foundColl = NULL;
         if (this->subDirectories[i]) foundColl = this->subDirectories[i]->findScriptCollection(path);
         if (foundColl) return foundColl;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::fieldChangedByUi(const caf::PdmFieldHandle *changedField, const QVariant &oldValue, const QVariant &newValue)
{
    if (&directory == changedField)
    {
        QFileInfo fi(directory);
        this->setUiName(fi.baseName());
        this->readContentFromDisc();
        RimUiTreeModelPdm* treeModel = RIMainWindow::instance()->uiPdmModel();
        if (treeModel) treeModel->rebuildUiSubTree(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &directory)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = static_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        myAttr->m_selectDirectory = true;
    }
}
