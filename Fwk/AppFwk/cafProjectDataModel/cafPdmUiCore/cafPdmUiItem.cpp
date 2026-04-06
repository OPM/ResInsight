//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of the GNU General Public License or
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

#include "cafPdmLogging.h"
#include "cafPdmOptionItemInfo.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiItemInfo.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafUpdateEditorsScheduler.h"
#include <QIcon>

namespace caf
{
//==================================================================================================
/// PdmUiItem
//==================================================================================================

bool PdmUiItem::sm_showExtraDebugText = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiName( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;
    if ( uiConfigName == uiConfigNameForStaticData() && sttInfo ) return sttInfo->m_uiName;

    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();

    if ( conInfo && !( conInfo->m_uiName.isNull() ) ) return conInfo->m_uiName;
    if ( defInfo && !( defInfo->m_uiName.isNull() ) ) return defInfo->m_uiName;
    if ( sttInfo && !( sttInfo->m_uiName.isNull() ) ) return sttInfo->m_uiName;

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiName( const QString& uiName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_uiName = uiName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiItem::uiConfigNameForStaticData()
{
    return "uiConfigNameForStaticData";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<QIcon> PdmUiItem::uiIcon( const QString& uiConfigName ) const
{
    return uiIconProvider( uiConfigName ).icon();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const IconProvider PdmUiItem::uiIconProvider( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->iconProvider().valid() ) return conInfo->iconProvider();
    if ( defInfo && defInfo->iconProvider().valid() ) return defInfo->iconProvider();
    if ( sttInfo && sttInfo->iconProvider().valid() ) return sttInfo->iconProvider();

    return IconProvider();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiIcon( const IconProvider& uiIconProvider, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_iconProvider = uiIconProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiIconFromResourceString( const QString& uiIconResourceName, const QString& uiConfigName /*= ""*/ )
{
    setUiIcon( caf::IconProvider( uiIconResourceName ), uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QColor PdmUiItem::uiContentTextColor( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && ( conInfo->m_contentTextColor.isValid() ) ) return conInfo->m_contentTextColor;
    if ( defInfo && ( defInfo->m_contentTextColor.isValid() ) ) return defInfo->m_contentTextColor;
    if ( sttInfo && ( sttInfo->m_contentTextColor.isValid() ) ) return sttInfo->m_contentTextColor;

    return QColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiContentTextColor( const QColor& color, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_contentTextColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiToolTip( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    QString text;

    if ( conInfo && !( conInfo->m_toolTip.isNull() ) )
    {
        text = conInfo->m_toolTip;
        if ( PdmUiItem::showExtraDebugText() && !conInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( conInfo->m_extraDebugText );
        }
    }

    if ( text.isEmpty() && defInfo && !( defInfo->m_toolTip.isNull() ) )
    {
        text = defInfo->m_toolTip;
        if ( PdmUiItem::showExtraDebugText() && !defInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( defInfo->m_extraDebugText );
        }
    }

    if ( text.isEmpty() && sttInfo && !( sttInfo->m_toolTip.isNull() ) )
    {
        text = sttInfo->m_toolTip;
        if ( PdmUiItem::showExtraDebugText() && !sttInfo->m_extraDebugText.isEmpty() )
        {
            text += QString( " (%1)" ).arg( sttInfo->m_extraDebugText );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiToolTip( const QString& uiToolTip, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_toolTip = uiToolTip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString PdmUiItem::uiWhatsThis( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_whatsThis.isNull() ) ) return conInfo->m_whatsThis;
    if ( defInfo && !( defInfo->m_whatsThis.isNull() ) ) return defInfo->m_whatsThis;
    if ( sttInfo && !( sttInfo->m_whatsThis.isNull() ) ) return sttInfo->m_whatsThis;

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiWhatsThis( const QString& uiWhatsThis, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_whatsThis = uiWhatsThis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiHidden( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_isHidden.has_value() ) return conInfo->m_isHidden.value();
    if ( defInfo && defInfo->m_isHidden.has_value() ) return defInfo->m_isHidden.value();
    if ( sttInfo && sttInfo->m_isHidden.has_value() ) return sttInfo->m_isHidden.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiHidden( bool isHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isHidden = isHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiTreeHidden( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_isTreeHidden.has_value() ) return conInfo->m_isTreeHidden.value();
    if ( defInfo && defInfo->m_isTreeHidden.has_value() ) return defInfo->m_isTreeHidden.value();
    if ( sttInfo && sttInfo->m_isTreeHidden.has_value() ) return sttInfo->m_isTreeHidden.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiTreeHidden( bool isTreeHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isTreeHidden = isTreeHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiTreeChildrenHidden( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_isTreeChildrenHidden.has_value() ) return conInfo->m_isTreeChildrenHidden.value();
    if ( defInfo && defInfo->m_isTreeChildrenHidden.has_value() ) return defInfo->m_isTreeChildrenHidden.value();
    if ( sttInfo && sttInfo->m_isTreeChildrenHidden.has_value() ) return sttInfo->m_isTreeChildrenHidden.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiTreeChildrenHidden( bool isTreeChildrenHidden, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isTreeChildrenHidden = isTreeChildrenHidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiReadOnly( const QString& uiConfigName /*= ""*/ ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_isReadOnly.has_value() ) return conInfo->m_isReadOnly.value();
    if ( defInfo && defInfo->m_isReadOnly.has_value() ) return defInfo->m_isReadOnly.value();
    if ( sttInfo && sttInfo->m_isReadOnly.has_value() ) return sttInfo->m_isReadOnly.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiReadOnly( bool isReadOnly, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isReadOnly = isReadOnly;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::notifyAllFieldsInMultiFieldChangedEvents( const QString& uiConfigName /*= "" */ ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_notifyAllFieldsInMultiFieldChangedEvents.has_value() )
        return conInfo->m_notifyAllFieldsInMultiFieldChangedEvents.value();
    if ( defInfo && defInfo->m_notifyAllFieldsInMultiFieldChangedEvents.has_value() )
        return defInfo->m_notifyAllFieldsInMultiFieldChangedEvents.value();
    if ( sttInfo && sttInfo->m_notifyAllFieldsInMultiFieldChangedEvents.has_value() )
        return sttInfo->m_notifyAllFieldsInMultiFieldChangedEvents.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setNotifyAllFieldsInMultiFieldChangedEvents( bool enable, const QString& uiConfigName /*= "" */ )
{
    m_configItemInfos[uiConfigName].m_notifyAllFieldsInMultiFieldChangedEvents = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiItem::uiEditorTypeName( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_editorTypeName.isEmpty() ) ) return conInfo->m_editorTypeName;
    if ( defInfo && !( defInfo->m_editorTypeName.isEmpty() ) ) return defInfo->m_editorTypeName;
    if ( sttInfo && !( sttInfo->m_editorTypeName.isEmpty() ) ) return sttInfo->m_editorTypeName;

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiEditorTypeName( const QString& editorTypeName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_editorTypeName = editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmUiItem::ui3dEditorTypeName( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && !( conInfo->m_3dEditorTypeName.isEmpty() ) ) return conInfo->m_3dEditorTypeName;
    if ( defInfo && !( defInfo->m_3dEditorTypeName.isEmpty() ) ) return defInfo->m_3dEditorTypeName;
    if ( sttInfo && !( sttInfo->m_3dEditorTypeName.isEmpty() ) ) return sttInfo->m_3dEditorTypeName;

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUi3dEditorTypeName( const QString& editorTypeName, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_3dEditorTypeName = editorTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isUiGroup() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItemInfo::LabelPosition PdmUiItem::uiLabelPosition( const QString& uiConfigName ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo ) return conInfo->m_labelPosition;
    if ( defInfo ) return defInfo->m_labelPosition;
    if ( sttInfo ) return sttInfo->m_labelPosition;

    return PdmUiItemInfo::LabelPosition::LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiLabelPosition( PdmUiItemInfo::LabelPosition position, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_labelPosition = position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::isCustomContextMenuEnabled( const QString& uiConfigName /*= ""*/ ) const
{
    const PdmUiItemInfo* conInfo = configInfo( uiConfigName );
    const PdmUiItemInfo* defInfo = defaultInfo();
    const PdmUiItemInfo* sttInfo = m_staticItemInfo;

    if ( conInfo && conInfo->m_isCustomContextMenuEnabled.has_value() )
        return conInfo->m_isCustomContextMenuEnabled.value();
    if ( defInfo && defInfo->m_isCustomContextMenuEnabled.has_value() )
        return defInfo->m_isCustomContextMenuEnabled.value();
    if ( sttInfo && sttInfo->m_isCustomContextMenuEnabled.has_value() )
        return sttInfo->m_isCustomContextMenuEnabled.value();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setCustomContextMenuEnabled( bool enableCustomContextMenu, const QString& uiConfigName /*= ""*/ )
{
    m_configItemInfos[uiConfigName].m_isCustomContextMenuEnabled = enableCustomContextMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::configInfo( const QString& uiConfigName ) const
{
    if ( uiConfigName == "" || uiConfigName.isNull() ) return nullptr;

    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find( uiConfigName );

    if ( it != m_configItemInfos.end() ) return &( it->second );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

const PdmUiItemInfo* PdmUiItem::defaultInfo() const
{
    std::map<QString, PdmUiItemInfo>::const_iterator it;
    it = m_configItemInfos.find( "" );

    if ( it != m_configItemInfos.end() ) return &( it->second );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateConnectedEditors() const
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for ( it = m_editors.begin(); it != m_editors.end(); ++it )
    {
        ( *it )->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::scheduleUpdateConnectedEditors() const
{
    UpdateEditorsScheduler::instance()->scheduleUpdateConnectedEditors( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateAllRequiredEditors() const
{
    updateConnectedEditors();

    PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem::~PdmUiItem()
{
    std::set<PdmUiEditorHandle*>::iterator it;
    for ( it = m_editors.begin(); it != m_editors.end(); ++it )
    {
        ( *it )->m_pdmItem = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiItem::PdmUiItem()
    : m_staticItemInfo( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::updateUiIconFromState( bool isActive, const QString& uiConfigName )
{
    IconProvider normalIconProvider = this->uiIconProvider( uiConfigName );
    normalIconProvider.setActive( isActive );
    this->setUiIcon( normalIconProvider, uiConfigName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<PdmUiEditorHandle*> PdmUiItem::connectedEditors() const
{
    std::vector<PdmUiEditorHandle*> editors;
    for ( auto e : m_editors )
    {
        editors.push_back( e );
    }

    return editors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::hasEditor( PdmUiEditorHandle* editor ) const
{
    return m_editors.count( editor ) > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiItem::showExtraDebugText()
{
    return sm_showExtraDebugText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::enableExtraDebugText( bool enable )
{
    sm_showExtraDebugText = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::setUiItemInfo( PdmUiItemInfo* itemInfo )
{
    m_staticItemInfo = itemInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::removeFieldEditor( PdmUiEditorHandle* fieldView )
{
    m_editors.erase( fieldView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiItem::addFieldEditor( PdmUiEditorHandle* fieldView )
{
    m_editors.insert( fieldView );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiItem::attributeVariant( const QString& key, const QString& uiConfigName ) const
{
    // Check config-specific attributes first
    if ( !uiConfigName.isEmpty() )
    {
        auto configIt = m_attributeMaps.find( uiConfigName );
        if ( configIt != m_attributeMaps.end() )
        {
            auto attrIt = configIt->second.find( key );
            if ( attrIt != configIt->second.end() )
            {
                return attrIt->second;
            }
        }
    }

    // Fall back to default config ("")
    auto defaultIt = m_attributeMaps.find( "" );
    if ( defaultIt != m_attributeMaps.end() )
    {
        auto attrIt = defaultIt->second.find( key );
        if ( attrIt != defaultIt->second.end() )
        {
            return attrIt->second;
        }
    }

    return QVariant(); // Return invalid QVariant if not found
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<QString> PdmUiItem::attributeNames( const QString& uiConfigName ) const
{
    std::list<QString> result;

    // Start with default config attributes
    auto defaultIt = m_attributeMaps.find( "" );
    if ( defaultIt != m_attributeMaps.end() )
    {
        for ( const auto& [key, value] : defaultIt->second )
        {
            result.push_back( key );
        }
    }

    // Add config-specific attributes (avoid duplicates)
    if ( !uiConfigName.isEmpty() )
    {
        auto configIt = m_attributeMaps.find( uiConfigName );
        if ( configIt != m_attributeMaps.end() )
        {
            for ( const auto& [key, value] : configIt->second )
            {
                // Check if key already exists in result
                if ( std::find( result.begin(), result.end(), key ) == result.end() )
                {
                    result.push_back( key );
                }
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Validate that all attributes set on this item are in the supported set
/// Logs warnings for unsupported attributes
//--------------------------------------------------------------------------------------------------
void PdmUiItem::validateAttributes( const QString&           contextName,
                                    const std::set<QString>& supportedAttributes,
                                    const QString&           uiConfigName ) const
{
    auto allAttributeNames = attributeNames( uiConfigName );
    for ( const auto& key : allAttributeNames )
    {
        if ( supportedAttributes.find( key ) == supportedAttributes.end() )
        {
            CAF_PDM_LOG_WARNING(
                QString( "%1: Unsupported attribute '%2' set on field. Supported attributes are: %3" )
                    .arg( contextName )
                    .arg( key )
                    .arg( QStringList( supportedAttributes.begin(), supportedAttributes.end() ).join( ", " ) ) );
        }
    }
}

} // End of namespace caf
