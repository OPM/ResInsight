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

#include "cafPdmUiTreeView.h"

#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"

#include <QHBoxLayout>
#include "cafPdmUiTreeViewEditor.h"


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::PdmUiTreeView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    m_layout = new QVBoxLayout(this);
    m_layout->insertStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    setLayout(m_layout);

    m_treeViewEditor = new PdmUiTreeViewEditor();

    QWidget * widget = m_treeViewEditor->getOrCreateWidget(this);

    this->m_layout->insertWidget(0, widget);
    this->m_layout->setStretchFactor(widget, 10);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeView::~PdmUiTreeView()
{
    if (m_treeViewEditor) delete m_treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setUiConfigurationName(QString uiConfigName)
{
    // Reset everything, and possibly create widgets etc afresh
    if (m_uiConfigName != uiConfigName)
    { 
        m_uiConfigName = uiConfigName;

        m_treeViewEditor->updateUi(m_uiConfigName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeView::setPdmObject(caf::PdmObject* object)
{
    m_treeViewEditor->setPdmObject(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeView::treeView()
{
    return m_treeViewEditor->treeView();
}

} //End of namespace caf

