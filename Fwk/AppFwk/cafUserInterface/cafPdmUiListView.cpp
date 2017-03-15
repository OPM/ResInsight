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


#include "cafPdmUiListView.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiListViewEditor.h"

#include <QHBoxLayout>


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListView::PdmUiListView(QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f)
{
    m_layout = new QVBoxLayout(this);
    m_layout->insertStretch(1, 1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    setLayout(m_layout);

    m_listViewEditor = new PdmUiListViewEditor();

    QWidget* widget = m_listViewEditor->getOrCreateWidget(this);
    m_layout->addWidget(widget);

    this->m_layout->setStretchFactor(widget, 10);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiListView::~PdmUiListView()
{
    if (m_listViewEditor) delete m_listViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiListView::setPdmObject(caf::PdmObjectCollection* object)
{
    CAF_ASSERT(m_listViewEditor);

    m_listViewEditor->setPdmObject(object);
}


} //End of namespace caf

