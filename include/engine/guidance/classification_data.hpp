#ifndef OSRM_GUIDANCE_CLASSIFICATION_DATA_HPP_
#define OSRM_GUIDANCE_CLASSIFICATION_DATA_HPP_

#include "util/simple_logger.hpp"

#include <string>
#include <unordered_map>


// Forward Declaration to allow usage of external osmium::Way
namespace osmium
{
class Way;
}

namespace osrm
{
namespace engine
{
namespace guidance
{

enum class FunctionalRoadClass : short
{
    UNKNOWN = 0,
    MOTORWAY,
    MOTORWAY_LINK,
    TRUNK,
    TRUNK_LINK,
    PRIMARY,
    PRIMARY_LINK,
    SECONDARY,
    SECONDARY_LINK,
    TERTIARY,
    TERTIARY_LINK,
    UNCLASSIFIED,
    RESIDENTIAL,
    SERVICE,
    LIVING_STREET,
    LOW_PRIORITY_ROAD // a road simply included for connectivity. Should be avoided at all cost
};

inline FunctionalRoadClass functionalRoadClassFromTag(std::string const &value)
{
    const static auto initializeClassHash = []()
    {
        std::unordered_map<std::string, FunctionalRoadClass> hash;
        hash["motorway"] = FunctionalRoadClass::MOTORWAY;
        hash["motorway_link"] = FunctionalRoadClass::MOTORWAY_LINK;
        hash["trunk"] = FunctionalRoadClass::TRUNK;
        hash["trunk_link"] = FunctionalRoadClass::TRUNK_LINK;
        hash["primary"] = FunctionalRoadClass::PRIMARY;
        hash["primary_link"] = FunctionalRoadClass::PRIMARY_LINK;
        hash["secondary"] = FunctionalRoadClass::SECONDARY;
        hash["secondary_link"] = FunctionalRoadClass::SECONDARY_LINK;
        hash["tertiary"] = FunctionalRoadClass::TERTIARY;
        hash["tertiary_link"] = FunctionalRoadClass::TERTIARY_LINK;
        hash["unclassified"] = FunctionalRoadClass::UNCLASSIFIED;
        hash["residential"] = FunctionalRoadClass::RESIDENTIAL;
        hash["service"] = FunctionalRoadClass::SERVICE;
        hash["living_street"] = FunctionalRoadClass::LIVING_STREET;
        hash["track"] = FunctionalRoadClass::LOW_PRIORITY_ROAD;
        hash["road"] = FunctionalRoadClass::LOW_PRIORITY_ROAD;
        hash["path"] = FunctionalRoadClass::LOW_PRIORITY_ROAD;
        hash["driveway"] = FunctionalRoadClass::LOW_PRIORITY_ROAD;
        return hash;
    };

    static const std::unordered_map<std::string, FunctionalRoadClass> class_hash =
        initializeClassHash();

    if (class_hash.find(value) != class_hash.end())
    {
        return class_hash.find(value)->second;
    }
    else
    {
        util::SimpleLogger().Write(logDEBUG) << "Unknown road class encountered: " << value;
        return FunctionalRoadClass::UNKNOWN;
    }
}

inline bool isRampClass(const FunctionalRoadClass road_class)
{
    // Primary Roads and down are usually too small to announce their links as ramps
    return road_class == FunctionalRoadClass::MOTORWAY_LINK || road_class == FunctionalRoadClass::TRUNK_LINK;
    //|| road_class == PRIMARY_LINK ||
    //   road_class == SECONDARY_LINK || road_class == TERTIARY_LINK;
}

// TODO augment this with all data required for guidance generation
// TODO built a feature ID structure to keep features
const constexpr unsigned INVALID_FEATURE_ID = 0;
struct RoadClassificationData
{
    FunctionalRoadClass road_class = FunctionalRoadClass::UNKNOWN;
    unsigned feature_id = INVALID_FEATURE_ID;

    void augment(const osmium::Way &way);
};

inline bool operator==( const RoadClassificationData lhs, const RoadClassificationData rhs )
{
  return lhs.road_class == rhs.road_class;
}

} // namespace guidance
} // namespace engine
} // namespace osrm

#endif // OSRM_GUIDANCE_CLASSIFICATION_DATA_HPP_
