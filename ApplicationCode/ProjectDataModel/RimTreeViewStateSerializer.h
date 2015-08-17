#pragma once
 
#include <QModelIndex>

class QTreeView;
class QString;
class QAbstractItemModel;

class RimTreeViewStateSerializer
{
public:
    static void applyTreeViewStateFromString(QTreeView* treeView, const QString& treeViewState);
    static void storeTreeViewStateToString  (const QTreeView* treeView, QString& treeViewState);

    static QModelIndex getModelIndexFromString(QAbstractItemModel* model, const QString& currentIndexString);
    static void encodeStringFromModelIndex(const QModelIndex mi, QString& currentIndexString);
};

