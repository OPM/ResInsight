/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RIStdInclude.h"
#include "RIPreferencesDialog.h"

#include "cafAppEnum.h"
#include "cafPdmObject.h"

#include "RimUiTreeModelPdm.h"
#include "qtgroupboxpropertybrowser.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RIPreferencesDialog::RIPreferencesDialog(QWidget* parent, caf::PdmObject* object)
    : QDialog(parent)
{
    CVF_ASSERT(object);
    
    m_pdmObject = object;

    setupUi();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RIPreferencesDialog::setupUi()
{
    setWindowTitle("Preferences");

    m_uiManagerPdm = new RimUiPropertyCreatorPdm(this);

    QVBoxLayout* dialogLayout = new QVBoxLayout;
    setLayout(dialogLayout);

    QtGroupBoxPropertyBrowser* groupPropertyBrowser = new QtGroupBoxPropertyBrowser();
    m_uiManagerPdm->setPropertyBrowser(groupPropertyBrowser);
    m_uiManagerPdm->createAndShowPropertiesForObject(m_pdmObject);
    dialogLayout->addWidget(groupPropertyBrowser);


    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    dialogLayout->addWidget(buttonBox);

    this->resize(400, 200);
}
