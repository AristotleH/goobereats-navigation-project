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

StreetMap::StreetMap() {}

StreetMap::~StreetMap() {}

bool StreetMap::load(std::string mapFile)
{
    ifstream fileStream;
    fileStream.open(mapFile.c_str(), ios::in);
    
    if (!fileStream)
        return false;
    
    StreetSegment* street;
    std::string streetName;
    
    int numStreetSegments;
    std::string::size_type endOfLastNumber;
    double coordData[NUMS_PER_SEGMENT];
    std::size_t startIndex;
    std::size_t endIndex;
    
    int a = 0;
    
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
            a += 2;
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
