//
//  PointToPointRouter.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

#include <queue>
#include <set>
#include <list>

#include "providedMyVersion.h"
#include "support.h"

using namespace std;

struct MapNode
{
    GeoCoord m_coord;
    MapNode* m_prevNode;
    StreetSegment* m_prevSegment;
    double m_gScore;
    double m_hScore;
    MapNode(GeoCoord coord, MapNode* prev, StreetSegment* prevSegment, double gScore, double hScore)
        : m_coord(coord), m_prevNode(prev), m_prevSegment(prevSegment), m_gScore(gScore), m_hScore(hScore) {}
};

bool operator<(const MapNode& lhs, const MapNode& rhs)
{
    //greater than and not less than because MapNodes with least f-score are prioritized
    return ((lhs.m_gScore + lhs.m_hScore) > (rhs.m_gScore + rhs.m_hScore));
}

class PointToPointRouter
{
public:
    PointToPointRouter(const StreetMap* sm);
    ~PointToPointRouter();
    DELIVERY_RESULT generatePointToPointRoute(const GeoCoord& start,
                                              const GeoCoord& end,
                                              std::list<StreetSegment>& route,
                                              double& totalDistanceTravelled) const;
private:
    void constructPath(const MapNode* nodeStartSegment, const GeoCoord& start, std::list<StreetSegment>& route);
};

/*
 A* algorithm implementation for point-to-point routing.
 */
DELIVERY_RESULT PointToPointRouter::generatePointToPointRoute(const GeoCoord& start,
                                                              const GeoCoord& end,
                                                              std::list<StreetSegment>& route,
                                                              double& totalDistanceTravelled)
{
    priority_queue<MapNode*> open;
    set<GeoCoord> closed;
    vector<StreetSegment> connectingSegments;
    //pointer to reference, which is just the reference refers to and not the reference itself
    //as the reference doesn't exist in memory
    double gScoreGenerated = 0.0;
    double hScoreGenerated = 0.0; //distanceEarthMiles();
    MapNode* current = new MapNode(start,
                                   nullptr,
                                   nullptr,
                                   gScoreGenerated,
                                   hScoreGenerated);
    open.push(current);
    if (!getSegmentsThatStartWith(end, connectingSegments)  &&
        !getSegmentsThatStartWith(start, connectingSegments))
        //return RESULT FAILURE FILL IN HERE;
    ;
    while (!open.empty())
    {
        current = open.top();
        open.pop();
        if (current->m_hScore == 0)
        {
            constructPath(current, start, route);
            //return SUCCESS WE DID IT FILL IN HERE
        }
        getSegmentsThatStartWith(current, connectingSegments);
        for (vector<StreetSegment>::iterator it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
        {
            if (closed.find(it->m_end) != closed.end())
                continue;
            gScoreGenerated = current->m_gScore; //+ distanceEarthMiles(current->m_coord, *it);
            hScoreGenerated = 0.0; // distanceEarthMiles(current->m_coord, end);
            open.push(new MapNode(it->m_end,
                                  current,
                                  &(*it),
                                  gScoreGenerated,
                                  hScoreGenerated));
        }
        closed.insert(current->m_coord);
        delete current;
    }
    //return NOT FOUND
}

void PointToPointRouter::constructPath(const MapNode* nodeStartSegment, const GeoCoord& start, std::list<StreetSegment>& route)
{
    while (!(nodeStartSegment->m_coord == start))
    {
        route.push_front(*(nodeStartSegment->m_prevSegment));
        nodeStartSegment = nodeStartSegment->m_prevNode;
    }
}
