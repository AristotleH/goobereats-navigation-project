#include "provided.h"
#include <vector>
using namespace std;

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
    const StreetMap* STREET_MAP;
    
    void addCommands(const list<StreetSegment>& segments, vector<DeliveryCommand>& commands) const;
    string cardinalDirection(const StreetSegment& segment) const;
    bool streetRequiresTurn(const StreetSegment& seg1, const StreetSegment& seg2, string& direction) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    : STREET_MAP(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    list<StreetSegment> routePerDelivery;
    DeliveryCommand routeFinished;
    
    double distancePerDelivery;
    totalDistanceTravelled = 0;
    DeliveryResult result;
    
    PointToPointRouter pathfinder(STREET_MAP);
    
    GeoCoord startingCoord = depot;
    GeoCoord endingCoord;
    
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it)
    {
        endingCoord = it->location;
        
        result = pathfinder.generatePointToPointRoute(startingCoord,
                                                      endingCoord,
                                                      routePerDelivery,
                                                      distancePerDelivery);
        if (result != DELIVERY_SUCCESS)
            return result;
        
        totalDistanceTravelled += distancePerDelivery;
        
        addCommands(routePerDelivery, commands);
        routeFinished.initAsDeliverCommand(it->item);
        commands.push_back(routeFinished);
        
        startingCoord = endingCoord;
    }
    result = pathfinder.generatePointToPointRoute(startingCoord,
                                                  depot,
                                                  routePerDelivery,
                                                  distancePerDelivery);
    totalDistanceTravelled += distancePerDelivery;
    addCommands(routePerDelivery, commands);
    
    return result;
}

void DeliveryPlannerImpl::addCommands(const list<StreetSegment>& segments, vector<DeliveryCommand>& commands) const
{
    auto itPrevious = segments.begin();
    DeliveryCommand command;
    command.initAsProceedCommand(cardinalDirection(*itPrevious), itPrevious->name, 0);
    
    for (auto itCurrent = segments.begin(); itCurrent != segments.end(); ++itCurrent)
    {
        if (itCurrent->name == itPrevious->name)
        {
            command.increaseDistance(distanceEarthMiles(itCurrent->start, itCurrent->end));
            continue;
        }
        commands.push_back(command);
        
        string turnToTake;
        if (streetRequiresTurn(*itPrevious, *itCurrent, turnToTake))
        {
            command.initAsTurnCommand(turnToTake, itCurrent->name);
            commands.push_back(command);
        }
        
        command.initAsProceedCommand(cardinalDirection(*itCurrent), 
                                     itCurrent->name,
                                     distanceEarthMiles(itCurrent->start, itCurrent->end));
        itPrevious = itCurrent;
    }
    commands.push_back(command);
}

string DeliveryPlannerImpl::cardinalDirection(const StreetSegment& segment) const
{
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

bool DeliveryPlannerImpl::streetRequiresTurn(const StreetSegment& seg1, const StreetSegment& seg2, string& direction) const
{
    double dir = angleBetween2Lines(seg1, seg2);
    
    cerr << seg1.name << " to " << seg2.name << " is " << dir << endl;;
    
    if (dir >= 1  &&  dir < 180)
    {
        direction = "left";
        return true;
    }
    else if (dir >= 180  &&  dir <= 359)
    {
        direction = "right";
        return true;
    }
    return false;
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
