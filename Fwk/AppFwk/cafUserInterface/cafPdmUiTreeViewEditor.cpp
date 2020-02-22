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

#include "cafPdmUiTreeViewEditor.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafPdmUiDragDropInterface.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeViewQModel.h"
#include "cafSelectionManager.h"

#include <QDragMoveEvent>
#include <QEvent>
#include <QGridLayout>
#include <QMenu>
#include <QModelIndexList>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <QWidget>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class PdmUiTreeViewWidget : public QTreeView
{
public:
    explicit PdmUiTreeViewWidget(QWidget* parent = nullptr) : QTreeView(parent) {};
    ~PdmUiTreeViewWidget() override {};

    bool isTreeItemEditWidgetActive() const
    {
        return state() == QAbstractItemView::EditingState;
    }

protected:
    void dragMoveEvent(QDragMoveEvent* event) override
    {
        caf::PdmUiTreeViewQModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewQModel*>(model());
        if (treeViewModel && treeViewModel->dragDropInterface())
        {
            treeViewModel->dragDropInterface()->onProposedDropActionUpdated(event->proposedAction());
        }

        QTreeView::dragMoveEvent(event);
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override
    {
        caf::PdmUiTreeViewQModel* treeViewModel = dynamic_cast<caf::PdmUiTreeViewQModel*>(model());
        if (treeViewModel && treeViewModel->dragDropInterface())
        {
            treeViewModel->dragDropInterface()->onDragCanceled();
        }

        QTreeView::dragLeaveEvent(event);
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::PdmUiTreeViewEditor()
{
    m_useDefaultContextMenu = false;
    m_updateSelectionManager = false;
    m_appendClassNameToUiItemText = false;
    m_layout = nullptr;
    m_treeView = nullptr;
    m_treeViewModel = nullptr;
    m_delegate = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewEditor::~PdmUiTreeViewEditor()
{
    m_treeViewModel->setPdmItemRoot(nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiTreeViewEditor::createWidget(QWidget* parent)
{
    m_mainWidget = new QWidget(parent);
    m_layout     = new QVBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_mainWidget->setLayout(m_layout);

    m_treeViewModel = new caf::PdmUiTreeViewQModel(this);
    m_treeView = new PdmUiTreeViewWidget(m_mainWidget);
    m_treeView->setModel(m_treeViewModel);
    m_treeView->installEventFilter(this);

    m_delegate = new PdmUiTreeViewItemDelegate(m_treeView, m_treeViewModel);

    m_treeView->setItemDelegate(m_delegate);

    connect(treeView()->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotOnSelectionChanged( const QItemSelection & , const QItemSelection & )));

    m_layout->addWidget(m_treeView);

    updateContextMenuSignals();

    return m_mainWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    PdmUiTreeViewEditorAttribute editorAttributes;

    {
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>(this->pdmItemRoot());
        if (uiObjectHandle)
        {
            uiObjectHandle->objectEditorAttribute(uiConfigName, &editorAttributes);            
        }
    }

    m_treeViewModel->setColumnHeaders(editorAttributes.columnHeaders);
    m_treeViewModel->setUiConfigName(uiConfigName);
    m_treeViewModel->setPdmItemRoot(this->pdmItemRoot());

    if (editorAttributes.currentObject)
    {
        PdmUiObjectHandle* uiObjectHandle = editorAttributes.currentObject->uiCapability();
        if (uiObjectHandle)
        {
            selectAsCurrentItem(uiObjectHandle);
        }
    }

    if (m_delegate)
    {
        m_delegate->clearAttributes();
        updateItemDelegateForSubTree();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTreeView* PdmUiTreeViewEditor::treeView()
{
    return m_treeView;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::isTreeItemEditWidgetActive() const
{
    return m_treeView->isTreeItemEditWidgetActive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectedUiItems(std::vector<PdmUiItem*>& objects)
{
    if (!this->treeView()) return;

    QModelIndexList idxList = this->treeView()->selectionModel()->selectedIndexes();

    for (int i = 0; i < idxList.size(); i++)
    {
        caf::PdmUiItem* item = this->m_treeViewModel->uiItemFromModelIndex(idxList[i]);
        if (item)
        {
            objects.push_back(item);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateMySubTree(PdmUiItem* uiItem)
{
    if (m_treeViewModel)
    {
        m_treeViewModel->updateSubTree(uiItem);
        QModelIndex index = m_treeViewModel->findModelIndex(uiItem);
        updateItemDelegateForSubTree(index);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableDefaultContextMenu(bool enable)
{
    m_useDefaultContextMenu = enable;

    updateContextMenuSignals();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableSelectionManagerUpdating(bool enable)
{
    m_updateSelectionManager = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateContextMenuSignals()
{
    if (!m_treeView) return;

    if (m_useDefaultContextMenu)
    {
        m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));
    }
    else
    {
        m_treeView->setContextMenuPolicy(Qt::DefaultContextMenu);
        disconnect(m_treeView, nullptr, this, nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::customMenuRequested(QPoint pos)
{
    // This seems a bit strange. Why ?
    SelectionManager::instance()->setActiveChildArrayFieldHandle(this->currentChildArrayFieldHandle());

    QMenu menu;
    PdmUiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget(m_mainWidget->parentWidget());

    caf::PdmUiCommandSystemProxy::instance()->populateMenuWithDefaultCommands("PdmUiTreeViewEditor", &menu);

    if (menu.actions().size() > 0)
    {
        // Qt doc: QAbstractScrollArea and its subclasses that map the context menu event to coordinates of the viewport().
        QPoint globalPos = m_treeView->viewport()->mapToGlobal(pos);

        menu.exec(globalPos);
    }

    PdmUiCommandSystemProxy::instance()->setCurrentContextMenuTargetWidget(nullptr);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmChildArrayFieldHandle* PdmUiTreeViewEditor::currentChildArrayFieldHandle()
{
    PdmUiItem* currentSelectedItem = SelectionManager::instance()->selectedItem(SelectionManager::FIRST_LEVEL);

    PdmUiFieldHandle* uiFieldHandle = dynamic_cast<PdmUiFieldHandle*>(currentSelectedItem);
    if (uiFieldHandle)
    {
        PdmFieldHandle* fieldHandle = uiFieldHandle->fieldHandle();

        if (dynamic_cast<PdmChildArrayFieldHandle*>(fieldHandle))
        {
            return dynamic_cast<PdmChildArrayFieldHandle*>(fieldHandle);
        }
    }

    PdmObjectHandle* pdmObject = dynamic_cast<caf::PdmObjectHandle*>(currentSelectedItem);
    if (pdmObject)
    {
        PdmChildArrayFieldHandle* parentChildArray = dynamic_cast<PdmChildArrayFieldHandle*>(pdmObject->parentField());

        if (parentChildArray)
        {
            return parentChildArray;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectAsCurrentItem(const PdmUiItem* uiItem)
{
    QModelIndex index = m_treeViewModel->findModelIndex(uiItem);
    QModelIndex currentIndex = m_treeView->currentIndex();

    m_treeView->clearSelection();

    m_treeView->setCurrentIndex(index);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::selectItems(std::vector<const PdmUiItem*> uiItems)
{
    m_treeView->clearSelection();

    if (uiItems.empty())
    {
        return;
    }

    QModelIndex index = findModelIndex(uiItems.back());
    m_treeView->setCurrentIndex(index);

    for (const PdmUiItem* uiItem : uiItems)
    {
        QModelIndex itemIndex = findModelIndex(uiItem);
        m_treeView->selectionModel()->select(itemIndex, QItemSelectionModel::Select);
    }
    m_treeView->setFocus(Qt::MouseFocusReason);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::slotOnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    this->updateSelectionManager();

    emit selectionChanged();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setExpanded(const PdmUiItem* uiItem, bool doExpand) const
{
    QModelIndex index = m_treeViewModel->findModelIndex(uiItem);
    m_treeView->setExpanded(index, doExpand);

    if (doExpand)
    {
        m_treeView->scrollTo(index);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiItem* PdmUiTreeViewEditor::uiItemFromModelIndex(const QModelIndex& index) const
{
    return m_treeViewModel->uiItemFromModelIndex(index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QModelIndex PdmUiTreeViewEditor::findModelIndex(const PdmUiItem* object) const
{
    return m_treeViewModel->findModelIndex(object);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::setDragDropInterface(PdmUiDragDropInterface* dragDropInterface)
{
    m_treeViewModel->setDragDropInterface(dragDropInterface);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        this->updateSelectionManager();
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateSelectionManager()
{
    if (m_updateSelectionManager)
    {
        std::vector<PdmUiItem*> items;
        this->selectedUiItems(items);
        SelectionManager::instance()->setSelectedItems(items);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::updateItemDelegateForSubTree(const QModelIndex& modelIndex /*= QModelIndex()*/)
{

    auto allIndices = m_treeViewModel->allIndicesRecursive();
    for (QModelIndex index : allIndices)
    {
        PdmUiItem* uiItem = m_treeViewModel->uiItemFromModelIndex(index);
        PdmUiObjectHandle* uiObjectHandle = dynamic_cast<PdmUiObjectHandle*>(uiItem);
        if (uiObjectHandle)
        {
            PdmUiTreeViewItemAttribute attribute;
            uiObjectHandle->objectEditorAttribute("", &attribute);
            if (!attribute.tag.isEmpty())
            {
                m_delegate->addAttribute(index, attribute);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewEditor::enableAppendOfClassNameToUiItemText(bool enable)
{
    m_appendClassNameToUiItemText = enable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmUiTreeViewEditor::isAppendOfClassNameToUiItemTextEnabled()
{
    return m_appendClassNameToUiItemText;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeViewItemDelegate::PdmUiTreeViewItemDelegate(QObject* parent, PdmUiTreeViewQModel* model)
    : QStyledItemDelegate(parent)
    , m_model(model)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::clearAttributes()
{
    m_attributes.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::addAttribute(QModelIndex index, const PdmUiTreeViewItemAttribute& attribute)
{
    m_attributes[index] = attribute;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiTreeViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);


    auto it = m_attributes.find(index);
    if (it == m_attributes.end()) return;

    // Save painter so we can restore it
    painter->save();

    const int insideTopBottomMargins  = 1;
    const int insideleftRightMargins  = 6;
    const int outsideLeftRightMargins = 4;

    QFont font = QApplication::font();
    if (font.pixelSize() > 0)
    {
        font.setPixelSize(std::max(1, font.pixelSize() - 1));
    }
    else
    {
        font.setPointSize(std::max(1, font.pointSize() - 1));
    }
    painter->setFont(font);

    QString text = it->second.tag;
    QColor bgColor = it->second.bgColor;
    QColor fgColor = it->second.fgColor;
    
    QSize textSize(QFontMetrics(font).size(Qt::TextSingleLine, text));
    QRect rect = option.rect;
    QSize fullSize = rect.size();
    int textDiff = (fullSize.height() - textSize.height());

    QRect textRect;
    if (it->second.position == PdmUiTreeViewItemAttribute::AT_END)
    {
        QPoint bottomRight = rect.bottomRight() - QPoint(outsideLeftRightMargins, 0);
        QPoint textBottomRight = bottomRight - QPoint(insideleftRightMargins, textDiff / 2);
        QPoint textTopLeft = textBottomRight - QPoint(textSize.width(), textSize.height());
        textRect = QRect(textTopLeft, textBottomRight);
    }
    else
    {
        QPoint textTopLeft = QPoint(0, rect.topLeft().y()) + QPoint(outsideLeftRightMargins + insideleftRightMargins, + textDiff / 2);
        QPoint textBottomRight = textTopLeft + QPoint(textSize.width(), textSize.height());
        textRect = QRect(textTopLeft, textBottomRight);
    }
    QRect tagRect = textRect.marginsAdded(QMargins(insideleftRightMargins, insideTopBottomMargins, insideleftRightMargins, insideTopBottomMargins));

    QBrush brush(bgColor);

    painter->setBrush(brush);
    painter->setPen(bgColor);
    painter->setRenderHint(QPainter::Antialiasing);
    const double xRoundingRadiusPercent = 50.0;
    const double yRoundingRadiusPercent = 25.0;
    painter->drawRoundedRect(tagRect, xRoundingRadiusPercent, yRoundingRadiusPercent, Qt::RelativeSize);
 
    painter->setPen(fgColor);
    painter->drawText(textRect, Qt::AlignCenter, text);

    // Restore painter
    painter->restore();
}

} // end namespace caf
