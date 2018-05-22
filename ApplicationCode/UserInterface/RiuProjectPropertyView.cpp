/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuProjectPropertyView.h"

#include "RiuMainWindow.h"
#include "RiuTreeViewEventFilter.h"

#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeView.h"

#include <QLabel>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuProjectAndPropertyView::RiuProjectAndPropertyView(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    // Tree View
    m_projectTreeView = new caf::PdmUiTreeView;
    m_projectTreeView->treeView()->setHeaderHidden(true);
    m_projectTreeView->treeView()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_projectTreeView->enableSelectionManagerUpdating(true);

    // Install event filter used to handle key press events
    RiuTreeViewEventFilter* treeViewEventFilter = new RiuTreeViewEventFilter(this);
    m_projectTreeView->treeView()->installEventFilter(treeViewEventFilter);

    // Drag and drop configuration
    m_projectTreeView->treeView()->setDragEnabled(true);
    m_projectTreeView->treeView()->viewport()->setAcceptDrops(true);
    m_projectTreeView->treeView()->setDropIndicatorShown(true);
    m_projectTreeView->treeView()->setDragDropMode(QAbstractItemView::DragDrop);

    m_projectTreeView->treeView()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_projectTreeView->treeView(), SIGNAL(customContextMenuRequested(const QPoint&)), RiuMainWindow::instance(), SLOT(customMenuRequested(const QPoint&)));

    // Property view
    m_propertyView = new caf::PdmUiPropertyView;

    connect(m_projectTreeView, SIGNAL(selectedObjectChanged(caf::PdmObjectHandle*)), m_propertyView, SLOT(showProperties(caf::PdmObjectHandle*)));

    QWidget* propertyEditorWithHeader = new QWidget;
    {
        QLabel* propertyHeader = new QLabel;
        propertyHeader->setText("Property Editor");
        propertyHeader->setStyleSheet("QLabel { background-color: #CCCCCC }");
        propertyHeader->setFixedHeight(20);

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->addWidget(propertyHeader);
        layout->addWidget(m_propertyView);

        propertyEditorWithHeader->setLayout(layout);
        propertyEditorWithHeader->setMinimumHeight(150);
    }

    QSplitter* splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_projectTreeView);
    splitter->addWidget(propertyEditorWithHeader);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(splitter);

    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuProjectAndPropertyView::setPdmItem(caf::PdmUiItem* object)
{
    m_propertyView->showProperties(nullptr);
    m_projectTreeView->setPdmItem(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuProjectAndPropertyView::showProperties(caf::PdmObjectHandle* object)
{
    m_propertyView->showProperties(object);
}
