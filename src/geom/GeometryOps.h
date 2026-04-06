#pragma once

#include <string>

#ifdef AICAD_WITH_OCCT
#include <TopoDS_Shape.hxx>
#endif

class GeometryOps
{
public:
#ifdef AICAD_WITH_OCCT
    static TopoDS_Shape makeDemoPrism();
    static bool exportStep(const TopoDS_Shape& shape, const std::string& path);
#else
    static bool exportStep(const std::string& path);
#endif
};
