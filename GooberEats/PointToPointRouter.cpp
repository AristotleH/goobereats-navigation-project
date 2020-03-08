#include "provided.h"
#include <list>
#include <queue>
#include <set>
using namespace std;

struct MapNode
{
    GeoCoord m_coord;
    MapNode* m_prevNode;
    StreetSegment m_prevSegment;
    double m_gScore;
    double m_hScore;
    MapNode(list<MapNode*>* nodeList, GeoCoord coord, MapNode* prev, StreetSegment prevSegment, double gScore, double hScore)
        : m_coord(coord), m_prevNode(prev), m_prevSegment(prevSegment), m_gScore(gScore), m_hScore(hScore)
    {
        nodeList->push_back(this);
    }
    MapNode(list<MapNode*>* nodeList, GeoCoord coord, double hScore)
        : m_coord(coord), m_prevNode(nullptr), m_prevSegment(StreetSegment()), m_gScore(0), m_hScore(hScore)
    {
        nodeList->push_back(this);
    }
};

inline
double fScore(const MapNode* node)
{
    return node->m_gScore + node->m_hScore;
}

struct MapNodePtrComparator
{
    bool operator() (const MapNode* lhs, const MapNode* rhs)
    {
        //greater than and not less than because MapNodes with least f-score are prioritized
        return (fScore(lhs) > fScore(rhs));
    }
};

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
    
    void constructPath(const MapNode* node, const GeoCoord& start,
                       list<StreetSegment>& route, double& distance) const;
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
    priority_queue<MapNode*, vector<MapNode*>, MapNodePtrComparator> open;
    set<GeoCoord> closed;
    
    vector<StreetSegment> connectingSegments;
    DeliveryResult result = NO_ROUTE;
    //pointer to reference, which is just the reference refers to and not the reference itself
    //as the reference doesn't exist in memory
    double g = 0.0;
    double h = distanceEarthMiles(start, end);
    
    MapNode* current = new MapNode(&allMapNodes, start, h);
    open.push(current);
    
    MapNode* next;
    
    if (!route.empty())
        route.clear();
    
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
            closed.insert(current->m_coord);
            
            if (current->m_coord == end)
            {
                result = DELIVERY_SUCCESS;
                break;
            }
            STREET_MAP->getSegmentsThatStartWith(current->m_coord, connectingSegments);
            for (auto it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
            {
                if (closed.find(it->end) != closed.end())
                    continue;
                g = current->m_gScore + distanceEarthMiles(it->start, it->end);
                h = distanceEarthMiles(it->end, end);
                
                next = new MapNode(&allMapNodes,
                                   it->end,
                                   current,
                                   StreetSegment(it->start, it->end, it->name),
                                   g,
                                   h);
                open.push(next);
            }
        }
    }
    if (result == DELIVERY_SUCCESS)
        constructPath(current, start, route, totalDistanceTravelled);
    
    deleteMapNodes(allMapNodes);
    return result;
}

void PointToPointRouterImpl::constructPath(const MapNode* node, const GeoCoord& start,
                                           list<StreetSegment>& route, double& distance) const
{
    distance = 0;
    while (node->m_prevNode != nullptr)
    {
        route.push_front(node->m_prevSegment);
        distance += distanceEarthMiles( node->m_prevSegment.start, node->m_prevSegment.end);
        node = node->m_prevNode;
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
