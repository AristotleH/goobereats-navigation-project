#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
using namespace std;

// Constant representing number of coordinate doubles per street segment
const int NUMS_PER_SEGMENT = 4;

/*
 Hash function for GeoCoord objects; provided in spec.
 */
unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

/*
 Definition of StreetMapImpl; private members were added to spec's skeleton code.
 */
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

/*
 Constructor for StreetMapImpl; class has no dynamically-allocated objects at its creation,
 so this constructor does nothing.
 */
StreetMapImpl::StreetMapImpl()
{
}

/*
 Destructor for StreetMapImpl; deletes all dynamically-allocated StreetSegment objects created during its lifetime.
 */
StreetMapImpl::~StreetMapImpl()
{
    for (auto it = allSegments.begin(); it != allSegments.end(); ++it)
        delete *it;
}

/*
 Loads StreetSegment representations from a text file into an std::list and creates a mapping between their
 starting GeoCoord objects and the segments in an ExpandableHashMap.
 */
bool StreetMapImpl::load(string mapFile)
{
    // file stream object created to the file indicated with mapFile
    std::ifstream fileStream;
    fileStream.open(mapFile.c_str(), std::ios::in);
    
    // if the file is empty/can't be read, return
    if (!fileStream)
        return false;
    
    // variables that will be reused throughout the loading process of all StreetSegment objects
    // (streets and their names)
    StreetSegment* street;
    StreetSegment* reversedStreet;
    std::string streetName;
    
    // variables that will be reused throughout the loading process of all StreetSegment objects
    // (intermediary variables between file line load and StreetSegment creation)
    int numStreetSegments;
    std::string coordData[NUMS_PER_SEGMENT];
    std::size_t startIndex;
    std::size_t endIndex;
    
    // string that each line will be read into
    std::string line;
    
    // for every line in the file
    while (std::getline(fileStream, line)) //automatically moves to next line
    {
        // segment group's encoding begins with StreetSegment name
        streetName = line;
        
        // next line contains number of segments with that name
        fileStream >> numStreetSegments;
        fileStream.ignore(10000, '\n');
        
        // for each one of those segments
        for (int i = 0; i < numStreetSegments; ++i)
        {
            // read in their coordinates
            std::getline(fileStream, line);
            startIndex = 0;
            // split up the line into the four coordinate values it contains; place them into array
            for (int i = 0; i < NUMS_PER_SEGMENT; ++i)
            {
                endIndex = line.find(' ', startIndex);
                coordData[i] = line.substr(startIndex, endIndex - startIndex);
                startIndex = endIndex + 1;
            }
            
            // build StreetSegment object with these coordinates and the current street name
            street = new StreetSegment(GeoCoord(coordData[0], coordData[1]),
                                       GeoCoord(coordData[2], coordData[3]),
                                       streetName);
            // add this StreetSegment to StretMap's containers
            addSegment(street);
            // add this reversed segment to StreetMap's containers
            addSegment(reverse(street));
        }
    }
    //end of file, so all segments were imported
    return true;
}

/*
 Sets reference vector as the StreetSegments with the same starting coordinate as the GeoCoord that's passed in.
 */
bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    // use ExpandableHashMap's find() function to get a pointer to a vector of StreetSegment pointers that have the same
    // starting coordinate as the one passed-in
    const vector<StreetSegment*>* found = coordToSegments.find(gc);
    // if there aren't any, then the pointer is nullptr; in that case, return false as there aren't any segments that have
    // that starting coordinate
    if (found == nullptr)
        return false;
    // if the passed-in reference vector has elements, remove all of them
    if (!segs.empty())
        segs.clear();
    // for each StreetSegment pointed to in the found vector, copy its value to the passed-in reference vector
    for (auto it = found->begin(); it != found->end(); ++it)
        segs.push_back(**it);
    // we found at least one segment, so return true
    return true;
}

/*
 Adds the passed-in StreetSegment pointer to StreetMap's containers.
 */
void StreetMapImpl::addSegment(StreetSegment* segment)
{
    // add the segment to the std::list that keeps track of all allocated segments
    allSegments.push_back(segment);
    // get pointer to vector of segment pointers with the same starting location that are currently in the ExpandableHashMap
    std::vector<StreetSegment*>* sharesStartCoord = coordToSegments.find(segment->start);
    // if there's at least one segment found, then add the new segment's pointer to the existing vector
    if (sharesStartCoord != nullptr)
    {
        sharesStartCoord->push_back(segment);
    }
    // if there are currently no segments with that starting location, make a vector that will store such segments,
    // add the new one, and associate the start of this new segment with the new vector
    else
    {
        std::vector<StreetSegment*> newVector;
        newVector.push_back(segment);
        coordToSegments.associate(segment->start, newVector);
    }
}
/*
 Create new segment with the same coordinates, but with swapped positions so the direction is reversed.
 */
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
