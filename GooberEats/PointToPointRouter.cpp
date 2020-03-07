#include "provided.h"
#include <list>
#include <queue>
#include <set>
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

inline
double fScore(double gScore, double hScore)
{
    return gScore + hScore;
}

bool operator<(const MapNode& lhs, const MapNode& rhs)
{
    //greater than and not less than because MapNodes with least f-score are prioritized
    return (fScore(lhs.m_gScore, lhs.m_hScore) > fScore(rhs.m_gScore, rhs.m_hScore));
}

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* STREET_MAP;
    
    void constructPath(const MapNode* nodeStartSegment, const GeoCoord& start,
                       list<StreetSegment>& route, double& totalDistanceTravelled) const;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    : STREET_MAP(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

/*
 Finds route from starting coordinate to ending coordinate using the A* algorithm.
 */
DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    priority_queue<MapNode*> open;
    set<GeoCoord> closed;
    vector<StreetSegment> connectingSegments;
    //pointer to reference, which is just the reference refers to and not the reference itself
    //as the reference doesn't exist in memory
    double gScoreGenerated = 0.0;
    double hScoreGenerated = distanceEarthMiles(start, end);
    MapNode* current = new MapNode(
        start,
        nullptr,
        nullptr,
        gScoreGenerated,
        hScoreGenerated);
    open.push(current);
    if (!STREET_MAP->getSegmentsThatStartWith(end, connectingSegments)  &&
        !STREET_MAP->getSegmentsThatStartWith(start, connectingSegments))
    {
        return BAD_COORD;
    }
    while (!open.empty())
    {
        current = open.top();
        open.pop();
        if (current->m_hScore == 0)
        {
            constructPath(current, start, route, totalDistanceTravelled);
            return DELIVERY_SUCCESS;
        }
        STREET_MAP->getSegmentsThatStartWith(current->m_coord, connectingSegments);
        for (auto it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
        {
            if (closed.find(it->end) != closed.end())
                continue;
            gScoreGenerated = current->m_gScore + distanceEarthMiles(current->m_coord, it->end);
            hScoreGenerated = distanceEarthMiles(current->m_coord, end);
            open.push(new MapNode(it->end,
                                  current,
                                  &(*it),
                                  gScoreGenerated,
                                  hScoreGenerated));
        }
        closed.insert(current->m_coord);
        delete current;
    }
    return NO_ROUTE;
}

void PointToPointRouterImpl::constructPath(const MapNode* nodeStartSegment, const GeoCoord& start,
                                           list<StreetSegment>& route, double& totalDistanceTravelled) const
{
    totalDistanceTravelled = 0;
    while (!(nodeStartSegment->m_coord == start))
    {
        route.push_front(*(nodeStartSegment->m_prevSegment));
        totalDistanceTravelled += distanceEarthMiles(
            nodeStartSegment->m_prevSegment->start,
            nodeStartSegment->m_prevSegment->end);
        nodeStartSegment = nodeStartSegment->m_prevNode;
    }
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
