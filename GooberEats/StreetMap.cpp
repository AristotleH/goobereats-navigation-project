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

using namespace std;

class StreetMap
{
public:
    StreetMap();
    ~StreetMap();
    bool load(std::string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc,
                                  std::vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment*>> geocoordToSegments;
    set<StreetSegment> allSegments;
    
    void addSegment(StreetSegment* segment);
    StreetSegment* reverse(const StreetSegment* original);
};

bool StreetMap::load(std::string mapFile)
{
    ifstream fileStream;
    fileStream.open(mapFile.c_str(), ios::in);
    
    if (!fileStream)
        return false;
    
    StreetSegment* street;
    StreetSegment* reversedStreet;
    std::string streetName;
    
    int numStreetSegments;
    std::string::size_type endOfLastNumber;
    double coordData[NUMS_PER_SEGMENT];
    
    std::string line;
    while (std::getline(fileStream, line)) //automatically moves to next line
    {
        streetName = line;
        std::getline(fileStream, line);
        
        numStreetSegments = std::stoi(line);
        
        std::getline(fileStream, line);
        
        for (int i = 0; i < numStreetSegments; ++i)
        {
            for (int i = 0; i < NUMS_PER_SEGMENT; ++i)
                coordData[i] = std::stod(line, &endOfLastNumber);
            street = new StreetSegment(GeoCoord(coordData[0], coordData[1]),
                                       GeoCoord(coordData[2], coordData[3]),
                                       streetName);
            reversedStreet = reverse(street);
            addSegment(street);
            addSegment(reversedStreet);
        }
    }
    return true;
}

void StreetMap::addSegment(StreetSegment* segment)
{
    allSegments.insert(*segment);
    vector<StreetSegment*>* sharedStartCoord = geocoordToSegments.find(segment->m_start);
    if (sharedStartCoord == nullptr)
    {
        sharedStartCoord = new vector<StreetSegment*>;
        geocoordToSegments.associate(segment->m_start, *sharedStartCoord);
    }
    sharedStartCoord->push_back(segment);
}

inline
StreetSegment* StreetMap::reverse(const StreetSegment* original)
{
    return new StreetSegment(original->m_end, original->m_start, original->m_name);
}
