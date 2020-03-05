//
//  support.h
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#ifndef support_h
#define support_h

#include "providedMyVersion.h"

const int NUMS_PER_SEGMENT = 4;

unsigned int hash(const std::string& key); // yields value in range 0 to ~4 billion
unsigned int hash(const GeoCoord& g);

bool operator==(const GeoCoord& lhs, const GeoCoord& rhs);

bool operator<(const GeoCoord& lhs, const GeoCoord& rhs);
bool operator<(const StreetSegment& lhs, const StreetSegment& rhs);

#endif /* support_h */
