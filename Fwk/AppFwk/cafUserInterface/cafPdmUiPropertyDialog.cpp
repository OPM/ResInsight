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


#include "cafPdmUiPropertyDialog.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <assert.h>



namespace caf {

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyDialog::PdmUiPropertyDialog(QWidget* parent, caf::PdmObject* object, const QString& windowTitle)
    : QDialog(parent)
{
    assert(object);

    m_pdmObject = object;
    m_windowTitle = windowTitle;

    setupUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyDialog::setupUi()
{
    setWindowTitle(m_windowTitle);

    m_pdmUiPropertyView = new caf::PdmUiPropertyView(this);

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);

    dialogLayout->addWidget(m_pdmUiPropertyView);
    m_pdmUiPropertyView->showProperties(m_pdmObject);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    dialogLayout->addWidget(buttonBox);

    this->resize(400, 200);
}


}  // end namespace caf
