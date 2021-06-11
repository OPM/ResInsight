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

#include "cafCmdFeatureMenuBuilder.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafFactory.h"

#include <QAction>
#include <QMenu>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder::CmdFeatureMenuBuilder()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder::~CmdFeatureMenuBuilder()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::operator<<( const QString& commandId )
{
    if ( commandId == "Separator" )
    {
        addSeparator();
    }
    else
    {
        addCmdFeature( commandId );
    }

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::addCmdFeature( const QString commandId, const QString& uiText )
{
    CAF_ASSERT( !commandId.isEmpty() );

    MenuItem i;
    i.itemType = MenuItem::COMMAND;
    i.itemName = commandId;
    i.uiText   = uiText;
    m_items.push_back( i );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::addCmdFeatureWithUserData( const QString   commandId,
                                                                         const QString&  uiText,
                                                                         const QVariant& userData )
{
    MenuItem i;
    i.itemType = MenuItem::COMMAND;
    i.itemName = commandId;
    i.uiText   = uiText;
    i.userData = userData;
    m_items.push_back( i );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::addSeparator()
{
    MenuItem i;
    i.itemType = MenuItem::SEPARATOR;
    m_items.push_back( i );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::subMenuStart( const QString& menuName, const QIcon& menuIcon )
{
    MenuItem i;
    i.itemType = MenuItem::SUBMENU_START;
    i.itemName = menuName;
    i.icon     = menuIcon;
    m_items.push_back( i );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeatureMenuBuilder& CmdFeatureMenuBuilder::subMenuEnd()
{
    MenuItem i;
    i.itemType = MenuItem::SUBMENU_END;
    m_items.push_back( i );

    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeatureMenuBuilder::appendToMenu( QMenu* menu )
{
    CAF_ASSERT( menu );

    std::vector<QMenu*> menus = { menu };
    for ( size_t i = 0; i < m_items.size(); i++ )
    {
        if ( m_items[i].itemType == MenuItem::SEPARATOR )
        {
            menus.back()->addSeparator();
        }
        else if ( m_items[i].itemType == MenuItem::SUBMENU_START )
        {
            QMenu* subMenu = menus.back()->addMenu( m_items[i].icon, m_items[i].itemName );
            menus.push_back( subMenu );
        }
        else if ( m_items[i].itemType == MenuItem::SUBMENU_END )
        {
            if ( menus.size() > 1 )
            {
                QMenu* completeSubMenu = menus.back();
                menus.pop_back();

                if ( !menus.empty() )
                {
                    // Remove the sub menu action if no (sub) actions are present in the sub menu
                    if ( completeSubMenu->actions().isEmpty() )
                    {
                        QMenu* menuWithEmptySubMenu = menus.back();

                        QAction* subMenuAction = completeSubMenu->menuAction();

                        menuWithEmptySubMenu->removeAction( subMenuAction );
                    }
                }
            }
        }
        else
        {
            CmdFeatureManager* commandManager = CmdFeatureManager::instance();
            QMenu*             currentMenu    = menus.back();
            caf::CmdFeature*   feature        = commandManager->getCommandFeature( m_items[i].itemName.toStdString() );
            CAF_ASSERT( feature );

            if ( feature->canFeatureBeExecuted() )
            {
                const QAction* act;
                if ( !m_items[i].userData.isNull() )
                {
                    act = commandManager->actionWithUserData( m_items[i].itemName, m_items[i].uiText, m_items[i].userData );
                }
                else
                {
                    act = commandManager->action( m_items[i].itemName );
                }

                CAF_ASSERT( act );

                bool duplicateAct = false;
                for ( QAction* existingAct : currentMenu->actions() )
                {
                    // If action exist, continue to make sure the action is positioned at the first
                    // location of a command ID
                    if ( existingAct == act )
                    {
                        duplicateAct = true;
                        break;
                    }
                }
                if ( duplicateAct ) continue;

                currentMenu->addAction( const_cast<QAction*>( act ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdFeatureMenuBuilder::isCmdFeatureAdded( const QString& commandId )
{
    for ( const MenuItem& item : m_items )
    {
        if ( item.itemType == MenuItem::COMMAND && item.itemName == commandId )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t CmdFeatureMenuBuilder::itemCount() const
{
    return m_items.size();
}

} // end namespace caf
