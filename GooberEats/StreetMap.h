//
//  StreetMap.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#include <fstream>
#include <vector>
#include <set>

#include "support.h"
#include "ExpandableHashMap.h"

unsigned int hash(const GeoCoord& g)
{
    return std::hash<std::string>()(std::to_string(g.m_lat) + std::to_string(g.m_lon));
}

unsigned int hash(const StreetSegment& s)
{
    return std::hash<std::string>()(std::to_string(s.m_start.m_lat) + std::to_string(s.m_start.m_lon) +
                                    std::to_string(s.m_end.m_lat) + std::to_string(s.m_end.m_lon));
}

bool operator<(const GeoCoord& lhs, const GeoCoord& rhs)
{
    return hash(lhs) < hash(rhs);
}

bool operator<(const StreetSegment& lhs, const StreetSegment& rhs)
{
    return hash(lhs) < hash(rhs);
}

class StreetMap
{
public:
    StreetMap();
    ~StreetMap();
    bool load(std::string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc,
                                  std::vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, std::vector<StreetSegment*>> geocoordToSegments;
    std::set<StreetSegment> allSegments;
    
    void addSegment(StreetSegment* segment);
    StreetSegment* reverse(const StreetSegment* original);
};

StreetMap::StreetMap() {}

StreetMap::~StreetMap() {}

bool StreetMap::load(std::string mapFile)
{
    std::ifstream fileStream;
    fileStream.open(mapFile.c_str(), std::ios::in);
    
    if (!fileStream)
        return false;
    
    StreetSegment* street;
    std::string streetName;
    
    int numStreetSegments;
    std::string::size_type endOfLastNumber;
    double coordData[NUMS_PER_SEGMENT];
    std::size_t startIndex;
    std::size_t endIndex;
    
    std::string line;
    while (std::getline(fileStream, line)) //automatically moves to next line
    {
        streetName = line;
        std::getline(fileStream, line);
        numStreetSegments = std::stoi(line);
        
        for (int i = 0; i < numStreetSegments; ++i)
        {
            std::getline(fileStream, line);
            endOfLastNumber = 0;
            startIndex = 0;
            for (int i = 0; i < NUMS_PER_SEGMENT; ++i)
            {
                endIndex = line.find(' ', startIndex);
                coordData[i] = std::stod(line.substr(startIndex, endIndex - startIndex));
                startIndex = endIndex + 1;
            }
            street = new StreetSegment(GeoCoord(coordData[0], coordData[1]),
                                       GeoCoord(coordData[2], coordData[3]),
                                       streetName);
            addSegment(street);
            addSegment(reverse(street));
        }
    }
    return true;
}

void StreetMap::addSegment(StreetSegment* segment)
{
    allSegments.insert(*segment);
    std::vector<StreetSegment*>* sharedStartCoord = geocoordToSegments.find(segment->m_start);
    if (sharedStartCoord == nullptr)
    {
        sharedStartCoord = new std::vector<StreetSegment*>;
        geocoordToSegments.associate(segment->m_start, *sharedStartCoord);
    }
    sharedStartCoord->push_back(segment);
}

inline
StreetSegment* StreetMap::reverse(const StreetSegment* original)
{
    return new StreetSegment(original->m_end, original->m_start, original->m_name);
}
