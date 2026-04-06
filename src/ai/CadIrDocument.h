#pragma once

#include <string>
#include <vector>

struct CadIrFeature
{
    std::string id;
    std::string type;
};

class CadIrDocument
{
public:
    bool loadFromJson(const std::string& jsonText);

    const std::string& partName() const;
    const std::string& units() const;
    const std::vector<CadIrFeature>& features() const;

private:
    std::string m_partName;
    std::string m_units;
    std::vector<CadIrFeature> m_features;
};
