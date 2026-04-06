#include "CadApplication.h"

#include "document/OcafDocument.h"
#include "viewer/ViewerWindow.h"

#include <QApplication>

CadApplication::CadApplication(int& argc, char** argv)
    : m_qtApp(std::make_unique<QApplication>(argc, argv))
    , m_mainWindow(std::make_unique<ViewerWindow>())
    , m_document(std::make_unique<OcafDocument>())
{
    m_document->createEmpty();
}

CadApplication::~CadApplication() = default;

int CadApplication::run()
{
    m_mainWindow->setWindowTitle("AI CAD Creator");
    m_mainWindow->resize(1400, 900);
    m_mainWindow->show();
    return m_qtApp->exec();
}
