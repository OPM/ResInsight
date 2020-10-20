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

#pragma once

#include <map>
#include <set>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QStringList>

class QAction;
class QKeySequence;
class QWidget;

namespace caf
{
class CmdFeature;

//==================================================================================================
///
//==================================================================================================
class CmdFeatureManager : public QObject
{
    Q_OBJECT

public:
    static CmdFeatureManager* instance();
    ~CmdFeatureManager() override;

    QAction* action( const QString& commandId );
    QAction* action( const QString& commandId, const QString& customActionText );
    QAction* actionWithUserData( const QString& commandId, const QString& customActionText, const QVariant& userData );

    void refreshStates( const QStringList& commandIdList = QStringList() );
    void refreshEnabledState( const QStringList& commandIdList = QStringList() );
    void refreshCheckedState( const QStringList& commandIdList = QStringList() );

    CmdFeature*              getCommandFeature( const std::string& commandId );
    std::vector<CmdFeature*> commandFeaturesMatchingSubString( const std::string& subString ) const;
    std::vector<CmdFeature*> commandFeaturesMatchingKeyboardShortcut( const QKeySequence& keySequence ) const;

    void     setCurrentContextMenuTargetWidget( QWidget* targetWidget );
    QWidget* currentContextMenuTargetWidget();

private:
    CmdFeatureManager();

    std::pair<CmdFeature*, size_t> createFeature( const std::string& commandId );
    std::pair<CmdFeature*, size_t> findExistingCmdFeature( const std::string& commandId );

    CmdFeature* commandFeature( const std::string& commandId ) const;

private:
    std::vector<CmdFeature*>      m_commandFeatures;
    std::map<std::string, size_t> m_commandIdToFeatureIdxMap;
    std::map<QAction*, size_t>    m_actionToFeatureIdxMap;

    QPointer<QWidget> m_currentContextMenuTargetWidget;
};

} // end namespace caf
