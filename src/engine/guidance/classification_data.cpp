#include "engine/guidance/classification_data.hpp"

#include <osmium/osm.hpp>

#include <string>
#include <iostream>

namespace osrm
{
namespace engine
{
namespace guidance
{
void RoadClassificationData::invalidate()
{
    road_class = FunctionalRoadClass::UNKNOWN;
    feature_id = 0;
}

void RoadClassificationData::augment(const osmium::Way &way)
{
    const char *data = way.get_value_by_key("highway");
    if (data)
        road_class = functionalRoadClassFromTag(data);
    const char *feature = way.get_value_by_key("feature_id");
    if (feature)
        feature_id = std::stoi(feature);
}
} // namespace guidance
} // namespace engine
} // namespace osrm
