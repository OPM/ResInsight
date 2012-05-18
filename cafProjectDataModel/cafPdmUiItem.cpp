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

#include "cafPdmUiItem.h"


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList PdmOptionItemInfo::extractUiTexts(const QList<PdmOptionItemInfo>& optionList)
{
    QStringList texts;
    int i; 
    for (i = 0; i < optionList.size(); ++i)
    {
        texts.push_back(optionList[i].optionUiText);
    }
    return texts;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::findValue(const QList<PdmOptionItemInfo>& optionList , QVariant fieldValue, unsigned int* indexToValue /*= NULL*/)
{
    // Find this field value in the list if present
    unsigned int i;
    bool foundFieldValue = false;

    for(i = 0; i < static_cast<unsigned int>(optionList.size()); ++i)
    {
        if (optionList[i].value == fieldValue)
        {
            foundFieldValue = true;
            break;
        }
    }
    if (indexToValue) *indexToValue = i;
    return foundFieldValue;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiName() const
{
    if(m_dynamicItemInfo.m_uiName.isNull()) 
    {
        if(m_staticItemInfo) return m_staticItemInfo->m_uiName;   
        else return QString(""); 
    }
    else
    {
        return m_dynamicItemInfo.m_uiName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QIcon PdmUiItem::uiIcon() const
{
    if(m_dynamicItemInfo.m_icon.isNull()) 
    {
        if(m_staticItemInfo) return m_staticItemInfo->m_icon;
        else return QIcon();
    }
    else
    {
        return m_dynamicItemInfo.m_icon;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiToolTip() const
{
    if(m_dynamicItemInfo.m_toolTip.isNull()) 
    {
        if(m_staticItemInfo) return m_staticItemInfo->m_toolTip;   
        else return QString("");
    }
    else
    {
        return m_dynamicItemInfo.m_toolTip;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiWhatsThis() const
{
    if(m_dynamicItemInfo.m_whatsThis.isNull()) 
    {
        if(m_staticItemInfo) return m_staticItemInfo->m_whatsThis; 
        else return QString("");
    }
    else
    {
        return m_dynamicItemInfo.m_whatsThis;
    }
}

} //End of namespace caf

