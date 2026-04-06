#pragma once

#include <memory>

class QApplication;
class ViewerWindow;
class OcafDocument;

class CadApplication
{
public:
    CadApplication(int& argc, char** argv);
    ~CadApplication();

    int run();

private:
    std::unique_ptr<QApplication> m_qtApp;
    std::unique_ptr<ViewerWindow> m_mainWindow;
    std::unique_ptr<OcafDocument> m_document;
};
