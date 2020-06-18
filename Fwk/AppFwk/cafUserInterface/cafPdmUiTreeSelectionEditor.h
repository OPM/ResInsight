//##################################################################################################
//
//   Custom Visualization Core library
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

#include "cafPdmUiFieldEditorHandle.h"

#include <QAbstractItemModel>

class QLabel;
class QTreeView;
class QAbstractItemModel;
class QCheckBox;
class QLineEdit;
class QSortFilterProxyModel;
class QModelIndex;
class QItemSelection;

namespace caf
{
class PdmUiTreeSelectionQModel;

//==================================================================================================
///
//==================================================================================================
class PdmUiTreeSelectionEditorAttribute : public PdmUiEditorAttribute
{
public:
    bool showTextFilter;
    bool showToggleAllCheckbox;
    bool singleSelectionMode;
    bool setCurrentIndexWhenItemIsChecked;

    /// currentIndexFieldHandle is used to communicate the value of current item in the tree view
    /// This is useful when displaying a list of appEnums, and a dependent view is displaying content based on
    /// the current item in the tree view
    /// Make sure the type of the receiving field is of the same type as the field used in PdmUiTreeSelectionEditor
    caf::PdmFieldHandle* currentIndexFieldHandle;

public:
    PdmUiTreeSelectionEditorAttribute()
    {
        showTextFilter                   = true;
        showToggleAllCheckbox            = true;
        singleSelectionMode              = false;
        setCurrentIndexWhenItemIsChecked = false;

        currentIndexFieldHandle = nullptr;
    }
};

//==================================================================================================
///
//==================================================================================================
class PdmUiTreeSelectionEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTreeSelectionEditor();
    ~PdmUiTreeSelectionEditor() override;

protected:
    void     configureAndUpdateUi( const QString& uiConfigName ) override;
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    QMargins calculateLabelContentMargins() const override;
    bool     isMultiRowEditor() const override;

private slots:
    void customMenuRequested( const QPoint& pos );

    void slotSetSelectedOn();
    void slotSetSelectedOff();
    void slotSetSubItemsOn();
    void slotSetSubItemsOff();

    void slotToggleAll();

    void slotTextFilterChanged();

    void slotClicked( const QModelIndex& index );

    void slotScrollToFirstCheckedItem();

private:
    void currentChanged( const QModelIndex& current );

    void setCheckedStateOfSelected( bool checked );
    void setCheckedStateForSubItemsOfSelected( bool checked );
    void checkAllItems();
    void unCheckAllItems();

    QModelIndexList allVisibleSourceModelIndices() const;
    void recursiveAppendVisibleSourceModelIndices( const QModelIndex& parent, QModelIndexList* sourceModelIndices ) const;

private:
    QPointer<QTreeView>       m_treeView;
    QPointer<QShortenedLabel> m_label;
    QPointer<QCheckBox>       m_toggleAllCheckBox;
    QPointer<QLineEdit>       m_textFilterLineEdit;

    PdmUiTreeSelectionQModel* m_model;
    QSortFilterProxyModel*    m_proxyModel;

    PdmUiTreeSelectionEditorAttribute m_attributes;
};

} // end namespace caf
