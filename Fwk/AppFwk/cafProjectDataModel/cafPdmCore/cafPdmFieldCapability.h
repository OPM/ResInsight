#pragma once

#include <vector>

class QString;

namespace caf
{
class PdmFieldCapability
{
public:
    PdmFieldCapability() {}
    virtual ~PdmFieldCapability() {}

    virtual std::vector<std::pair<QString, QString>> attributes() const { return {}; }
    virtual void setAttributes( const std::vector<std::pair<QString, QString>>& attributes ) {}
};

} // End of namespace caf
