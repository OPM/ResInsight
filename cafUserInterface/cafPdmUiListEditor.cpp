//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cafPdmUiListEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QLineEdit>
#include <QLabel>
#include <QListView>
#include <QStringListModel>
#include <QBoxLayout>
#include <QListView>
#include <QDebug>


#include <assert.h>

//==================================================================================================
/// Helper class used to override flags to disable editable items
//==================================================================================================
class MyStringListModel : public QStringListModel
{
public:
    MyStringListModel(QObject *parent = 0) : QStringListModel(parent) { }

    virtual Qt::ItemFlags flags (const QModelIndex& index) const
    {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
};



namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiListEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListEditor::PdmUiListEditor()
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListEditor::~PdmUiListEditor()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    assert(!m_listView.isNull());
    assert(!m_label.isNull());
    assert(m_listView->selectionModel());

    QIcon ic = field()->uiIcon(uiConfigName);
    if (!ic.isNull())
    {
        m_label->setPixmap(ic.pixmap(ic.actualSize(QSize(64, 64))));
    }
    else
    {
        m_label->setText(field()->uiName(uiConfigName));
    }

    m_label->setVisible(!field()->isUiHidden(uiConfigName));
    m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));

    m_listView->setEnabled(!field()->isUiReadOnly(uiConfigName));

    PdmUiListEditorAttribute attributes;
    field()->ownerObject()->editorAttribute(field(), uiConfigName, &attributes);

    bool fromMenuOnly = false;
    QList<PdmOptionItemInfo> enumNames = field()->valueOptions(&fromMenuOnly);
    if (!enumNames.isEmpty() && fromMenuOnly == true)
    {
        QStringList texts = PdmOptionItemInfo::extractUiTexts(enumNames);
        m_model->setStringList(texts);
        
        int col = 0;
        int row = field()->uiValue().toInt();
        QModelIndex mi = m_model->index(row, col);
            
        m_listView->selectionModel()->blockSignals(true);

        m_listView->selectionModel()->select(mi, QItemSelectionModel::SelectCurrent);
        m_listView->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::SelectCurrent);

        m_listView->selectionModel()->blockSignals(false);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListEditor::createEditorWidget(QWidget * parent)
{
    m_listView = new QListView(parent);

    m_model = new MyStringListModel(m_listView);
    m_listView->setModel(m_model);

    connect(m_listView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection& )), this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection& )));

    return m_listView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiListEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().size() == 1)
    {
        QModelIndex mi = selected.indexes()[0];
        int col = mi.column();
        int row = mi.row();
        
        QVariant v;
        v = row;
        QVariant uintValue(v.toUInt());

        this->setValueToField(uintValue);
    }
}


} // end namespace caf
