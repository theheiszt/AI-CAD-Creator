#pragma once

#include <string>
#include <vector>

namespace selectors
{

enum class SelectorKind
{
    MaxZPlanarFace,
    MinZPlanarFace,
    LargestPlanarFace,
    VerticalPlanarFaces,
    AllFaces
};

struct SelectorQuery
{
    SelectorKind kind = SelectorKind::AllFaces;
    std::string rule;
};

struct NamedRegion
{
    std::string name;
    SelectorQuery query;
    std::string description;
    bool multiMatch = false;
};

std::string toRuleString(SelectorKind kind);
std::vector<NamedRegion> demoPrismNamedRegions();

} // namespace selectors
