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


#include "cafPdmUiEditorHandle.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiEditorHandle::PdmUiEditorHandle() 
    : m_pdmItem(nullptr)
    , m_isConfiguringUi(false)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiEditorHandle::~PdmUiEditorHandle()
{
    if (m_pdmItem) m_pdmItem->removeFieldEditor(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiEditorHandle::updateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_isConfiguringUi);
    m_isConfiguringUi = true;
    m_currentConfigName = uiConfigName;
    this->configureAndUpdateUi(uiConfigName);
    m_isConfiguringUi = false;

    emit uiUpdated();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiEditorHandle::updateUi()
{
    CAF_ASSERT(!m_isConfiguringUi);
    m_isConfiguringUi = true;
    this->configureAndUpdateUi(m_currentConfigName);
    m_isConfiguringUi = false;

    emit uiUpdated();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiEditorHandle::updateUiIncludingParent()
{
    if (m_parentEditor)
    {
        m_parentEditor->updateUiIncludingParent();
    }
    else
    {
        this->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiEditorHandle::bindToPdmItem(PdmUiItem* item)
{
    if (m_pdmItem) m_pdmItem->removeFieldEditor(this);
    m_pdmItem = item; 
    if (m_pdmItem) m_pdmItem->addFieldEditor(this);
}


} //End of namespace caf

