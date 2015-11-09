#pragma once

#include <QtGui/QMainWindow>
#include <QAbstractItemModel>
#include <QItemSelection>

class DemoPdmObject;
class QTreeView;
class QUndoView;

namespace caf
{
    class PdmObjectGroup;
    class PdmObjectHandle;
    class UiTreeModelPdm;
    class PdmUiPropertyView;
    class PdmUiTreeView;
    class PdmUiTableView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    static MainWindow* instance();
    void setPdmRoot(caf::PdmObjectHandle* pdmRoot);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockPanels();


    void buildTestModel();
    void releaseTestData();

private slots:
    void slotInsert();
    void slotRemove();
    void slotRemoveAll();
    void slotSelectionChanged(const QItemSelection &, const QItemSelection & );
    void slotSimpleSelectionChanged();
    void slotShowTableView();


private:
    static MainWindow* sm_mainWindowInstance;

private:
    QTreeView*                  m_treeView;
    QUndoView*                  undoView;

    caf::UiTreeModelPdm*        m_treeModelPdm;
    caf::PdmUiTreeView*         m_pdmUiTreeView;
    caf::PdmUiTreeView*         m_pdmUiTreeView2;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;
    caf::PdmUiTableView*        m_pdmUiTableView;
    caf::PdmObjectGroup*        m_testRoot;

};

