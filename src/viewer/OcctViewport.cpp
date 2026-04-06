#include "OcctViewport.h"

#include "geom/GeometryOps.h"
#include "selectors/SelectorTypes.h"
#include "selectors/ShapeSelectors.h"

#include <QMouseEvent>
#include <QPaintEngine>
#include <QPaintEvent>
#include <QPoint>
#include <QResizeEvent>
#include <QShowEvent>
#include <QWheelEvent>

#ifdef AICAD_WITH_OCCT
#include <AIS_ColoredShape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Window.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_TypeOfColor.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif
#endif

namespace
{
enum class NavigationMode
{
    None,
    Rotate,
    Pan,
    Zoom
};

int faceSelectionMode()
{
#ifdef AICAD_WITH_OCCT
    return AIS_Shape::SelectionMode(TopAbs_FACE);
#else
    return 0;
#endif
}
}

class OcctViewport::Impl
{
public:
    bool initialized = false;
    bool demoQueued = false;

#ifdef AICAD_WITH_OCCT
    Handle(Aspect_DisplayConnection) displayConnection;
    Handle(OpenGl_GraphicDriver) graphicDriver;
    Handle(V3d_Viewer) viewer;
    Handle(AIS_InteractiveContext) context;
    Handle(V3d_View) view;
    Handle(Aspect_Window) window;
    Handle(AIS_ColoredShape) displayedShape;
    TopoDS_Shape currentShape;
#endif

    std::vector<selectors::NamedRegion> namedRegions;
    std::string activeRegion;

    NavigationMode navigationMode = NavigationMode::None;
    QPoint pressPos;
    QPoint lastPos;
    bool movedSincePress = false;
};

OcctViewport::OcctViewport(QWidget* parent)
    : QWidget(parent)
    , m_impl(new Impl())
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAutoFillBackground(false);
    setMinimumSize(640, 480);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

OcctViewport::~OcctViewport()
{
    delete m_impl;
}

bool OcctViewport::displayDemoPrism()
{
#ifdef AICAD_WITH_OCCT
    if (!initializeOcct())
    {
        m_impl->demoQueued = true;
        return false;
    }

    m_impl->currentShape = GeometryOps::makeDemoPrism();
    if (m_impl->currentShape.IsNull())
    {
        return false;
    }

    m_impl->namedRegions = selectors::demoPrismNamedRegions();
    m_impl->activeRegion = "top_face";
    return redisplayCurrentShape();
#else
    return false;
#endif
}

bool OcctViewport::exportCurrentShapeStep(const QString& path)
{
#ifdef AICAD_WITH_OCCT
    if (path.isEmpty() || m_impl->currentShape.IsNull())
    {
        return false;
    }

    return GeometryOps::exportStep(m_impl->currentShape, path.toStdString());
#else
    Q_UNUSED(path)
    return false;
#endif
}

QStringList OcctViewport::availableNamedRegions() const
{
    QStringList names;
    for (const auto& region : m_impl->namedRegions)
    {
        names << QString::fromStdString(region.name);
    }
    return names;
}

QString OcctViewport::descriptionForNamedRegion(const QString& name) const
{
    const std::string regionName = name.toStdString();
    for (const auto& region : m_impl->namedRegions)
    {
        if (region.name == regionName)
        {
            return QString::fromStdString(region.description);
        }
    }
    return {};
}

QString OcctViewport::selectorRuleForNamedRegion(const QString& name) const
{
    const std::string regionName = name.toStdString();
    for (const auto& region : m_impl->namedRegions)
    {
        if (region.name == regionName)
        {
            return QString::fromStdString(region.query.rule);
        }
    }
    return {};
}

bool OcctViewport::highlightNamedRegion(const QString& name)
{
    const std::string regionName = name.toStdString();
    for (const auto& region : m_impl->namedRegions)
    {
        if (region.name == regionName)
        {
            m_impl->activeRegion = regionName;
            return redisplayCurrentShape();
        }
    }
    return false;
}

QString OcctViewport::activeNamedRegion() const
{
    return QString::fromStdString(m_impl->activeRegion);
}

void OcctViewport::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (initializeOcct() && m_impl->demoQueued)
    {
        m_impl->demoQueued = false;
        displayDemoPrism();
    }
}

void OcctViewport::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
#ifdef AICAD_WITH_OCCT
    if (m_impl->initialized && !m_impl->view.IsNull())
    {
        m_impl->view->MustBeResized();
    }
#endif
}

void OcctViewport::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
#ifdef AICAD_WITH_OCCT
    if (m_impl->initialized && !m_impl->view.IsNull())
    {
        m_impl->view->Redraw();
    }
#else
    Q_UNUSED(event)
#endif
}

