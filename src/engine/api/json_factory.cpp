#include "engine/api/json_factory.hpp"

#include "engine/guidance/route_step.hpp"
#include "engine/guidance/step_maneuver.hpp"
#include "engine/guidance/route_leg.hpp"
#include "engine/guidance/route.hpp"
#include "engine/guidance/leg_geometry.hpp"
#include "engine/polyline_compressor.hpp"
#include "engine/hint.hpp"

#include <boost/assert.hpp>
#include <boost/range/irange.hpp>
#include <boost/optional.hpp>

#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <vector>

namespace osrm
{
namespace engine
{
namespace api
{
namespace json
{
namespace detail
{

std::string instructionToString(extractor::TurnInstruction instruction)
{
    std::string token;
    // FIXME this could be an array.
    switch (instruction)
    {
    case extractor::TurnInstruction::GoStraight:
        token = "continue";
        break;
    case extractor::TurnInstruction::TurnSlightRight:
        token = "bear right";
        break;
    case extractor::TurnInstruction::TurnRight:
        token = "right";
        break;
    case extractor::TurnInstruction::TurnSharpRight:
        token = "sharp right";
        break;
    case extractor::TurnInstruction::UTurn:
        token = "uturn";
        break;
    case extractor::TurnInstruction::TurnSharpLeft:
        token = "sharp left";
        break;
    case extractor::TurnInstruction::TurnLeft:
        token = "left";
        break;
    case extractor::TurnInstruction::TurnSlightLeft:
        token = "bear left";
        break;
    case extractor::TurnInstruction::HeadOn:
        token = "head on";
        break;
    case extractor::TurnInstruction::EnterRoundAbout:
        token = "enter roundabout";
        break;
    case extractor::TurnInstruction::LeaveRoundAbout:
        token = "leave roundabout";
        break;
    case extractor::TurnInstruction::StayOnRoundAbout:
        token = "stay on roundabout";
        break;
    case extractor::TurnInstruction::StartAtEndOfStreet:
        token = "depart";
        break;
    case extractor::TurnInstruction::ReachedYourDestination:
        token = "arrive";
        break;
    case extractor::TurnInstruction::NameChanges:
        token = "name changed";
        break;

    case extractor::TurnInstruction::NoTurn:
    case extractor::TurnInstruction::ReachViaLocation:
    case extractor::TurnInstruction::EnterAgainstAllowedDirection:
    case extractor::TurnInstruction::LeaveAgainstAllowedDirection:
    case extractor::TurnInstruction::InverseAccessRestrictionFlag:
    case extractor::TurnInstruction::AccessRestrictionFlag:
    case extractor::TurnInstruction::AccessRestrictionPenalty:
        BOOST_ASSERT_MSG(false, "Invalid turn type used");
        break;

    default:
        BOOST_ASSERT_MSG(false, "unknown TurnInstruction");
        break;
    }

    return token;
}

util::json::Array coordinateToLonLat(const util::Coordinate &coordinate)
{
    util::json::Array array;
    array.values.push_back(static_cast<double>(toFloating(coordinate.lon)));
    array.values.push_back(static_cast<double>(toFloating(coordinate.lat)));
    return array;
}

// FIXME this actually needs to be configurable from the profiles
std::string modeToString(const extractor::TravelMode mode)
{
    std::string token;
    switch (mode)
    {
    case TRAVEL_MODE_INACCESSIBLE:
        token = "inaccessible";
        break;
    case TRAVEL_MODE_DRIVING:
        token = "driving";
        break;
    case TRAVEL_MODE_CYCLING:
        token = "cycling";
        break;
    case TRAVEL_MODE_WALKING:
        token = "walking";
        break;
    case TRAVEL_MODE_FERRY:
        token = "ferry";
        break;
    case TRAVEL_MODE_TRAIN:
        token = "train";
        break;
    case TRAVEL_MODE_PUSHING_BIKE:
        token = "pushing bike";
        break;
    case TRAVEL_MODE_MOVABLE_BRIDGE:
        token = "movable bridge";
        break;
    case TRAVEL_MODE_STEPS_UP:
        token = "steps up";
        break;
    case TRAVEL_MODE_STEPS_DOWN:
        token = "steps down";
        break;
    case TRAVEL_MODE_RIVER_UP:
        token = "river upstream";
        break;
    case TRAVEL_MODE_RIVER_DOWN:
        token = "river downstream";
        break;
    case TRAVEL_MODE_ROUTE:
        token = "rout";
        break;
    default:
        token = "other";
        break;
    }
    return token;
}

} // namespace detail

util::json::Object makeStepManeuver(const guidance::StepManeuver &maneuver)
{
    util::json::Object step_maneuver;
    step_maneuver.values["type"] = detail::instructionToString(maneuver.instruction);
    step_maneuver.values["location"] = detail::coordinateToLonLat(maneuver.location);
    step_maneuver.values["bearing_before"] = maneuver.bearing_before;
    step_maneuver.values["bearing_after"] = maneuver.bearing_after;
    return step_maneuver;
}

util::json::Object makeRouteStep(guidance::RouteStep &&step, util::json::Value geometry)
{
    util::json::Object route_step;
    route_step.values["distance"] = step.distance;
    route_step.values["duration"] = step.duration;
    route_step.values["name"] = std::move(step.name);
    route_step.values["mode"] = detail::modeToString(step.mode);
    route_step.values["maneuver"] = makeStepManeuver(step.maneuver);
    route_step.values["geometry"] = std::move(geometry);
    return route_step;
}

util::json::Object makeRoute(const guidance::Route &route,
                             util::json::Array &&legs,
                             boost::optional<util::json::Value> geometry)
{
    util::json::Object json_route;
    json_route.values["distance"] = route.distance;
    json_route.values["duration"] = route.duration;
    json_route.values["legs"] = std::move(legs);
    if (geometry)
    {
        json_route.values["geometry"] = std::move(*geometry);
    }
    return json_route;
}

util::json::Object
makeWaypoint(const util::Coordinate location, std::string &&name, const Hint &hint)
{
    util::json::Object waypoint;
    waypoint.values["location"] = detail::coordinateToLonLat(location);
    waypoint.values["name"] = std::move(name);
    waypoint.values["hint"] = hint.ToBase64();
    return waypoint;
}

util::json::Object makeRouteLeg(guidance::RouteLeg &&leg, util::json::Array &&steps)
{
    util::json::Object route_leg;
    route_leg.values["distance"] = leg.distance;
    route_leg.values["duration"] = leg.duration;
    route_leg.values["summary"] = std::move(leg.summary);
    route_leg.values["steps"] = std::move(steps);
    return route_leg;
}

util::json::Array makeRouteLegs(std::vector<guidance::RouteLeg> &&legs,
                                std::vector<util::json::Value> step_geometries)
{
    util::json::Array json_legs;
    auto step_geometry_iter = step_geometries.begin();
    for (const auto idx : boost::irange(0UL, legs.size()))
    {
        auto &&leg = std::move(legs[idx]);
        util::json::Array json_steps;
        json_steps.values.reserve(leg.steps.size());
        std::transform(
            std::make_move_iterator(leg.steps.begin()), std::make_move_iterator(leg.steps.end()),
            std::back_inserter(json_steps.values), [&step_geometry_iter](guidance::RouteStep &&step)
            {
                return makeRouteStep(std::move(step), std::move(*step_geometry_iter++));
            });
        json_legs.values.push_back(makeRouteLeg(std::move(leg), std::move(json_steps)));
    }

    return json_legs;
}
}
}
} // namespace engine
} // namespace osrm
