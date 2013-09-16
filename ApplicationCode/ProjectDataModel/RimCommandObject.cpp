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

#include "RimCommandObject.h"
#include "RiaApplication.h"
#include "RimCalcScript.h"
#include "RimProject.h"

#include "cafPdmUiTextEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmDocument.h"

#include <QFile>
#include "RimStatisticsCase.h"

// Included due to template use in pdm fields
#include "RimReservoirView.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimOilField.h"
#include "RimScriptCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimAnalysisModels.h"
#include "RimWellPathCollection.h"
#include "RimCaseCollection.h"




CAF_PDM_SOURCE_INIT(RimCommandObject, "RimCommandObject");
CAF_PDM_SOURCE_INIT(RimCommandExecuteScript, "RimCommandExecuteScript");
CAF_PDM_SOURCE_INIT(RimCommandIssueFieldChanged, "RimCommandIssueFieldChanged");

//------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandObject::RimCommandObject()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandObject::~RimCommandObject()
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandExecuteScript::RimCommandExecuteScript()
{
    CAF_PDM_InitFieldNoDefault(&name,       "Name",      "Name", "", "", "");

    CAF_PDM_InitField(&scriptText, "ScriptText",  QString(), "ScriptText", "", "" ,"");
    scriptText.setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

    CAF_PDM_InitField(&isEnabled,         "IsEnabled",      true, "Enabled ", "", "", "");
    
    
    CAF_PDM_InitField(&execute,         "Execute",      true, "Execute", "", "", "");
    execute.setIOWritable(false);
    execute.setIOReadable(false);
    execute.setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    execute.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandExecuteScript::~RimCommandExecuteScript()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandExecuteScript::redo()
{
    if (!isEnabled) return;

    RiaApplication* app = RiaApplication::instance();
    QString octavePath = app->octavePath();
    if (!octavePath.isEmpty())
    {
        // http://www.gnu.org/software/octave/doc/interpreter/Command-Line-Options.html#Command-Line-Options

        QStringList arguments;
        arguments.append("--path");
        arguments << QApplication::applicationDirPath();

        arguments.append("-q");
        
        arguments.append("--eval");
        arguments << this->scriptText();

        RiaApplication::instance()->launchProcess(octavePath, arguments);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandExecuteScript::undo()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandExecuteScript::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTextEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTextEditorAttribute*>(attribute);
    if (myAttr)
    {
        myAttr->showSaveButton = true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCommandExecuteScript::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandExecuteScript::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&execute == changedField)
    {
        RiaApplication* app = RiaApplication::instance();
        app->addCommandObject(this);
        app->executeCommandObjects();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCommandExecuteScript::isAsyncronous()
{
    return true;
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandFactory::createCommandObjects(const caf::PdmObjectGroup& selectedObjects, std::vector<RimCommandObject*>* commandObjects)
{
    for (size_t i = 0; i < selectedObjects.objects.size(); i++)
    {
        caf::PdmObject* pdmObject = selectedObjects.objects[i];

        if (dynamic_cast<RimCalcScript*>(pdmObject))
        {
            RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(pdmObject);

            QFile file(calcScript->absolutePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                QByteArray byteArray = file.readAll();
                QString scriptText(byteArray);

                RimCommandExecuteScript* command = new RimCommandExecuteScript;
                command->scriptText = scriptText;

                commandObjects->push_back(command);
            }
        }
        else if (dynamic_cast<RimStatisticsCase*>(pdmObject))
        {
            RimStatisticsCase* statisticsCase = dynamic_cast<RimStatisticsCase*>(pdmObject);

            RimCommandIssueFieldChanged* command = new RimCommandIssueFieldChanged;
            command->objectName = statisticsCase->uiName();
            command->fieldName = statisticsCase->m_calculateEditCommand.keyword();
            command->fieldValueToApply = "true";

            commandObjects->push_back(command);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandIssueFieldChanged::RimCommandIssueFieldChanged()
{
    CAF_PDM_InitFieldNoDefault(&commandName,       "CommandName",      "CommandName", "", "", "");

    CAF_PDM_InitField(&objectName, "ObjectName",  QString(), "ObjectName", "", "" ,"");
    CAF_PDM_InitField(&fieldName, "FieldName",  QString(), "FieldName", "", "" ,"");
    CAF_PDM_InitField(&fieldValueToApply, "FieldValueToApply",  QString(), "FieldValueToApply", "", "" ,"");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCommandIssueFieldChanged::~RimCommandIssueFieldChanged()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandIssueFieldChanged::redo()
{
    RiaApplication* app = RiaApplication::instance();
    PdmObject* project = app->project();

    caf::PdmObject* pdmObject = findObjectByName(project, this->objectName);

    if (pdmObject)
    {
        caf::PdmFieldHandle* fieldHandle = findFieldByKeyword(pdmObject, this->fieldName);

        if (fieldHandle)
        {
            QVariant variantValue(this->fieldValueToApply);
            fieldHandle->setValueFromUi(variantValue);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandIssueFieldChanged::undo()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCommandIssueFieldChanged::userDescriptionField()
{
    return &commandName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCommandIssueFieldChanged::childObjects(caf::PdmObject* pdmObject, std::vector<caf::PdmObject*>& children)
{
    if (!pdmObject) return;

    std::vector<caf::PdmFieldHandle*> fields;
    pdmObject->fields(fields);

    size_t fIdx;
    for (fIdx = 0; fIdx < fields.size(); ++fIdx)
    {
        if (fields[fIdx]) fields[fIdx]->childObjects(&children);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimCommandIssueFieldChanged::findObjectByName(caf::PdmObject* pdmObject, const QString& objectName)
{
    std::vector<caf::PdmFieldHandle*> fields;
    pdmObject->fields(fields);

    if (pdmObject->uiName() == objectName)
    {
        return pdmObject;
    }

    
    for (size_t fIdx = 0; fIdx < fields.size(); fIdx++)
    {
        if (fields[fIdx])
        {
            std::vector<caf::PdmObject*> children;
            fields[fIdx]->childObjects(&children);

            for (size_t cIdx = 0; cIdx < children.size(); cIdx++)
            {
                PdmObject* candidateObj = findObjectByName(children[cIdx], objectName);
                if (candidateObj)
                {
                    return candidateObj;
                }
            }
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCommandIssueFieldChanged::findFieldByKeyword(caf::PdmObject* pdmObject, const QString& keywordName)
{
    std::vector<caf::PdmFieldHandle*> fields;
    pdmObject->fields(fields);

    for (size_t fIdx = 0; fIdx < fields.size(); fIdx++)
    {
        if (fields[fIdx] && fields[fIdx]->keyword() == keywordName)
        {
            return fields[fIdx];
        }
    }

    return NULL;
}


