#include "CadIrDocument.h"

bool CadIrDocument::loadFromJson(const std::string& jsonText)
{
    // Stub only. Replace with a real JSON parser + schema validation.
    m_partName = jsonText.empty() ? "" : "parsed_part";
    m_units = "mm";
    m_features.clear();
    return !jsonText.empty();
}

const std::string& CadIrDocument::partName() const
{
    return m_partName;
}

const std::string& CadIrDocument::units() const
{
    return m_units;
}

const std::vector<CadIrFeature>& CadIrDocument::features() const
{
    return m_features;
}
