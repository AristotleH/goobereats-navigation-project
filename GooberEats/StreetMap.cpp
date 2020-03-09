#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
using namespace std;

const int NUMS_PER_SEGMENT = 4;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

inline
bool operator<(const StreetSegment& lhs, const StreetSegment& rhs)
{
    if (lhs.start < rhs.start) return true;
    if (rhs.start < lhs.start) return false;
    return lhs.end < rhs.end;
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, std::vector<StreetSegment*>> coordToSegments;
    std::list<StreetSegment*> allSegments;
    
    void addSegment(StreetSegment* segment);
    StreetSegment* reverse(const StreetSegment* original);
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
    for (auto it = allSegments.begin(); it != allSegments.end(); ++it)
        delete *it;
}

bool StreetMapImpl::load(string mapFile)
{
    std::ifstream fileStream;
    fileStream.open(mapFile.c_str(), std::ios::in);
    
    if (!fileStream)
        return false;
    
    StreetSegment* street;
    StreetSegment* reversedStreet;
    std::string streetName;
    
    int numStreetSegments;
    std::string coordData[NUMS_PER_SEGMENT];
    std::size_t startIndex;
    std::size_t endIndex;
    
    std::string line;
    while (std::getline(fileStream, line)) //automatically moves to next line
    {
        streetName = line;
        fileStream >> numStreetSegments;
        fileStream.ignore(10000, '\n');
        
        for (int i = 0; i < numStreetSegments; ++i)
        {
            std::getline(fileStream, line);
            startIndex = 0;
            for (int i = 0; i < NUMS_PER_SEGMENT; ++i)
            {
                endIndex = line.find(' ', startIndex);
                coordData[i] = line.substr(startIndex, endIndex - startIndex);
                startIndex = endIndex + 1;
            }
            
            street = new StreetSegment(GeoCoord(coordData[0], coordData[1]),
                                       GeoCoord(coordData[2], coordData[3]),
                                       streetName);
            addSegment(street);
            reversedStreet = new StreetSegment(street->end, street->start, street->name);
            addSegment(reversedStreet);
        }
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment*>* found = coordToSegments.find(gc);
    if (found == nullptr)
        return false;
    if (!segs.empty())
        segs.clear();
    for (auto it = found->begin(); it != found->end(); ++it)
        segs.push_back(**it);
    return true;
}

void StreetMapImpl::addSegment(StreetSegment* segment)
{
    allSegments.push_back(segment);
    std::vector<StreetSegment*>* sharesStartCoord = coordToSegments.find(segment->start);
    if (sharesStartCoord != nullptr)
    {
        sharesStartCoord->push_back(segment);
    }
    else
    {
        std::vector<StreetSegment*> newVector;
        newVector.push_back(segment);
        coordToSegments.associate(segment->start, newVector);
    }
}

inline
StreetSegment* StreetMapImpl::reverse(const StreetSegment* original)
{
    return new StreetSegment(original->end, original->start, original->name);
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
