//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmDocument.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafSelectionChangedReceiver.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QItemSelection;
class QLabel;
class QTableView;

namespace caf 
{

class PdmUiCheckBoxDelegate;
class PdmUiFieldEditorHandle;
class PdmUiItem;
class PdmUiTableViewDelegate;
class PdmUiTableViewQModel;
class PdmChildArrayFieldHandle;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewPushButtonEditorAttribute : public PdmUiEditorAttribute
{
public:
    void    registerPushButtonTextForFieldKeyword(const QString& keyword, const QString& text);

    bool    showPushButtonForFieldKeyword(const QString& keyword) const;
    QString pushButtonText(const QString& keyword) const;

private:
    std::map<QString, QString> m_fieldKeywordAndPushButtonText;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewEditorAttribute : public PdmUiEditorAttribute
{
public:
    enum ResizePolicy
    {
        NO_AUTOMATIC_RESIZE,
        RESIZE_TO_FIT_CONTENT,
        RESIZE_TO_FILL_CONTAINER
    };

    PdmUiTableViewEditorAttribute()
        : tableSelectionLevel(0)
        , rowSelectionLevel(1)
        , enableHeaderText(true)
        , minimumHeight(-1)
        , alwaysEnforceResizePolicy(false)
        , resizePolicy(NO_AUTOMATIC_RESIZE)
    {
        QPalette myPalette;
        baseColor = myPalette.color(QPalette::Active, QPalette::Base);
    }

    int                 selectionLevel;
    int                 tableSelectionLevel;
    int                 rowSelectionLevel;
    bool                enableHeaderText;
    std::vector<int>    columnWidths;
    int                 minimumHeight; ///< Not used if If < 0 
    QColor              baseColor;
    bool                alwaysEnforceResizePolicy;
    ResizePolicy        resizePolicy;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

class PdmUiTableViewEditor : public PdmUiFieldEditorHandle, public SelectionChangedReceiver
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiTableViewEditor();
    ~PdmUiTableViewEditor() override;

    void            enableHeaderText(bool enable);
    void            setTableSelectionLevel(int selectionLevel);
    void            setRowSelectionLevel(int selectionLevel);

    PdmObjectHandle* pdmObjectFromModelIndex(const QModelIndex& mi);
    QTableView*     tableView();

protected:
    QWidget*        createEditorWidget(QWidget * parent) override;
    QWidget*        createLabelWidget(QWidget * parent) override;
    void            configureAndUpdateUi(const QString& uiConfigName) override;

    void            onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;
    bool isMultiRowEditor() const override;

private:
    void            selectedUiItems(const QModelIndexList& modelIndexList, std::vector<PdmUiItem*>& objects);
    bool            isSelectionRoleDefined() const;
    void            updateSelectionManagerFromTableSelection();

    bool            eventFilter(QObject* obj, QEvent* event) override;
    PdmChildArrayFieldHandle* childArrayFieldHandle();

private slots:
    void            slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    friend class FocusEventHandler;

    QPointer<QShortenedLabel> m_tableHeading;
    QPointer<QLabel>          m_tableHeadingIcon;

    QTableView*             m_tableView;
    PdmUiTableViewQModel*   m_tableModelPdm;

    PdmUiTableViewDelegate* m_delegate;
    PdmUiCheckBoxDelegate*  m_checkboxDelegate;

    bool                    m_useDefaultContextMenu;
    int                     m_tableSelectionLevel;
    int                     m_rowSelectionLevel;
    bool                    m_isBlockingSelectionManagerChanged;
    bool                    m_isUpdatingSelectionQModel;

    caf::PdmChildArrayFieldHandle* m_previousFieldHandle;
};



} // end namespace caf
