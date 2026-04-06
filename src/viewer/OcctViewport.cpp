#include "OcctViewport.h"

#include "geom/GeometryOps.h"
#include "selectors/SelectorTypes.h"
#include "selectors/ShapeSelectors.h"

#include <QPaintEngine>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>

#ifdef AICAD_WITH_OCCT
#include <AIS_ColoredShape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Window.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_TypeOfColor.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif
#endif

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
};

OcctViewport::OcctViewport(QWidget* parent)
    : QWidget(parent)
    , m_impl(new Impl())
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAutoFillBackground(false);
    setMinimumSize(640, 480);
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
    m_impl->view->FitAll(0.01, false);
    m_impl->view->ZFitAll(0.01);
    m_impl->view->Redraw();
    return true;
#else
    return false;
#endif
}
