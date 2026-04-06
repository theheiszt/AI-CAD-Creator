#pragma once

#include <string>

class OcafDocument
{
public:
    OcafDocument();
    ~OcafDocument();

    bool createEmpty();
    bool saveAs(const std::string& path);
    bool open(const std::string& path);

private:
#ifdef AICAD_WITH_OCCT
    class Impl;
    Impl* m_impl;
#else
    void* m_impl;
#endif
};
