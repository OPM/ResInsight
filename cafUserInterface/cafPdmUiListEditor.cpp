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
#include <QEvent>
#include <QKeyEvent>

//==================================================================================================
/// Helper class used to override flags to disable editable items
//==================================================================================================
class MyStringListModel : public QStringListModel
{
public:
    MyStringListModel(QObject *parent = 0) : m_isItemsEditable(false), QStringListModel(parent) { }

    virtual Qt::ItemFlags flags (const QModelIndex& index) const
    {
        if (m_isItemsEditable)
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        else
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    void setItemsEditable(bool isEditable)
    {
        m_isItemsEditable = isEditable;
    }

private:
    bool m_isItemsEditable;
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
PdmUiListEditor::PdmUiListEditor(): m_optionsOnly(false)
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
        QString uiName = field()->uiName(uiConfigName);
        m_label->setText(uiName);
    }

    m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_listView->setEnabled(!field()->isUiReadOnly(uiConfigName));

    /// Demo code Not used yet
    // PdmUiListEditorAttribute attributes;
    // field()->ownerObject()->editorAttribute(field(), uiConfigName, &attributes);

    MyStringListModel* strListModel = dynamic_cast<MyStringListModel*>(m_model.data());

    assert(strListModel);

    m_options = field()->valueOptions(&m_optionsOnly);
    if (!m_options.isEmpty())
    {
        assert(m_optionsOnly); // Handling Additions on the fly not implemented

        strListModel->setItemsEditable(false);
        QModelIndex currentItem = 	m_listView->selectionModel()->currentIndex();
        QStringList texts = PdmOptionItemInfo::extractUiTexts(m_options);
        strListModel->setStringList(texts);

        QVariant fieldValue = field()->uiValue();
        if (fieldValue.type() == QVariant::Int || fieldValue.type() == QVariant::UInt)
        {
            int col = 0;
            int row = field()->uiValue().toInt();

            QModelIndex mi = strListModel->index(row, col);

            m_listView->selectionModel()->blockSignals(true);
            m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
            if (row >= 0 ) 
            {
                m_listView->selectionModel()->select(mi, QItemSelectionModel::SelectCurrent);
                m_listView->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::SelectCurrent);
            }
            else // A negative value (Undefined UInt ) is interpreted as no selection
            {
                 m_listView->selectionModel()->clearSelection();
            }

            m_listView->selectionModel()->blockSignals(false);
        }
        else if (fieldValue.type() == QVariant::List)
        {
            QList<QVariant> valuesSelectedInField = fieldValue.toList();
            QItemSelection selection;

            for (int i= 0 ; i < valuesSelectedInField.size(); ++i)
            {
                QModelIndex mi = strListModel->index(valuesSelectedInField[i].toInt(), 0);
                selection.append(QItemSelectionRange (mi));
            }

            m_listView->selectionModel()->blockSignals(true);
            
            m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            m_listView->selectionModel()->select(selection, QItemSelectionModel::Select);
            m_listView->selectionModel()->setCurrentIndex(currentItem, QItemSelectionModel::Current);

            m_listView->selectionModel()->blockSignals(false);
        }
    }
    else
    {
        m_listView->selectionModel()->blockSignals(true);

        QItemSelection selection =  m_listView->selectionModel()->selection();
        QModelIndex currentItem = 	m_listView->selectionModel()->currentIndex();
        QVariant fieldValue = field()->uiValue();
        QStringList texts = fieldValue.toStringList();
        texts.push_back("");
        strListModel->setStringList(texts);
        
        strListModel->setItemsEditable(true);

        m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_listView->selectionModel()->select(selection, QItemSelectionModel::Select);
        m_listView->selectionModel()->setCurrentIndex(currentItem, QItemSelectionModel::Current);

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
    connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(slotListItemEdited(const QModelIndex&, const QModelIndex&)));
    m_listView->installEventFilter(this);

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
    if (m_options.isEmpty()) return;

    //QModelIndexList idxList = selected.indexes();
    QModelIndexList idxList =  m_listView->selectionModel()->selectedIndexes();

    QVariant fieldValue = field()->uiValue();
    if (fieldValue.type() == QVariant::Int || fieldValue.type() == QVariant::UInt)
    {
        if (idxList.size() >= 1)
        {
            if (idxList[0].row() < m_options.size())
            {
                this->setValueToField(QVariant(static_cast<unsigned int>(idxList[0].row())));
            }
        }
    }
    else if (fieldValue.type() == QVariant::List)
    {
        QList<QVariant> valuesToSetInField;

        for (int i = 0; i < idxList.size(); ++i)
        {
            if (idxList[i].row() < m_options.size())
            {
                valuesToSetInField.push_back(QVariant(static_cast<unsigned int>(idxList[i].row())));
            }
        }

        this->setValueToField(valuesToSetInField);
    }
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiListEditor::slotListItemEdited(const QModelIndex&, const QModelIndex&)
{
    if (m_optionsOnly) return;
    assert(m_options.isEmpty()); // Not supported yet

    QStringList uiList = m_model->stringList();

    // Remove dummy elements specifically at the  end of list.
    
    QStringList result;
    foreach (const QString &str, uiList) 
    {
        if (str != "" && str != " ") result += str;
    }

    this->setValueToField(result);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiListEditor::eventFilter(QObject * listView, QEvent * event)
{
    if (listView == m_listView && event->type() == QEvent::KeyPress)
    {
        if (m_optionsOnly) return false;
        assert(m_options.isEmpty()); // Not supported yet

        QKeyEvent* keyEv = static_cast<QKeyEvent*>(event);
        if (keyEv->key() == Qt::Key_Delete || keyEv->key() == Qt::Key_Backspace )
        {
            QModelIndexList idxList =  m_listView->selectionModel()->selectedIndexes();
            bool isAnyDeleted = false;
            while(idxList.size())
            {
                m_model->removeRow(idxList[0].row());
                idxList =  m_listView->selectionModel()->selectedIndexes();
                isAnyDeleted = true;
            }

            if (isAnyDeleted)
            {
                QStringList uiList = m_model->stringList();

                // Remove dummy elements specifically at the  end of list.

                QStringList result;
                foreach (const QString &str, uiList) 
                {
                    if (str != "" && str != " ") result += str;
                }
                this->setValueToField(result);
            }
            return true;
        }
    }

    return false;
}

} // end namespace caf
