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
#include "cafPdmUiEditorHandle.h"

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
const QString PdmUiItem::uiName(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_uiName.isNull())) return conInfo->m_uiName;
    if (defInfo && !(defInfo->m_uiName.isNull())) return defInfo->m_uiName;
    if (sttInfo && !(sttInfo->m_uiName.isNull())) return sttInfo->m_uiName;

    return QString(""); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QIcon PdmUiItem::uiIcon(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_icon.isNull())) return conInfo->m_icon;
    if (defInfo && !(defInfo->m_icon.isNull())) return defInfo->m_icon;
    if (sttInfo && !(sttInfo->m_icon.isNull())) return sttInfo->m_icon;

    return QIcon();    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiToolTip(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_toolTip.isNull())) return conInfo->m_toolTip;
    if (defInfo && !(defInfo->m_toolTip.isNull())) return defInfo->m_toolTip;
    if (sttInfo && !(sttInfo->m_toolTip.isNull())) return sttInfo->m_toolTip;

    return QString(""); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiWhatsThis(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_whatsThis.isNull())) return conInfo->m_whatsThis;
    if (defInfo && !(defInfo->m_whatsThis.isNull())) return defInfo->m_whatsThis;
    if (sttInfo && !(sttInfo->m_whatsThis.isNull())) return sttInfo->m_whatsThis;

    return QString(""); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiHidden(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isHidden == -1)) return conInfo->m_isHidden;
    if (defInfo && !(defInfo->m_isHidden == -1)) return defInfo->m_isHidden;
    if (sttInfo && !(sttInfo->m_isHidden == -1)) return sttInfo->m_isHidden;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiReadOnly(QString uiConfigName /*= ""*/)
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isReadOnly == -1)) return conInfo->m_isReadOnly;
    if (defInfo && !(defInfo->m_isReadOnly == -1)) return defInfo->m_isReadOnly;
    if (sttInfo && !(sttInfo->m_isReadOnly == -1)) return sttInfo->m_isReadOnly;

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString PdmUiItem::uiEditorTypeName(const QString& uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_editorTypeName.isEmpty())) return conInfo->m_editorTypeName;
    if (defInfo && !(defInfo->m_editorTypeName.isEmpty())) return defInfo->m_editorTypeName;
    if (sttInfo && !(sttInfo->m_editorTypeName.isEmpty())) return sttInfo->m_editorTypeName;

    return QString();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::configInfo(QString uiConfigName) const
{
    if (uiConfigName == "" || uiConfigName.isNull()) return NULL;

    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find(uiConfigName);

    if (it != m_configItemInfos.end()) return &(it->second);

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::defaultInfo() const
{
    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find("");

    if (it != m_configItemInfos.end()) return &(it->second);

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateConnectedEditors()
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for (it = m_editors.begin(); it != m_editors.end(); ++it)
    {
        (*it)->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiItem::~PdmUiItem()
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for (it = m_editors.begin(); it != m_editors.end(); ++it)
    {
        (*it)->m_pdmItem = NULL;
    }
}

} //End of namespace caf

