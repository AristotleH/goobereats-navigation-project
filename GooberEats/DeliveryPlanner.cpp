#include "provided.h"
#include <vector>
using namespace std;

/*
 Definition of DeliveryPlannerImpl; private members were added to spec's skeleton code.
 */
class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    DeliveryOptimizer optimizer;
    PointToPointRouter pathfinder;
    
    void addCommands(const list<StreetSegment>& segments, vector<DeliveryCommand>& commands) const;
    string cardinalDirection(const StreetSegment& segment) const;
    bool streetRequiresTurn(const StreetSegment& seg1, const StreetSegment& seg2, string& direction) const;
};

/*
 Constructor for DeliveryPlannerImpl; passes in StreetMap arguments for DeliveryOptimizer and PointToPointRouter.
 */
DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    : optimizer(sm), pathfinder(sm)
{
}

/*
 Destructor for PointToPointRouterImpl; class has none of its own dynamically-allocated objects,
 so this destructor does nothing.
 */
DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

/*
 Utilizes other classes to report the directions and distances for a StreetMap and a vector of deliveries.
 */
DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    // place the deliveries vector into a separate vector so as to not modify the reference variable.
    vector<DeliveryRequest> optimizedDeliveries = deliveries;
    double originalCrowDistance;
    double optimizedCrowDistance;
    optimizer.optimizeDeliveryOrder(depot, optimizedDeliveries, originalCrowDistance, optimizedCrowDistance);
    
    // set up variables for the loop below
    list<StreetSegment> deliveryRoute;
    double deliveryDistance;
    totalDistanceTravelled = 0;
    DeliveryCommand routeFinished;
    DeliveryResult result;
    GeoCoord startCoord = depot;
    GeoCoord endCoord;
    
    // process every delivery in the delivery vector + the beginning delivery from the depot
    for (auto it = optimizedDeliveries.begin(); it != optimizedDeliveries.end(); ++it)
    {
        // update the ending coordinate of this delivery
        endCoord = it->location;
        
        // find a path of segments to reach the ending coordinate
        result = pathfinder.generatePointToPointRoute(startCoord,
                                                      endCoord,
                                                      deliveryRoute,
                                                      deliveryDistance);
        // if a route wasn't found, end the function
        if (result != DELIVERY_SUCCESS)
            return result;
        
        // add the distance traveled to the total distance and add the generated commands to the vector of all commands
        totalDistanceTravelled += deliveryDistance;
        addCommands(deliveryRoute, commands);
        
        // add a deliver command to the vector
        routeFinished.initAsDeliverCommand(it->item);
        commands.push_back(routeFinished);
        
        // update the starting coordinate for the next delivery
        startCoord = endCoord;
    }
    
    // generate a path back to the depot
    result = pathfinder.generatePointToPointRoute(startCoord, depot, deliveryRoute, deliveryDistance);
    
    // if a route wasn't found, end the function
    if (result != DELIVERY_SUCCESS)
        return result;
    
    // add the distance traveled to the total distance and add the generated commands to the vector of all commands
    totalDistanceTravelled += deliveryDistance;
    addCommands(deliveryRoute, commands);
    
    // return DELIVERY_SUCCESS (the only value that result can be)
    return result;
}

/*
 Adds commands corresponding to a list of StreetSegments for a delivery to the passed-in vector.
 */
void DeliveryPlannerImpl::addCommands(const list<StreetSegment>& segments, vector<DeliveryCommand>& commands) const
{
    // every delivery starts with a proceed command, so we initialize one first
    auto itPrevious = segments.begin();
    DeliveryCommand command;
    command.initAsProceedCommand(cardinalDirection(*itPrevious), itPrevious->name, 0);
    
    // process every street segment that's passed in
    for (auto itCurrent = segments.begin(); itCurrent != segments.end(); ++itCurrent)
    {
        // if the street segment is a continuation of the last one, just increase the distance of the last street
        if (itCurrent->name == itPrevious->name)
            command.increaseDistance(distanceEarthMiles(itCurrent->start, itCurrent->end));
        else
        {
            // add the previous street segment to the vector of commands
            commands.push_back(command);
            
            // if the next street requires a turn, add a turn command in the proper direction to the command vector
            string turnToTake;
            if (streetRequiresTurn(*itPrevious, *itCurrent, turnToTake))
            {
                command.initAsTurnCommand(turnToTake, itCurrent->name);
                commands.push_back(command);
            }
            
            // initialize a proceed command in the proper direction for the next StreetSegment
            command.initAsProceedCommand(cardinalDirection(*itCurrent),
                                         itCurrent->name,
                                         distanceEarthMiles(itCurrent->start, itCurrent->end));
        }
        // update the iterator to the previous segment
        itPrevious = itCurrent;
    }
    // if the route has at least one segment, add the last segment that was processed
    if (!segments.empty())
        commands.push_back(command);
}

/*
 Returns a cardinal direction for a StreetSegment's angle per the spec.
 */
string DeliveryPlannerImpl::cardinalDirection(const StreetSegment& segment) const
{
    // get the angle of the street segment and return the corresponding cardinal direction
    double dir = angleOfLine(segment);
    if (dir >= 0  &&  dir < 22.5)
        return "east";
    if (dir < 67.5)
        return "northeast";
    if (dir < 112.5)
        return "north";
    if (dir < 157.5)
        return "northwest";
    if (dir < 202.5)
        return "west";
    if (dir < 247.5)
        return "southwest";
    if (dir < 292.5)
        return "south";
    if (dir < 337.5)
        return "southeast";
    return "east";
}

/*
 Returns whether a turn is required for two StreetSegments and, if one is, pass the direction through a parameter.
 */
bool DeliveryPlannerImpl::streetRequiresTurn(const StreetSegment& seg1, const StreetSegment& seg2, string& direction) const
{
    // get the angle between the two StreetSegments
    double dir = angleBetween2Lines(seg1, seg2);
    
    // if the angle difference is small enough, a turn isn't needed
    if (dir < 1  ||  dir > 359)
        return false;
    
    // return the corresponding direction for the angle's turn and return that a turn is needed
    if (dir >= 1  &&  dir < 180)
        direction = "left";
    else
        direction = "right";
    return true;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
