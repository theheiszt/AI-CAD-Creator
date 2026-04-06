#pragma once

#include "selectors/SelectorTypes.h"

#include <vector>

#ifdef AICAD_WITH_OCCT
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#endif

namespace selectors
{

#ifdef AICAD_WITH_OCCT
std::vector<TopoDS_Face> resolveFaces(const TopoDS_Shape& shape, const SelectorQuery& query);
#endif

} // namespace selectors
