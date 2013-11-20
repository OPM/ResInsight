#pragma once

#include <QtGui/QMainWindow>
#include <QAbstractItemModel>
#include <QItemSelection>

class DemoPdmObject;
class QTreeView;

namespace caf
{
    class PdmObjectGroup;
    class PdmObject;
    class UiTreeModelPdm;
    class PdmUiPropertyView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    static MainWindow* instance();
    void setPdmRoot(caf::PdmObject* pdmRoot);

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


private:
    static MainWindow* sm_mainWindowInstance;

private:
    QTreeView*                  m_treeView;
    caf::UiTreeModelPdm*        m_treeModelPdm;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;
    caf::PdmObjectGroup*        m_testRoot;

};