void OcctViewport::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->view.IsNull() || m_impl->context.IsNull())
    {
        return;
    }

    setFocus();
    m_impl->pressPos = event->pos();
    m_impl->lastPos = event->pos();
    m_impl->movedSincePress = false;

    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier))
    {
        m_impl->navigationMode = NavigationMode::Pan;
        emit interactionStatus("Pan: drag with Shift + left mouse.");
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        m_impl->navigationMode = NavigationMode::Rotate;
        m_impl->context->MoveTo(event->position().x(), event->position().y(), m_impl->view, false);
        m_impl->view->StartRotation(event->position().x(), event->position().y(), 0.4);
        emit interactionStatus("Rotate: drag with left mouse. Click to select a face.");
        return;
    }

    if (event->button() == Qt::MiddleButton)
    {
        m_impl->navigationMode = NavigationMode::Pan;
        emit interactionStatus("Pan: drag with middle mouse.");
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        m_impl->navigationMode = NavigationMode::Zoom;
        m_impl->view->StartZoomAtPoint(event->position().x(), event->position().y());
        emit interactionStatus("Zoom: drag with right mouse or use the wheel.");
        return;
    }
#else
    Q_UNUSED(event)
#endif
}

void OcctViewport::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->view.IsNull() || m_impl->context.IsNull())
    {
        return;
    }

    const QPoint currentPos = event->pos();
    if (m_impl->navigationMode == NavigationMode::Rotate && (event->buttons() & Qt::LeftButton))
    {
        if ((currentPos - m_impl->pressPos).manhattanLength() > 2)
        {
            m_impl->movedSincePress = true;
        }
        m_impl->view->Rotation(currentPos.x(), currentPos.y());
        m_impl->view->Redraw();
        m_impl->lastPos = currentPos;
        return;
    }

    if (m_impl->navigationMode == NavigationMode::Pan
        && ((event->buttons() & Qt::MiddleButton) || (event->buttons() & Qt::LeftButton)))
    {
        if ((currentPos - m_impl->pressPos).manhattanLength() > 2)
        {
            m_impl->movedSincePress = true;
        }
        const QPoint delta = currentPos - m_impl->lastPos;
        m_impl->view->Pan(delta.x(), -delta.y(), 1.0, true);
        m_impl->view->Redraw();
        m_impl->lastPos = currentPos;
        return;
    }

    if (m_impl->navigationMode == NavigationMode::Zoom && (event->buttons() & Qt::RightButton))
    {
        if ((currentPos - m_impl->pressPos).manhattanLength() > 2)
        {
            m_impl->movedSincePress = true;
        }
        m_impl->view->ZoomAtPoint(m_impl->pressPos.x(), m_impl->pressPos.y(), currentPos.x(), currentPos.y());
        m_impl->view->Redraw();
        m_impl->lastPos = currentPos;
        return;
    }

    m_impl->context->MoveTo(currentPos.x(), currentPos.y(), m_impl->view, true);
    if (m_impl->context->HasDetected())
    {
        emit interactionStatus("Hover: face detected under cursor.");
    }
    else
    {
        emit interactionStatus("Ready: left drag rotate, middle drag pan, right drag or wheel zoom, left click select.");
    }
#else
    Q_UNUSED(event)
#endif
}

void OcctViewport::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->view.IsNull() || m_impl->context.IsNull())
    {
        return;
    }

    const bool wasClick = !m_impl->movedSincePress
        && (event->button() == Qt::LeftButton)
        && m_impl->navigationMode == NavigationMode::Rotate;

    m_impl->navigationMode = NavigationMode::None;

    if (!wasClick)
    {
        emit interactionStatus("Navigation complete.");
        return;
    }

    m_impl->context->MoveTo(event->position().x(), event->position().y(), m_impl->view, false);
    m_impl->context->SelectDetected();
    m_impl->context->UpdateCurrentViewer();
    m_impl->view->Redraw();

    const QString selectedRegion = namedRegionForFace();
    if (!selectedRegion.isEmpty())
    {
        emit pickedNamedRegion(selectedRegion);
        emit interactionStatus(QString("Selected face mapped to named region '%1'.").arg(selectedRegion));
    }
    else
    {
        emit interactionStatus(describeSelectedShape());
    }
#else
    Q_UNUSED(event)
#endif
}

void OcctViewport::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->view.IsNull())
    {
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        m_impl->view->FitAll(0.01, false);
        m_impl->view->ZFitAll(0.01);
        m_impl->view->Redraw();
        emit interactionStatus("View fit to visible geometry.");
    }
#else
    Q_UNUSED(event)
#endif
}

void OcctViewport::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->view.IsNull())
    {
        return;
    }

    const QPoint angle = event->angleDelta();
    if (angle.y() == 0)
    {
        return;
    }

    const QPoint pos = event->position().toPoint();
    const int zoomOffset = angle.y() > 0 ? -40 : 40;
    m_impl->view->StartZoomAtPoint(pos.x(), pos.y());
    m_impl->view->ZoomAtPoint(pos.x(), pos.y(), pos.x(), pos.y() + zoomOffset);
    m_impl->view->Redraw();
    emit interactionStatus(angle.y() > 0 ? "Zoomed in." : "Zoomed out.");
#else
    Q_UNUSED(event)
#endif
}

