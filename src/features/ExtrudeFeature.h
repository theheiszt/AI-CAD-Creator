#pragma once

#include "FeatureBase.h"

class ExtrudeFeature : public FeatureBase
{
public:
    ExtrudeFeature(std::string id, std::string profileId, double distanceMm);

    std::string typeName() const override;
    bool execute() override;

private:
    std::string m_profileId;
    double m_distanceMm;
};
