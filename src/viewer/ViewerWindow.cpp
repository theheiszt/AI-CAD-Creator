#include "ViewerWindow.h"

#include "viewer/OcctViewport.h"

#include <QAbstractItemView>
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTimer>

ViewerWindow::ViewerWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewport(new OcctViewport(this))
    , m_featureTree(nullptr)
    , m_namedRegionsTree(nullptr)
    , m_chatEdit(nullptr)
{
    setCentralWidget(m_viewport);
    addDockWidget(Qt::LeftDockWidgetArea, createFeatureTreeDock());
    addDockWidget(Qt::RightDockWidgetArea, createNamedRegionsDock());
    addDockWidget(Qt::RightDockWidgetArea, createChatDock());
    createMenus();
    populateFeatureTree();

    statusBar()->showMessage("Initializing OCCT AIS viewer...");
    QTimer::singleShot(0, this, [this]() {
        if (m_viewport->displayDemoPrism())
        {
            refreshNamedRegions();
            m_viewport->highlightNamedRegion("top_face");
            statusBar()->showMessage("Displayed demo prism and loaded named region selectors.");
        }
        else
        {
            statusBar()->showMessage("OCCT viewer initialized, but demo prism could not be displayed.");
        }
    });
}

void ViewerWindow::populateFeatureTree()
{
    if (!m_featureTree)
    {
        return;
    }

    m_featureTree->clear();

    auto* part = new QTreeWidgetItem(m_featureTree, {"Part001", "Part"});
    new QTreeWidgetItem(part, {"Sketch001", "Rectangle Profile"});
    new QTreeWidgetItem(part, {"Extrude001", "Prism / Extrude"});

    auto* regions = new QTreeWidgetItem(part, {"NamedRegions", "Semantic Selectors"});
    new QTreeWidgetItem(regions, {"top_face", "max_z_planar_face"});
    new QTreeWidgetItem(regions, {"bottom_face", "min_z_planar_face"});
    new QTreeWidgetItem(regions, {"largest_planar_face", "largest_planar_face"});
    new QTreeWidgetItem(regions, {"side_faces", "vertical_planar_faces"});

    part->setExpanded(true);
    regions->setExpanded(true);
}

void ViewerWindow::createMenus()
{
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* exportStepAction = new QAction("Export Demo STEP...", this);
    connect(exportStepAction, &QAction::triggered, this, [this]() {
        const QString path = QFileDialog::getSaveFileName(
            this,
            "Export Demo STEP",
            "demo_prism.step",
            "STEP Files (*.step *.stp)");

        if (path.isEmpty())
        {
            return;
        }

        if (m_viewport->exportCurrentShapeStep(path))
        {
            statusBar()->showMessage(QString("Exported STEP to %1").arg(path), 5000);
        }
        else
        {
            statusBar()->showMessage("STEP export failed.", 5000);
        }
    });
    fileMenu->addAction(exportStepAction);
}

void ViewerWindow::refreshNamedRegions()
{
    if (!m_namedRegionsTree)
    {
        return;
    }

    m_namedRegionsTree->clear();
    const QStringList names = m_viewport->availableNamedRegions();
    for (const QString& name : names)
    {
        auto* item = new QTreeWidgetItem(
            m_namedRegionsTree,
            {
                name,
                m_viewport->selectorRuleForNamedRegion(name),
                m_viewport->descriptionForNamedRegion(name)
            });

        if (name == m_viewport->activeNamedRegion())
        {
            m_namedRegionsTree->setCurrentItem(item);
        }
    }

    if (m_namedRegionsTree->topLevelItemCount() > 0 && m_namedRegionsTree->currentItem() == nullptr)
    {
        m_namedRegionsTree->setCurrentItem(m_namedRegionsTree->topLevelItem(0));
    }
}

QDockWidget* ViewerWindow::createFeatureTreeDock()
{
    auto* dock = new QDockWidget("Feature Tree", this);
    m_featureTree = new QTreeWidget(dock);
    m_featureTree->setHeaderLabels({"Feature", "Type"});
    dock->setWidget(m_featureTree);
    return dock;
}

QDockWidget* ViewerWindow::createNamedRegionsDock()
{
    auto* dock = new QDockWidget("Named Regions", this);
    m_namedRegionsTree = new QTreeWidget(dock);
    m_namedRegionsTree->setHeaderLabels({"Region", "Rule", "Description"});
    m_namedRegionsTree->header()->setStretchLastSection(true);
    m_namedRegionsTree->setAlternatingRowColors(true);
    m_namedRegionsTree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_namedRegionsTree, &QTreeWidget::currentItemChanged, this,
        [this](QTreeWidgetItem* current, QTreeWidgetItem*) {
            if (current == nullptr)
            {
                return;
            }

            const QString regionName = current->text(0);
            if (m_viewport->highlightNamedRegion(regionName))
            {
                statusBar()->showMessage(
                    QString("Highlighted named region '%1' using selector '%2'.")
                        .arg(regionName, current->text(1)),
                    4000);
            }
        });

    dock->setWidget(m_namedRegionsTree);
    return dock;
}

QDockWidget* ViewerWindow::createChatDock()
{
    auto* dock = new QDockWidget("AI CAD Chat", this);
    m_chatEdit = new QTextEdit(dock);
    m_chatEdit->setPlaceholderText(
        "Describe the part or edit request here...\n\n"
        "Current demo body: rectangle sketch -> OCCT prism/extrude.\n"
        "The viewer also exposes topology-independent named regions like top_face and side_faces.");
    dock->setWidget(m_chatEdit);
    return dock;
}
