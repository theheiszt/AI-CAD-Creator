#include "ExtrudeFeature.h"

ExtrudeFeature::ExtrudeFeature(std::string id, std::string profileId, double distanceMm)
    : FeatureBase(std::move(id))
    , m_profileId(std::move(profileId))
    , m_distanceMm(distanceMm)
{
    addDependency(m_profileId);
}

std::string ExtrudeFeature::typeName() const
{
    return "extrude";
}

bool ExtrudeFeature::execute()
{
    // Replace with real OCCT profile->prism logic.
    return m_distanceMm > 0.0 && !m_profileId.empty();
}
