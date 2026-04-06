#include "OcafDocument.h"

#ifdef AICAD_WITH_OCCT
#include <BinXCAFDrivers.hxx>
#include <Handle_TDocStd_Document.hxx>
#include <TDocStd_Application.hxx>
#include <XCAFApp_Application.hxx>

class OcafDocument::Impl
{
public:
    Handle(TDocStd_Application) app;
    Handle(TDocStd_Document) doc;
};
#endif

OcafDocument::OcafDocument()
#ifdef AICAD_WITH_OCCT
    : m_impl(new Impl())
#else
    : m_impl(nullptr)
#endif
{
}

OcafDocument::~OcafDocument()
{
#ifdef AICAD_WITH_OCCT
    delete m_impl;
#endif
}

bool OcafDocument::createEmpty()
{
#ifdef AICAD_WITH_OCCT
    if (m_impl == nullptr)
    {
        return false;
    }

    m_impl->app = XCAFApp_Application::GetApplication();
    BinXCAFDrivers::DefineFormat(m_impl->app);
    if (m_impl->app.IsNull())
    {
        return false;
    }

    m_impl->app->NewDocument("BinXCAF", m_impl->doc);
    return !m_impl->doc.IsNull();
#else
    return true;
#endif
}

bool OcafDocument::saveAs(const std::string&)
{
    return false;
}

bool OcafDocument::open(const std::string&)
{
    return false;
}
