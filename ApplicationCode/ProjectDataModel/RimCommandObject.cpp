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

#include "cafPdmUiTextEditor.h"
#include "cafPdmDocument.h"

#include <QFile>

CAF_PDM_SOURCE_INIT(RimCommandObject, "RimCommandObject");
CAF_PDM_SOURCE_INIT(RimCommandExecuteScript, "RimCommandExecuteScript");

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
    CAF_PDM_InitField(&scriptText, "ScriptText",  QString(), "ScriptText", "", "" ,"");
    scriptText.setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

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
    }
}
