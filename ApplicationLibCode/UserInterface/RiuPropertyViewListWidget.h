/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#pragma once

#include <QDialog>
#include <QStringList>

namespace caf
{
class PdmObject;
class PdmUiPropertyView;
} // namespace caf

class QDialogButtonBox;
class QWidget;
class QString;
class QStackedWidget;
class QListView;
class QStandardItemModel;
class QLabel;

class RiuPropertyViewListWidget : public QDialog
{
public:
    RiuPropertyViewListWidget( QWidget* parent, caf::PdmObject* object, const QString& windowTitle, const QStringList& uiConfigNameForTabs );
    ~RiuPropertyViewListWidget() override;

    QSize             minimumSizeHint() const override;
    QSize             sizeHint() const override;
    QDialogButtonBox* dialogButtonBox();

protected:
    void showEvent( QShowEvent* event ) override;
    void done( int result ) override;

private:
    QString settingsKey() const;
    bool    restoreDialogGeometry();
    void    saveDialogGeometry();

private:
    QStandardItemModel*                  m_propertyModel;
    QStackedWidget*                      m_propertyStack;
    QListView*                           m_propertyList;
    QLabel*                              m_pageTitle;
    QString                              m_windowTitle;
    QString                              m_objectClassKeyword;
    std::vector<caf::PdmUiPropertyView*> m_pageWidgets;
    QDialogButtonBox*                    m_dialogButtonBox;
    bool                                 m_geometryRestored = false;
};
