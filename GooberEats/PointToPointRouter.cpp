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
    MapNode(list<MapNode*> nodeList, GeoCoord coord, MapNode* prev, StreetSegment* prevSegment, double gScore, double hScore)
        : m_coord(coord), m_prevNode(prev), m_prevSegment(prevSegment), m_gScore(gScore), m_hScore(hScore)
    {
        nodeList.push_back(this);
    }
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
    void deleteMapNodes(list<MapNode*>& nodes) const;
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
    list<MapNode*> allMapNodes;
    priority_queue<MapNode*> open;
    set<GeoCoord> closed;
    vector<StreetSegment> connectingSegments;
    DeliveryResult result = NO_ROUTE;
    //pointer to reference, which is just the reference refers to and not the reference itself
    //as the reference doesn't exist in memory
    double g = 0.0;
    double h = distanceEarthMiles(start, end);
    
    cerr << "Now using A*..." << endl << endl;
    
    MapNode* current = new MapNode(allMapNodes, start, nullptr, nullptr, g, h);
    open.push(current);
    
    MapNode* next;
    
    if (!STREET_MAP->getSegmentsThatStartWith(end, connectingSegments)  &&
        !STREET_MAP->getSegmentsThatStartWith(start, connectingSegments))
    {
        result = BAD_COORD;
    }
    else
    {
        while (!open.empty())
        {
            current = open.top();
            open.pop();
            
            if (current->m_hScore == 0)
            {
                result = DELIVERY_SUCCESS;
                break;
            }
            STREET_MAP->getSegmentsThatStartWith(current->m_coord, connectingSegments);
            for (auto it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
            {
                if (closed.find(it->end) != closed.end())
                    continue;
                
                cerr << it->end.latitudeText << ", " << it->end.longitudeText << endl;
                
                g = current->m_gScore + distanceEarthMiles(current->m_coord, it->end);
                h = distanceEarthMiles(it->end, end);
                next = new MapNode(allMapNodes, it->end, current, &(*it), g, h);
                open.push(next);
            }
            closed.insert(current->m_coord);
        }
    }
    if (result == DELIVERY_SUCCESS)
        constructPath(current, start, route, totalDistanceTravelled);
    deleteMapNodes(allMapNodes);
    return result;
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

void PointToPointRouterImpl::deleteMapNodes(list<MapNode*>& nodes) const
{
    auto it = nodes.begin();
    while (!nodes.empty())
    {
        delete *it;
        it = nodes.erase(it);
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
