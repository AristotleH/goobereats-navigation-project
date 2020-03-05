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

using namespace std;

struct MapNode
{
    GeoCoord m_coord;
    MapNode* m_prev;
    double m_gScore;
    double m_hScore;
    MapNode(GeoCoord coord, MapNode* prev, double gScore, double hScore)
        : m_coord(coord), m_prev(prev), m_gScore(gScore), m_hScore(hScore) {}
};

class PointToPointRouter
{
public:
    PointToPointRouter(const StreetMap* sm);
    ~PointToPointRouter();
    DELIVERY_RESULT generatePointToPointRoute(const GeoCoord& start,
                                              const GeoCoord& end,
                                              std::list<StreetSegment>& route,
                                              double& totalDistanceTravelled) const;
};

/*
 A* algorithm implementation for point-to-point routing.
 */
DELIVERY_RESULT PointToPointRouterImpl::generatePointToPointRoute(const GeoCoord& start,
                                                                  const GeoCoord& end,
                                                                  std::list<StreetSegment>& route,
                                                                  double& totalDistanceTravelled)
{
    priority_queue<MapNode*> open;
    set<GeoCoord*> closed;
    vector<GeoCoord> connectingSegments;
    //pointer to reference, which is just the reference refers to and not the reference itself
    //as the reference doesn't exist in memory
    double gScoreGenerated = 0.0;
    double hScoreGenerated = 0.0; //distanceEarthMiles();
    MapNode* current = new MapNode(start, nullptr, gScoreGenerated, hScoreGenerated);
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
            constructPath(current);
            //return SUCCESS WE DID IT FILL IN HERE
        }
        getSegmentsThatStartWith(current, connectingSegments);
        for (vector<GeoCoord>::const_iterator it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
        {
            if ()
        }
    }
}

void PointToPointRouterImpl::constructPath(MapNode* endingNode)
{
    
}
