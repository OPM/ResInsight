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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"

#include "cvfObject.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCommandObject();
    ~RimCommandObject() override;

    virtual bool isAsyncronous() { return false; };

    virtual void redo() {};
    virtual void undo() {};
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandExecuteScript : public RimCommandObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCommandExecuteScript();
    ~RimCommandExecuteScript() override;

    caf::PdmField<QString>  name;
    caf::PdmField<bool>     isEnabled;
    caf::PdmField<bool>     execute;
    caf::PdmField<QString>  scriptText;

    void redo() override;
    void undo() override;

    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    bool isAsyncronous() override;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandIssueFieldChanged : public RimCommandObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCommandIssueFieldChanged();
    ~RimCommandIssueFieldChanged() override;

    caf::PdmField<QString>  commandName;
    caf::PdmField<QString>  objectName;
    caf::PdmField<QString>  fieldName;
    caf::PdmField<QString>  fieldValueToApply;

    void redo() override;
    void undo() override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void childObjects(caf::PdmObject* pdmObject, std::vector<caf::PdmObjectHandle*>& children);
    caf::PdmObjectHandle* findObjectByName(caf::PdmObjectHandle* root, const QString& name);
    caf::PdmFieldHandle* findFieldByKeyword(caf::PdmObjectHandle* pdmObject, const QString& fieldName);

};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimCommandFactory
{
public:
    static void createCommandObjects(const caf::PdmObjectGroup& selectedObjects, std::vector<RimCommandObject*>* commandObjects);
};

