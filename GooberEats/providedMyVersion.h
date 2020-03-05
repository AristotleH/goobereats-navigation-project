//
//  providedMyVersion.h
//  GooberEats
//
//  Created by Aristotle Henderson on 3/3/20.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#ifndef providedMyVersion_h
#define providedMyVersion_h

#include <string>

struct GeoCoord
{
    double m_lat;
    double m_lon;
    GeoCoord(double lat, double lon) : m_lat(lat), m_lon(lon) {}
};

struct StreetSegment
{
    GeoCoord m_start;
    GeoCoord m_end;
    std::string m_name;
    StreetSegment(GeoCoord start, GeoCoord end, std::string name) : m_start(start), m_end(end), m_name(name) {};
};

#endif /* providedMyVersion_h */
