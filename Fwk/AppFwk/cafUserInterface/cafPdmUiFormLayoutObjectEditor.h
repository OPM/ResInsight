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

#include "cafPdmUiOrdering.h"
#include "cafPdmUiWidgetObjectEditorHandle.h"

#include <QPointer>
#include <QString>

#include <map>

class QMinimizePanel;
class QGridLayout;
class QWidget;

namespace caf
{
class PdmUiFieldEditorHandle;
class PdmUiGroup;
class PdmUiOrdering;

//==================================================================================================
///
//==================================================================================================
class PdmUiFormLayoutObjectEditor : public PdmUiWidgetObjectEditorHandle
{
    Q_OBJECT
public:
    PdmUiFormLayoutObjectEditor();
    ~PdmUiFormLayoutObjectEditor() override;

protected:
    /// When overriding this function, use findOrCreateGroupBox() or findOrCreateFieldEditor() for detailed control
    /// Use recursivelyConfigureAndUpdateUiItemsInGridLayoutColumn() for automatic layout of group and field widgets
    virtual void recursivelyConfigureAndUpdateTopLevelUiOrdering( const PdmUiOrdering& topLevelUiOrdering,
                                                                  const QString&       uiConfigName ) = 0;

    bool recursivelyConfigureAndUpdateUiOrderingInNewGridLayout( const PdmUiOrdering& uiOrdering,
                                                                 QWidget*             containerWidget,
                                                                 const QString&       uiConfigName );
    int  recursivelyConfigureAndUpdateUiOrderingInGridLayout( const PdmUiOrdering& uiOrdering,
                                                              QWidget*             containerWidgetWithGridLayout,
                                                              const QString&       uiConfigName );

    int recursivelyAddGroupToGridLayout( PdmUiItem*     currentItem,
                                         QWidget*       containerWidget,
                                         const QString& uiConfigName,
                                         QGridLayout*   parentLayout,
                                         int            currentRowIndex,
                                         int            currentColumn,
                                         int            itemColumnSpan );

    QMinimizePanel*         findOrCreateGroupBox( QWidget* parent, PdmUiGroup* group, const QString& uiConfigName );
    PdmUiFieldEditorHandle* findOrCreateFieldEditor( QWidget* parent, PdmUiFieldHandle* field, const QString& uiConfigName );

    static void ensureWidgetContainsEmptyGridLayout( QWidget* containerWidget, QMargins contentMargins = QMargins() );

private slots:
    void groupBoxExpandedStateToggled( bool isExpanded );

private:
    bool isUiGroupExpanded( const PdmUiGroup* uiGroup ) const;
    void cleanupBeforeSettingPdmObject() override;
    void configureAndUpdateUi( const QString& uiConfigName ) override;

    static void recursiveVerifyUniqueNames( const std::vector<PdmUiItem*>& uiItems,
                                            const QString&                 uiConfigName,
                                            std::set<QString>*             fieldKeywordNames,
                                            std::set<QString>*             groupNames );

private:
    std::map<PdmFieldHandle*, PdmUiFieldEditorHandle*> m_fieldViews;
    std::set<PdmFieldHandle*> m_usedFields; ///< used temporarily to store the new(complete) set of used fields
    std::map<QString, QPointer<QMinimizePanel>> m_groupBoxes;
    std::map<QString, QPointer<QMinimizePanel>> m_newGroupBoxes; ///< used temporarily to store the new(complete) set of
                                                                 ///< group boxes
    std::map<QString, std::map<QString, bool>> m_objectKeywordGroupUiNameExpandedState;
};

} // end namespace caf
