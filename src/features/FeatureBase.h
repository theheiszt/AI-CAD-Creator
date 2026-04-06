#pragma once

#include <string>
#include <vector>

class FeatureBase
{
public:
    explicit FeatureBase(std::string id);
    virtual ~FeatureBase();

    const std::string& id() const;
    const std::vector<std::string>& dependencies() const;

    virtual std::string typeName() const = 0;
    virtual bool execute() = 0;

protected:
    void addDependency(std::string featureId);

private:
    std::string m_id;
    std::vector<std::string> m_dependencies;
};
