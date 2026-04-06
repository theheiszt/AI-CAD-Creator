#include "GeometryOps.h"

#ifdef AICAD_WITH_OCCT
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

TopoDS_Shape GeometryOps::makeDemoPrism()
{
    const gp_Pnt p1(0.0, 0.0, 0.0);
    const gp_Pnt p2(100.0, 0.0, 0.0);
    const gp_Pnt p3(100.0, 50.0, 0.0);
    const gp_Pnt p4(0.0, 50.0, 0.0);

    BRepBuilderAPI_MakeWire wireBuilder;
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3));
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p3, p4));
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p4, p1));

    const TopoDS_Wire profileWire = wireBuilder.Wire();
    const TopoDS_Face profileFace = BRepBuilderAPI_MakeFace(profileWire);
    return BRepPrimAPI_MakePrism(profileFace, gp_Vec(0.0, 0.0, 25.0)).Shape();
}

bool GeometryOps::exportStep(const TopoDS_Shape& shape, const std::string& path)
{
    if (shape.IsNull() || path.empty())
    {
        return false;
    }

    STEPControl_Writer writer;
    const IFSelect_ReturnStatus transferStatus = writer.Transfer(shape, STEPControl_AsIs);
    if (transferStatus != IFSelect_RetDone)
    {
        return false;
    }

    return writer.Write(path.c_str()) == IFSelect_RetDone;
}
#else
bool GeometryOps::exportStep(const std::string&)
{
    return false;
}
#endif
