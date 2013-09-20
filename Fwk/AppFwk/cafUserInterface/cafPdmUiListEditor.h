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

class QGridLayout;
class QStringListModel;
class QItemSelection;
class QListView;
class QLabel;
class QModelIndex;

namespace caf 
{

//==================================================================================================
/// 
//==================================================================================================
class PdmUiListEditorAttribute : public PdmUiEditorAttribute
{
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
    QPointer<QListView>         m_listView;
    QPointer<QLabel>            m_label;
    QPointer<QStringListModel>  m_model;

    QList<PdmOptionItemInfo> m_options;
    bool                     m_optionsOnly;



};


} // end namespace caf
