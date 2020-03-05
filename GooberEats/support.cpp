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
    return std::hash<std::string>()(std::to_string(g.m_lat) + std::to_string(g.m_lon));
}

unsigned int hash(const StreetSegment& s)
{
    return std::hash<std::string>()(std::to_string(s.m_start.m_lat) + std::to_string(s.m_start.m_lon) +
                                    std::to_string(s.m_end.m_lat) + std::to_string(s.m_end.m_lon));
}

bool operator==(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return lhs.m_lat == rhs.m_lat  &&  lhs.m_lon == rhs.m_lon;
}

bool operator<(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return hash(lhs) < hash(rhs);
}

bool operator<(const StreetSegment& lhs, const StreetSegment& rhs)
{
    return hash(lhs) < hash(rhs);
}
