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


#include "cafPdmUiItem.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmPtrField.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo(const QString& anOptionUiText, const QVariant& aValue, bool anIsDimmed /* = false */, QIcon anIcon /* = QIcon()*/)
    : m_value(aValue),
    m_optionUiText(anOptionUiText),
    m_isDimmed(anIsDimmed),
    m_icon(anIcon),
    m_level(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmOptionItemInfo::PdmOptionItemInfo(const QString& anOptionUiText, caf::PdmObjectHandle* obj, bool anIsDimmed /*= false*/, QIcon anIcon /*= QIcon()*/)
    : m_optionUiText(anOptionUiText),
    m_isDimmed(anIsDimmed),
    m_icon(anIcon),
    m_level(0)
{
    m_value = QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(obj));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmOptionItemInfo::setLevel(int level)
{
    m_level = level;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString PdmOptionItemInfo::optionUiText() const
{
    return m_optionUiText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QVariant PdmOptionItemInfo::value() const
{
    return m_value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmOptionItemInfo::isDimmed() const
{
    return m_isDimmed;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QIcon PdmOptionItemInfo::icon() const
{
    return m_icon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int PdmOptionItemInfo::level() const
{
    return m_level;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList PdmOptionItemInfo::extractUiTexts(const QList<PdmOptionItemInfo>& optionList)
{
    QStringList texts;
    
    for (auto option : optionList)
    {
        texts.push_back(option.optionUiText());
    }

    return texts;
}


//==================================================================================================
/// PdmUiItem
//==================================================================================================


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
bool PdmUiItem::isUiTreeHidden(QString uiConfigName) const
{
    // TODO: Must be separated from uiHidden when childField object embedding is implemented

    return isUiHidden(uiConfigName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiTreeChildrenHidden(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo && !(conInfo->m_isTreeChildrenHidden == -1)) return conInfo->m_isTreeChildrenHidden;
    if (defInfo && !(defInfo->m_isTreeChildrenHidden == -1)) return defInfo->m_isTreeChildrenHidden;
    if (sttInfo && !(sttInfo->m_isTreeChildrenHidden == -1)) return sttInfo->m_isTreeChildrenHidden;

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
PdmUiItemInfo::LabelPosType PdmUiItem::uiLabelPosition(QString uiConfigName) const
{
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName);
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if (conInfo) return conInfo->m_labelAlignment;
    if (defInfo) return defInfo->m_labelAlignment;
    if (sttInfo) return sttInfo->m_labelAlignment;

    return PdmUiItemInfo::LEFT;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateUiIconFromState(bool isActive, QString uiConfigName)
{
    static const QString iconStorageConfigNamePostfix = "_Internally_StoredNormalIcon";
    const PdmUiItemInfo* conInfo = configInfo(uiConfigName + iconStorageConfigNamePostfix);
    QIcon normalIcon;

    if (conInfo)  normalIcon = conInfo->m_icon;
    else          normalIcon = this->uiIcon(uiConfigName);

    this->setUiIcon(normalIcon, uiConfigName + iconStorageConfigNamePostfix);

    if ( isActive )
    {
        this->setUiIcon(normalIcon, uiConfigName);
        m_configItemInfos.erase(uiConfigName + iconStorageConfigNamePostfix);
    }
    else
    {
        QIcon disabledIcon(normalIcon.pixmap(16, 16, QIcon::Disabled));
        this->setUiIcon(disabledIcon, uiConfigName);
        this->setUiIcon(normalIcon, uiConfigName + iconStorageConfigNamePostfix);
    }
}

} //End of namespace caf

