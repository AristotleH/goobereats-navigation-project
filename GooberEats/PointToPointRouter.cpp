#include "provided.h"
#include <list>
#include <queue>
#include <set>
using namespace std;

/*
 Stores locations within StreetMap, their relationships to immediate locations/segments within the A* algorithm,
 and their properties used within a search.
 */
struct MapNode
{
    // location on map
    GeoCoord m_coord;
    // previous location's MapNode on A*'s path
    MapNode* m_prevNode;
    // previous street segment on A*'s path
    StreetSegment m_prevSegment;
    // MapNode's g score - the sum of all segment lengths traveled thus far on this node's corresponding path
    double m_gScore;
    // MapNode's h (heuristic) score - the distance as the crow flies to the destination coordinate from this node
    double m_hScore;
    // Constructs MapNode with a previous node - both constructors add a pointer to the node to a list containing all nodes
    MapNode(list<MapNode*>* nodeList, GeoCoord coord, MapNode* prev, StreetSegment prevSegment, double gScore, double hScore)
        : m_coord(coord), m_prevNode(prev), m_prevSegment(prevSegment), m_gScore(gScore), m_hScore(hScore)
    {
        nodeList->push_back(this);
    }
    // Constructs MapNode at the origin of the pathfinding algorithm
    MapNode(list<MapNode*>* nodeList, GeoCoord coord, double hScore)
        : m_coord(coord), m_prevNode(nullptr), m_gScore(0), m_hScore(hScore)
    {
        nodeList->push_back(this);
    }
};

/*
 Computes f score of a map node by summing their g and h scores.
 */
inline
double fScore(const MapNode* node)
{
    return node->m_gScore + node->m_hScore;
}

/*
 Defines a comparator for use in a priority queue of MapNode pointers; compares the pointed-to MapNodes' f scores.
 */
struct MapNodePtrComparator
{
    bool operator() (const MapNode* lhs, const MapNode* rhs) const
    {
        // greater than and not less than because MapNodes with least f-score are prioritized
        return (fScore(lhs) > fScore(rhs));
    }
};

/*
 Definition of PointToPointRouterImpl; private members were added to spec's skeleton code.
 */
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
    
    void constructPath(const MapNode* node, list<StreetSegment>& route, double& distance) const;
    void deleteMapNodes(list<MapNode*>& nodes) const;
};

/*
 Constructor for StreetMapImpl; class has none of its own dynamically-allocated objects,
 so this constructor does nothing but set this object's StreetMap pointer to the one that's passed in.
 */
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    : STREET_MAP(sm)
{
}

/*
 Destructor for PointToPointRouterImpl; class has none of its own dynamically-allocated objects,
 so this destructor does nothing.
 */
PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

/*
 Finds route on object's pointed-to StreetMap from starting coordinate to ending coordinate using the A* algorithm.
 */
DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    // all MapNodes created in this function will have a pointer placed in this list
    list<MapNode*> allMapNodes;
    // priority queue that will be used to determine which MapNode to search first
    priority_queue<MapNode*, vector<MapNode*>, MapNodePtrComparator> open;
    // set of GeoCoords that should not be visited again
    set<GeoCoord> closed;
    
    // vector that segments which connect to a MapNode will be placed in
    vector<StreetSegment> connectingSegments;
    // by default, the result is that a route isn't found
    DeliveryResult result = NO_ROUTE;
    
    // g score of first MapNode is 0 (no distance between node and origin); h is calculated by definition
    double g = 0.0;
    double h = distanceEarthMiles(start, end);
    
    // origin node created; pushed to queue; this variable will be used throughout A* for representation of current node
    MapNode* current = new MapNode(&allMapNodes, start, h);
    open.push(current);
    
    //used throughout A*, variable to represent newly-added node
    MapNode* next;
    
    //if the passed-in vector isn't empty, clear it
    if (!route.empty())
        route.clear();
    
    // if the start or end node is invalid, set result accordingly and skip A*
    if (!STREET_MAP->getSegmentsThatStartWith(start, connectingSegments)  ||
        !STREET_MAP->getSegmentsThatStartWith(end, connectingSegments))
    {
        result = BAD_COORD;
    }
    // A* time!
    else
    {
        // while there are nodes left to process
        while (!open.empty())
        {
            // extract the top node off of the queue
            current = open.top();
            open.pop();
            
            // add top node's coordinate to list of those that we can no longer process
            closed.insert(current->m_coord);
            
            // if the segment's ending node is the goal, we're done pathfinding! break out of the loop and report a success
            if (current->m_coord == end)
            {
                result = DELIVERY_SUCCESS;
                break;
            }
            // get all segments that begin from this node and store in vector
            STREET_MAP->getSegmentsThatStartWith(current->m_coord, connectingSegments);
            // for each connecting segment to the current node
            for (auto it = connectingSegments.begin(); it != connectingSegments.end(); ++it)
            {
                // if the connecting segment's ending coordinate is one that we've already visited, ignore the segment
                if (closed.find(it->end) != closed.end())
                    continue;
                
                // compute the g and h scores of the to-be-created node at the end of the connecting segment
                g = current->m_gScore + distanceEarthMiles(it->start, it->end);
                h = distanceEarthMiles(it->end, end);
                
                // generate a new MapNode with the arguments...
                    // &allMapNodes: a pointer to the allMapNodes list that this node will be added to
                    // it->end: the location of this node at the ending coordinate of the connecting segment
                    // current: the MapNode immediately previous to this connecting segment (node for the
                        // segment's starting coordinate)
                    // StreetSegment(it->start, it->end, it->name): a new StreetSegment created from the connecting segment's
                        // data
                    // g: our computed g score
                    // h: our computed h score
                next = new MapNode(&allMapNodes,
                                   it->end,
                                   current,
                                   StreetSegment(it->start, it->end, it->name),
                                   g,
                                   h);
                // add this new node to the queue
                open.push(next);
            }
        }
    }
    // if we exited the loop because we successfully reached the goal, construct a path back to the start and pu
        // that path in the route vector
    if (result == DELIVERY_SUCCESS)
        constructPath(current, route, totalDistanceTravelled);
    
    // delete all nodes that were created
    deleteMapNodes(allMapNodes);
    // return the result we reached
    return result;
}

/*
 Builds a list of StreetSegments from the starting coordinate to the ending coordinate based on the
 linked list of MapNodes that were created during A* and passes back their total distance.
 Requires the starting node's previous pointer to point to nullptr; the above A* implementation doesn't revisit points,
 so lists won't be circular and the below loop won't be infinite.
 */
void PointToPointRouterImpl::constructPath(const MapNode* node, list<StreetSegment>& route, double& distance) const
{
    // the distance of all segments begins at zero
    distance = 0;
    // for every MapNode linked to the goal's MapNode
    while (node->m_prevNode != nullptr)
    {
        // put the MapNode at the front of the list - we're going from the end to the start of these map nodes,
            // so we need to reverse their order
        route.push_front(node->m_prevSegment);
        // add the distance of the map node to the total distance
        distance += distanceEarthMiles(node->m_prevSegment.start, node->m_prevSegment.end);
        // go to the next map node that's closer to the origin
        node = node->m_prevNode;
    }
}

/*
 Deletes all MapNodes in a list of pointers to MapNodes.
 */
void PointToPointRouterImpl::deleteMapNodes(list<MapNode*>& nodes) const
{
    // continously sets an iterator to the beginning of the list and deletes the dynamically-allocated
        // MapNode at that position and the pointer itself until the list is empty
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