QPaintEngine* OcctViewport::paintEngine() const
{
    return nullptr;
}

bool OcctViewport::initializeOcct()
{
#ifdef AICAD_WITH_OCCT
    if (m_impl->initialized)
    {
        return true;
    }

    winId();

    m_impl->displayConnection = new Aspect_DisplayConnection();
    m_impl->graphicDriver = new OpenGl_GraphicDriver(m_impl->displayConnection);
    m_impl->viewer = new V3d_Viewer(m_impl->graphicDriver);
    m_impl->viewer->SetDefaultLights();
    m_impl->viewer->SetLightOn();

    m_impl->context = new AIS_InteractiveContext(m_impl->viewer);
    m_impl->view = m_impl->viewer->CreateView();

#ifdef _WIN32
    m_impl->window = new WNT_Window(reinterpret_cast<Aspect_Handle>(winId()));
#else
    m_impl->window = new Xw_Window(m_impl->displayConnection, static_cast<Aspect_Drawable>(winId()));
#endif

    m_impl->view->SetWindow(m_impl->window);
    if (!m_impl->window->IsMapped())
    {
        m_impl->window->Map();
    }

    m_impl->view->SetBackgroundColor(Quantity_Color(0.16, 0.18, 0.22, Quantity_TOC_RGB));
    m_impl->view->MustBeResized();
    m_impl->view->Redraw();

    m_impl->initialized = true;
    emit interactionStatus("OCCT viewer ready: left drag rotate, middle drag pan, right drag or wheel zoom, left click select.");
    return true;
#else
    return false;
#endif
}

bool OcctViewport::redisplayCurrentShape()
{
#ifdef AICAD_WITH_OCCT
    if (!m_impl->initialized || m_impl->currentShape.IsNull())
    {
        return false;
    }

    if (!m_impl->displayedShape.IsNull())
    {
        m_impl->context->Remove(m_impl->displayedShape, false);
        m_impl->displayedShape.Nullify();
    }

    m_impl->displayedShape = new AIS_ColoredShape(m_impl->currentShape);
    m_impl->displayedShape->SetColor(Quantity_Color(0.75, 0.77, 0.81, Quantity_TOC_RGB));

    if (!m_impl->activeRegion.empty())
    {
        for (const auto& region : m_impl->namedRegions)
        {
            if (region.name != m_impl->activeRegion)
            {
                continue;
            }

            const auto matchedFaces = selectors::resolveFaces(m_impl->currentShape, region.query);
            const Quantity_Color highlightColor(region.multiMatch
                    ? Quantity_Color(0.32, 0.71, 0.52, Quantity_TOC_RGB)
                    : Quantity_Color(0.96, 0.55, 0.18, Quantity_TOC_RGB));

            for (const auto& face : matchedFaces)
            {
                m_impl->displayedShape->SetCustomColor(face, highlightColor);
            }
            break;
        }
    }

    m_impl->context->Display(m_impl->displayedShape, false);
    m_impl->context->SetDisplayMode(m_impl->displayedShape, AIS_Shaded, false);
    m_impl->context->Deactivate(m_impl->displayedShape);
    m_impl->context->Activate(m_impl->displayedShape, faceSelectionMode(), Standard_True);
    m_impl->context->UpdateCurrentViewer();
    m_impl->view->FitAll(0.01, false);
    m_impl->view->ZFitAll(0.01);
    m_impl->view->Redraw();
    return true;
#else
    return false;
#endif
}

QString OcctViewport::namedRegionForFace() const
{
#ifdef AICAD_WITH_OCCT
    if (m_impl->context.IsNull() || !m_impl->context->HasSelectedShape())
    {
        return {};
    }

    const TopoDS_Shape selectedShape = m_impl->context->SelectedShape();
    if (selectedShape.IsNull() || selectedShape.ShapeType() != TopAbs_FACE)
    {
        return {};
    }

    const TopoDS_Face selectedFace = TopoDS::Face(selectedShape);
    for (const auto& region : m_impl->namedRegions)
    {
        const auto matches = selectors::resolveFaces(m_impl->currentShape, region.query);
        for (const auto& face : matches)
        {
            if (face.IsSame(selectedFace))
            {
                return QString::fromStdString(region.name);
            }
        }
    }
#else
    Q_UNUSED(this)
#endif
    return {};
}

QString OcctViewport::describeSelectedShape() const
{
#ifdef AICAD_WITH_OCCT
    if (m_impl->context.IsNull() || !m_impl->context->HasSelectedShape())
    {
        return "No shape selected.";
    }

    const TopoDS_Shape selectedShape = m_impl->context->SelectedShape();
    if (selectedShape.IsNull())
    {
        return "No shape selected.";
    }

    switch (selectedShape.ShapeType())
    {
        case TopAbs_FACE:
            return "Selected a face.";
        case TopAbs_EDGE:
            return "Selected an edge.";
        case TopAbs_VERTEX:
            return "Selected a vertex.";
        default:
            return "Selected a shape.";
    }
#else
    return {};
#endif
}
