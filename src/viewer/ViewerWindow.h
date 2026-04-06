#pragma once

#include <QMainWindow>

class QDockWidget;
class QTextEdit;
class QTreeWidget;
class OcctViewport;

class ViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ViewerWindow(QWidget* parent = nullptr);

private:
    void populateFeatureTree();
    void createMenus();
    void refreshNamedRegions();
    QDockWidget* createFeatureTreeDock();
    QDockWidget* createNamedRegionsDock();
    QDockWidget* createChatDock();

private:
    OcctViewport* m_viewport;
    QTreeWidget* m_featureTree;
    QTreeWidget* m_namedRegionsTree;
    QTextEdit* m_chatEdit;
};
