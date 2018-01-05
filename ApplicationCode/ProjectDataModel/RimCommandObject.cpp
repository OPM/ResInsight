/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include "RimEclipseStatisticsCase.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmValueField.h"

#include <QFile>


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

    CAF_PDM_InitField(&scriptText, "ScriptText",  QString(), "Script Text", "", "" ,"");
    scriptText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

    CAF_PDM_InitField(&isEnabled,         "IsEnabled",      true, "Enabled ", "", "", "");
    
    
    CAF_PDM_InitField(&execute,         "Execute",      true, "Execute", "", "", "");
    execute.xmlCapability()->setIOWritable(false);
    execute.xmlCapability()->setIOReadable(false);
    execute.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    execute.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
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
        QStringList arguments = app->octaveArguments();
        
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
        caf::PdmObjectHandle* pdmObject = selectedObjects.objects[i];

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
        else if (dynamic_cast<RimEclipseStatisticsCase*>(pdmObject))
        {
            RimEclipseStatisticsCase* statisticsCase = dynamic_cast<RimEclipseStatisticsCase*>(pdmObject);

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

    caf::PdmObjectHandle* pdmObject = findObjectByName(project, this->objectName);

    if (pdmObject)
    {
        caf::PdmFieldHandle* fieldHandle = findFieldByKeyword(pdmObject, this->fieldName);

        if (fieldHandle && fieldHandle->uiCapability())
        {
            caf::PdmValueField* valueField = dynamic_cast<caf::PdmValueField*>(fieldHandle);
            CVF_ASSERT(valueField);

            QVariant oldValue = valueField->toQVariant();
            QVariant newValue(this->fieldValueToApply);

            valueField->setFromQVariant(newValue);

            caf::PdmUiFieldHandle* uiFieldHandle = fieldHandle->uiCapability();
            uiFieldHandle->notifyFieldChanged(oldValue, newValue);
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
void RimCommandIssueFieldChanged::childObjects(caf::PdmObject* pdmObject, std::vector<caf::PdmObjectHandle*>& children)
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
caf::PdmObjectHandle* RimCommandIssueFieldChanged::findObjectByName(caf::PdmObjectHandle* pdmObject, const QString& objectName)
{
    std::vector<caf::PdmFieldHandle*> fields;
    pdmObject->fields(fields);

    caf::PdmUiObjectHandle* uiObjectHandle = uiObj(pdmObject);

    if (uiObjectHandle && uiObjectHandle->uiName() == objectName)
    {
        return pdmObject;
    }
    
    for (size_t fIdx = 0; fIdx < fields.size(); fIdx++)
    {
        if (fields[fIdx])
        {
            std::vector<caf::PdmObjectHandle*> children;
            fields[fIdx]->childObjects(&children);

            for (size_t cIdx = 0; cIdx < children.size(); cIdx++)
            {
                PdmObjectHandle* candidateObj = findObjectByName(children[cIdx], objectName);
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
caf::PdmFieldHandle* RimCommandIssueFieldChanged::findFieldByKeyword(caf::PdmObjectHandle* pdmObject, const QString& keywordName)
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


