//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#pragma once

#include "cafPdmUiObjectEditorHandle.h"

#include <QPointer>
#include <QString>

#include <map>

class QMinimizePanel;
class QGridLayout;

namespace caf {

class PdmUiFieldEditorHandle;
class PdmUiGroup;


//==================================================================================================
/// 
//==================================================================================================
class PdmUiWidgetBasedObjectEditor : public PdmUiObjectEditorHandle
{
    Q_OBJECT
public:
    PdmUiWidgetBasedObjectEditor();
    ~PdmUiWidgetBasedObjectEditor();

protected:
    virtual void            setupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, const QString& uiConfigName) = 0;
    virtual QWidget*        createWidget(QWidget* parent) override;
    
    bool                    isUiGroupExpanded(const PdmUiGroup* uiGroup) const;
    QMinimizePanel*         findOrCreateGroupBox(PdmUiGroup* group, QWidget* parent, const QString& uiConfigName);
    PdmUiFieldEditorHandle* findOrCreateFieldEditor(QWidget* parent, PdmUiFieldHandle* field, const QString& uiConfigName);

    static QGridLayout*     groupBoxLayout(QMinimizePanel* groupBox);
    
    void                    recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName);

private slots:
    void                    groupBoxExpandedStateToggled(bool isExpanded);

private:
    virtual void            cleanupBeforeSettingPdmObject() override;
    virtual void            configureAndUpdateUi(const QString& uiConfigName) override;
    
    static void             recursiveVerifyUniqueNames(const std::vector<PdmUiItem*>& uiItems, const QString& uiConfigName, std::set<QString>* fieldKeywordNames, std::set<QString>* groupNames);

private:
    QPointer<QWidget>                                   m_mainWidget;

    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*>  m_fieldViews; 
    std::map<QString, QPointer<QMinimizePanel> >        m_groupBoxes;
    std::map<QString, QPointer<QMinimizePanel> >        m_newGroupBoxes; ///< used temporarily to store the new(complete) set of group boxes
    std::map<QString, std::map<QString, bool> >         m_objectKeywordGroupUiNameExpandedState; 
};



} // end namespace caf
