//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015- Ceetron Solutions AS
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


#include "cafPdmUiPropertyViewDialog.h"

#include "cafPdmObject.h"
#include "cafPdmUiPropertyView.h"

#include <QVBoxLayout>


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::PdmUiPropertyViewDialog(QWidget* parent, PdmObject* object, const QString& windowTitle, const QString& uiConfigName)
{
    initialize(parent, object, windowTitle, uiConfigName, QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::PdmUiPropertyViewDialog(QWidget* parent, PdmObject* object, const QString& windowTitle,
                                                 const QString& uiConfigName, const QDialogButtonBox::StandardButtons& standardButtons)
{
    initialize(parent, object, windowTitle, uiConfigName, standardButtons);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiPropertyViewDialog::~PdmUiPropertyViewDialog()
{
    m_pdmUiPropertyView->showProperties(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::initialize(QWidget* parent, PdmObject* object, const QString& windowTitle,
                                         const QString& uiConfigName, const QDialogButtonBox::StandardButtons& standardButtons)
{
    m_pdmObject = object;
    m_windowTitle = windowTitle;
    m_uiConfigName = uiConfigName;

    setupUi(standardButtons);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiPropertyViewDialog::setupUi(const QDialogButtonBox::StandardButtons& standardButtons)
{
    setWindowTitle(m_windowTitle);

    m_pdmUiPropertyView = new PdmUiPropertyView(this);
    m_pdmUiPropertyView->setUiConfigurationName(m_uiConfigName);

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);

    dialogLayout->addWidget(m_pdmUiPropertyView);
    m_pdmUiPropertyView->showProperties(m_pdmObject);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(standardButtons);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    dialogLayout->addWidget(buttonBox);
}


} //End of namespace caf

