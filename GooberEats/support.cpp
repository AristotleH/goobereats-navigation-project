//
//  support.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#include <string>
#include <functional>

#include "support.h"

unsigned int hash(const std::string& key)
{
    std::hash<std::string> hasher;
    return static_cast<unsigned int>(hasher(key));
}

unsigned int hash(const GeoCoord& g)
{
    return std::hash<std::string>()(g.latitudeText + g.longitudeText);
}

bool operator==(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return lhs.m_lat == rhs.m_lat  &&  lhs.m_lon == rhs.m_lon;
}

bool operator<(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return (lhs.m_lat + lhs.m_lon) < (rhs.m_lat + rhs.m_lon);
}


bool operator<(const StreetSegment& lhs, const StreetSegment& rhs)
{
    return lhs.m_start < rhs.m_start;
}
