#include "FeatureBase.h"

FeatureBase::FeatureBase(std::string id)
    : m_id(std::move(id))
{
}

FeatureBase::~FeatureBase() = default;

const std::string& FeatureBase::id() const
{
    return m_id;
}

const std::vector<std::string>& FeatureBase::dependencies() const
{
    return m_dependencies;
}

void FeatureBase::addDependency(std::string featureId)
{
    m_dependencies.push_back(std::move(featureId));
}
