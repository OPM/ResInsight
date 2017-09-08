#pragma once

#include <QtGui/QMainWindow>
#include <QAbstractItemModel>
#include <QItemSelection>

class DemoPdmObject;
class QTreeView;
class QUndoView;
class QLabel;

namespace caf
{
    class PdmObjectCollection;
    class PdmObjectHandle;
    class UiTreeModelPdm;
    class PdmUiPropertyView;
    class PdmUiTreeView;
    class PdmUiTableView;
    class CustomObjectEditor;
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

    void slotSimpleSelectionChanged();
    void slotShowTableView();


private:
    static MainWindow* sm_mainWindowInstance;

private:
    QUndoView*                  undoView;

    caf::PdmUiTreeView*         m_pdmUiTreeView;
    caf::PdmUiTreeView*         m_pdmUiTreeView2;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;
    caf::PdmUiTableView*        m_pdmUiTableView;
    caf::PdmObjectCollection*   m_testRoot;

    caf::CustomObjectEditor*    m_customObjectEditor;

    QLabel*                     m_plotLabel;
    QLabel*                     m_smallPlotLabel;
};

