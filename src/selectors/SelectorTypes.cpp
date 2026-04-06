#include "SelectorTypes.h"

namespace selectors
{

std::string toRuleString(SelectorKind kind)
{
    switch (kind)
    {
    case SelectorKind::MaxZPlanarFace:
        return "max_z_planar_face";
    case SelectorKind::MinZPlanarFace:
        return "min_z_planar_face";
    case SelectorKind::LargestPlanarFace:
        return "largest_planar_face";
    case SelectorKind::VerticalPlanarFaces:
        return "vertical_planar_faces";
    case SelectorKind::AllFaces:
    default:
        return "all_faces";
    }
}

std::vector<NamedRegion> demoPrismNamedRegions()
{
    return {
        {
            "top_face",
            {SelectorKind::MaxZPlanarFace, toRuleString(SelectorKind::MaxZPlanarFace)},
            "Highest planar face. Useful as an edit target or manufacturing reference.",
            false
        },
        {
            "bottom_face",
            {SelectorKind::MinZPlanarFace, toRuleString(SelectorKind::MinZPlanarFace)},
            "Lowest planar face. Useful as a base plane or fixture reference.",
            false
        },
        {
            "largest_planar_face",
            {SelectorKind::LargestPlanarFace, toRuleString(SelectorKind::LargestPlanarFace)},
            "Largest planar face on the solid. Good fallback when topology changes.",
            false
        },
        {
            "side_faces",
            {SelectorKind::VerticalPlanarFaces, toRuleString(SelectorKind::VerticalPlanarFaces)},
            "All vertical planar side faces on the extruded prism.",
            true
        },
        {
            "all_faces",
            {SelectorKind::AllFaces, toRuleString(SelectorKind::AllFaces)},
            "Every face on the current solid.",
            true
        }
    };
}

} // namespace selectors
