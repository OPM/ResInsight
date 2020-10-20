//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafCmdFeature.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"

#include "cafPdmUiModelChangeDetector.h"

#include <QAction>
#include <QApplication>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeature::CmdFeature()
    : m_triggerModelChange( true )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFeature::~CmdFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* CmdFeature::action()
{
    return this->actionWithCustomText( QString( "" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* CmdFeature::actionWithCustomText( const QString& customText )
{
    return actionWithUserData( customText, QVariant() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QAction* CmdFeature::actionWithUserData( const QString& customText, const QVariant& userData )
{
    QAction* action = nullptr;

    std::map<QString, QAction*>::iterator it;
    it = m_customTextToActionMap.find( customText );

    if ( it != m_customTextToActionMap.end() && it->second != NULL )
    {
        action = it->second;
    }
    else
    {
        action = new QAction( this );

        connect( action, SIGNAL( triggered( bool ) ), SLOT( actionTriggered( bool ) ) );
        m_customTextToActionMap[customText] = action;
    }

    if ( !userData.isNull() )
    {
        action->setData( userData );
    }

    if ( dynamic_cast<QApplication*>( QCoreApplication::instance() ) )
    {
        this->setupActionLook( action );
    }
    if ( !customText.isEmpty() )
    {
        action->setText( customText );
    }

    return action;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshEnabledState()
{
    std::map<QString, QAction*>::iterator it;
    bool                                  isEnabled = this->isCommandEnabled();

    for ( it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it )
    {
        it->second->setEnabled( isEnabled );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshCheckedState()
{
    std::map<QString, QAction*>::iterator it;
    bool                                  isChecked = this->isCommandChecked();

    for ( it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it )
    {
        QAction* act = it->second;
        if ( act->isCheckable() )
        {
            it->second->setChecked( isChecked );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdFeature::canFeatureBeExecuted()
{
    return this->isCommandEnabled();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::applyShortcutWithHintToAction( QAction* action, const QKeySequence& keySequence )
{
    action->setShortcut( keySequence );

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 ) )
    // Qt made keyboard shortcuts in context menus platform dependent in Qt 5.10
    // With no global way of removing it.
    action->setShortcutVisibleInContextMenu( true );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::actionTriggered( bool isChecked )
{
    this->onActionTriggered( isChecked );

    if ( m_triggerModelChange )
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdFeature::isCommandChecked()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFeature::disableModelChangeContribution()
{
    m_triggerModelChange = false;
}

//--------------------------------------------------------------------------------------------------
/// Returns action user data.
/// May be called from onActionTriggered only
//--------------------------------------------------------------------------------------------------
const QVariant CmdFeature::userData() const
{
    QAction* action = qobject_cast<QAction*>( sender() );
    CAF_ASSERT( action );

    return action->data();
}

} // end namespace caf
