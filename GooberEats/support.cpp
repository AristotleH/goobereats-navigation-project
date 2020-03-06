//
//  support.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#include "support.h"

bool operator==(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return lhs.m_lat == rhs.m_lat  &&  lhs.m_lon == rhs.m_lon;
}
