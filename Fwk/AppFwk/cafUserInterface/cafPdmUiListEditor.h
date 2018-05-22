//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

class QItemSelection;
class QLabel;
class QListViewHeightHint;
class QModelIndex;
class QStringList;
class QStringListModel;

namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiListEditorAttribute : public PdmUiEditorAttribute
{
public:
    PdmUiListEditorAttribute()
        : m_heightHint(2000)
    {
        QPalette myPalette;

        m_baseColor = myPalette.color(QPalette::Active, QPalette::Base);
    }

public:
    QColor  m_baseColor;
    int     m_heightHint;
};


//==================================================================================================
/// 
//==================================================================================================
class PdmUiListEditor : public PdmUiFieldEditorHandle
{
    Q_OBJECT
    CAF_PDM_UI_FIELD_EDITOR_HEADER_INIT;

public:
    PdmUiListEditor(); 
    virtual ~PdmUiListEditor(); 

protected:
    virtual QWidget*    createEditorWidget(QWidget * parent);
    virtual QWidget*    createLabelWidget(QWidget * parent);
    virtual void        configureAndUpdateUi(const QString& uiConfigName);
    virtual bool        eventFilter ( QObject * listView, QEvent * event ); // To catch delete key press in list view.

protected slots:
    void                slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    void                slotListItemEdited(const QModelIndex&, const QModelIndex&);

private:
    QString             contentAsString() const;
    void                pasteFromString(const QString& content);
    
    void                trimAndSetValuesToField(const QStringList& stringList);

private:
    QPointer<QListViewHeightHint> m_listView;
    QPointer<QLabel>            m_label;
    QPointer<QStringListModel>  m_model;

    bool                    m_isEditOperationsAvailable;
    int                     m_optionItemCount;
};


} // end namespace caf
