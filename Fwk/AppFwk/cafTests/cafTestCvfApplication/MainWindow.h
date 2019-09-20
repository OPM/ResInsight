#pragma once

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QMainWindow>

class QTreeView;
class QUndoView;

class TapProject;

namespace caf
{
class PdmObjectCollection;
class PdmObjectHandle;
class UiTreeModelPdm;
class PdmUiPropertyView;
class PdmUiTreeView;
class PdmUiTableView;
} // namespace caf

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    static MainWindow* instance();
    void               setPdmRoot(caf::PdmObjectHandle* pdmRoot);

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

    void slotSimpleSelectionChanged();
    void slotShowTableView();

private:
    static MainWindow* sm_mainWindowInstance;

private:
    QUndoView* undoView;

    caf::PdmUiTreeView*     m_pdmUiTreeView;
    caf::PdmUiTreeView*     m_pdmUiTreeView2;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmUiTableView*    m_pdmUiTableView;

    TapProject* m_project;
};
