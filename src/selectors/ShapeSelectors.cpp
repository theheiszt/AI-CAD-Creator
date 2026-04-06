#include "ShapeSelectors.h"

#ifdef AICAD_WITH_OCCT
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Dir.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <algorithm>
#include <cmath>
#include <limits>

namespace selectors
{
namespace
{
constexpr double kDirectionTolerance = 1.0e-7;

std::vector<TopoDS_Face> collectFaces(const TopoDS_Shape& shape)
{
    std::vector<TopoDS_Face> faces;
    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next())
    {
        faces.push_back(TopoDS::Face(explorer.Current()));
    }
    return faces;
}

bool isPlanarFace(const TopoDS_Face& face)
{
    const BRepAdaptor_Surface surface(face, Standard_True);
    return surface.GetType() == GeomAbs_Plane;
}

double faceArea(const TopoDS_Face& face)
{
    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    return props.Mass();
}

void faceZRange(const TopoDS_Face& face, double& zMin, double& zMax)
{
    Bnd_Box box;
    BRepBndLib::Add(face, box);
    double xMin = 0.0;
    double yMin = 0.0;
    double xMax = 0.0;
    double yMax = 0.0;
    box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
}

bool isVerticalPlanarFace(const TopoDS_Face& face)
{
    if (!isPlanarFace(face))
    {
        return false;
    }

    const BRepAdaptor_Surface surface(face, Standard_True);
    const gp_Dir normal = surface.Plane().Axis().Direction();
    return std::abs(normal.Z()) < kDirectionTolerance;
}

} // namespace

std::vector<TopoDS_Face> resolveFaces(const TopoDS_Shape& shape, const SelectorQuery& query)
{
    std::vector<TopoDS_Face> faces = collectFaces(shape);
    if (faces.empty())
    {
        return {};
    }

    if (query.kind == SelectorKind::AllFaces)
    {
        return faces;
    }

    if (query.kind == SelectorKind::VerticalPlanarFaces)
    {
        std::vector<TopoDS_Face> matches;
        for (const TopoDS_Face& face : faces)
        {
            if (isVerticalPlanarFace(face))
            {
                matches.push_back(face);
            }
        }
        return matches;
    }

    std::vector<TopoDS_Face> planarFaces;
    for (const TopoDS_Face& face : faces)
    {
        if (isPlanarFace(face))
        {
            planarFaces.push_back(face);
        }
    }

    if (planarFaces.empty())
    {
        return {};
    }

    if (query.kind == SelectorKind::LargestPlanarFace)
    {
        auto it = std::max_element(
            planarFaces.begin(),
            planarFaces.end(),
            [](const TopoDS_Face& a, const TopoDS_Face& b) {
                return faceArea(a) < faceArea(b);
            });
        return { *it };
    }

    if (query.kind == SelectorKind::MaxZPlanarFace)
    {
        double bestZ = -std::numeric_limits<double>::infinity();
        TopoDS_Face bestFace;
        for (const TopoDS_Face& face : planarFaces)
        {
            double zMin = 0.0;
            double zMax = 0.0;
            faceZRange(face, zMin, zMax);
            if (zMax > bestZ)
            {
                bestZ = zMax;
                bestFace = face;
            }
        }
        return bestFace.IsNull() ? std::vector<TopoDS_Face>{} : std::vector<TopoDS_Face>{bestFace};
    }

    if (query.kind == SelectorKind::MinZPlanarFace)
    {
        double bestZ = std::numeric_limits<double>::infinity();
        TopoDS_Face bestFace;
        for (const TopoDS_Face& face : planarFaces)
        {
            double zMin = 0.0;
            double zMax = 0.0;
            faceZRange(face, zMin, zMax);
            if (zMin < bestZ)
            {
                bestZ = zMin;
                bestFace = face;
            }
        }
        return bestFace.IsNull() ? std::vector<TopoDS_Face>{} : std::vector<TopoDS_Face>{bestFace};
    }

    return {};
}

} // namespace selectors
#endif
