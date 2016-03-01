#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "util/coordinate_calculation.hpp"

#include <boost/assert.hpp>

#include "osrm/coordinate.hpp"

#include <iomanip>
#include <algorithm>
#include <cstdint>
#include <limits>

namespace osrm
{
namespace util
{

// TODO: Make template type, add tests
struct RectangleInt2D
{
    RectangleInt2D()
        : min_lon(std::numeric_limits<int32_t>::max()),
          max_lon(std::numeric_limits<int32_t>::min()),
          min_lat(std::numeric_limits<int32_t>::max()), max_lat(std::numeric_limits<int32_t>::min())
    {
    }

    RectangleInt2D(FixedLongitude min_lon_, FixedLongitude max_lon_, FixedLatitude min_lat_, FixedLatitude max_lat_)
        : min_lon(min_lon_), max_lon(max_lon_), min_lat(min_lat_), max_lat(max_lat_)
    {
    }

    RectangleInt2D(FloatLongitude min_lon_, FloatLongitude max_lon_, FloatLatitude min_lat_, FloatLatitude max_lat_)
        : min_lon(toFixed(min_lon_)), max_lon(toFixed(max_lon_)), min_lat(toFixed(min_lat_)), max_lat(toFixed(max_lat_))
    {
    }

    FixedLongitude min_lon, max_lon;
    FixedLatitude min_lat, max_lat;

    void MergeBoundingBoxes(const RectangleInt2D &other)
    {
        min_lon = std::min(min_lon, other.min_lon);
        max_lon = std::max(max_lon, other.max_lon);
        min_lat = std::min(min_lat, other.min_lat);
        max_lat = std::max(max_lat, other.max_lat);
        BOOST_ASSERT(min_lon != FixedLongitude(std::numeric_limits<int32_t>::min()));
        BOOST_ASSERT(min_lat != FixedLatitude(std::numeric_limits<int32_t>::min()));
        BOOST_ASSERT(max_lon != FixedLongitude(std::numeric_limits<int32_t>::min()));
        BOOST_ASSERT(max_lat != FixedLatitude(std::numeric_limits<int32_t>::min()));
    }

    Coordinate Centroid() const
    {
        Coordinate centroid;
        // The coordinates of the midpoints are given by:
        // x = (x1 + x2) /2 and y = (y1 + y2) /2.
        centroid.lon = (min_lon + max_lon) / FixedLongitude(2);
        centroid.lat = (min_lat + max_lat) / FixedLatitude(2);
        return centroid;
    }

    bool Intersects(const RectangleInt2D &other) const
    {
        Coordinate upper_left(other.min_lon, other.max_lat);
        Coordinate upper_right(other.max_lon, other.max_lat);
        Coordinate lower_right(other.max_lon, other.min_lat);
        Coordinate lower_left(other.min_lon, other.min_lat);

        return (Contains(upper_left) || Contains(upper_right) || Contains(lower_right) ||
                Contains(lower_left));
    }

    double GetMinDist(const Coordinate location) const
    {
        const bool is_contained = Contains(location);
        if (is_contained)
        {
            return 0.0f;
        }

        enum Direction
        {
            INVALID = 0,
            NORTH = 1,
            SOUTH = 2,
            EAST = 4,
            NORTH_EAST = 5,
            SOUTH_EAST = 6,
            WEST = 8,
            NORTH_WEST = 9,
            SOUTH_WEST = 10
        };

        Direction d = INVALID;
        if (location.lat > max_lat)
            d = (Direction)(d | NORTH);
        else if (location.lat < min_lat)
            d = (Direction)(d | SOUTH);
        if (location.lon > max_lon)
            d = (Direction)(d | EAST);
        else if (location.lon < min_lon)
            d = (Direction)(d | WEST);

        BOOST_ASSERT(d != INVALID);

        double min_dist = std::numeric_limits<double>::max();
        switch (d)
        {
        case NORTH:
            min_dist = coordinate_calculation::greatCircleDistance(
                location, Coordinate(location.lon, max_lat));
            break;
        case SOUTH:
            min_dist = coordinate_calculation::greatCircleDistance(
                location, Coordinate(location.lon, min_lat));
            break;
        case WEST:
            min_dist = coordinate_calculation::greatCircleDistance(
                location, Coordinate(min_lon, location.lat));
            break;
        case EAST:
            min_dist = coordinate_calculation::greatCircleDistance(
                location, Coordinate(max_lon, location.lat));
            break;
        case NORTH_EAST:
            min_dist =
                coordinate_calculation::greatCircleDistance(location, Coordinate(max_lon, max_lat));
            break;
        case NORTH_WEST:
            min_dist =
                coordinate_calculation::greatCircleDistance(location, Coordinate(min_lon, max_lat));
            break;
        case SOUTH_EAST:
            min_dist =
                coordinate_calculation::greatCircleDistance(location, Coordinate(max_lon, min_lat));
            break;
        case SOUTH_WEST:
            min_dist =
                coordinate_calculation::greatCircleDistance(location, Coordinate(min_lon, min_lat));
            break;
        default:
            break;
        }

        BOOST_ASSERT(min_dist < std::numeric_limits<double>::max());

        return min_dist;
    }

    double GetMinMaxDist(const Coordinate location) const
    {
        double min_max_dist = std::numeric_limits<double>::max();
        // Get minmax distance to each of the four sides
        const Coordinate upper_left(min_lon, max_lat);
        const Coordinate upper_right(max_lon, max_lat);
        const Coordinate lower_right(max_lon, min_lat);
        const Coordinate lower_left(min_lon, min_lat);

        min_max_dist =
            std::min(min_max_dist,
                     std::max(coordinate_calculation::greatCircleDistance(location, upper_left),
                              coordinate_calculation::greatCircleDistance(location, upper_right)));

        min_max_dist =
            std::min(min_max_dist,
                     std::max(coordinate_calculation::greatCircleDistance(location, upper_right),
                              coordinate_calculation::greatCircleDistance(location, lower_right)));

        min_max_dist =
            std::min(min_max_dist,
                     std::max(coordinate_calculation::greatCircleDistance(location, lower_right),
                              coordinate_calculation::greatCircleDistance(location, lower_left)));

        min_max_dist =
            std::min(min_max_dist,
                     std::max(coordinate_calculation::greatCircleDistance(location, lower_left),
                              coordinate_calculation::greatCircleDistance(location, upper_left)));
        return min_max_dist;
    }

    bool Contains(const Coordinate location) const
    {
        const bool lons_contained = (location.lon >= min_lon) && (location.lon <= max_lon);
        const bool lats_contained = (location.lat >= min_lat) && (location.lat <= max_lat);
        return lons_contained && lats_contained;
    }

    friend std::ostream& operator<<(std::ostream& out, const RectangleInt2D& rect);
};
inline std::ostream& operator<<(std::ostream& out, const RectangleInt2D& rect)
{
    out << std::setprecision(12) << "(" << toFloating(rect.min_lon) << "," << toFloating(rect.max_lon) << "," << toFloating(rect.min_lat) << "," << toFloating(rect.max_lat) << ")";
    return out;
}
}
}

#endif
