#include "provided.h"
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
using namespace std;

/*
 Definition of DeliveryOptimizerImpl; private members were added to spec's skeleton code.
 */
class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* STREET_MAP;
    
    double getCrowDistance(const vector<DeliveryRequest>& deliveries, const GeoCoord& origin) const;
};

/*
 Constructor for DeliveryOptimizerImpl; class has none of its own dynamically-allocated objects,
 so this constructor does nothing but set this object's StreetMap pointer to the one that's passed in.
 */
DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
    : STREET_MAP(sm)
{
}

/*
 Destructor for PointToPointRouterImpl; class has none of its own dynamically-allocated objects,
 so this destructor does nothing.
 */
DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

/*
 Finds a closer-to-optimal solution to the TSP applied to the list of delivery locations through a modification
 of simulated annealing.
 */
void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    // constants used multiple times in this function
    const double PCT_HEAT_RETAINED = 0.95;
    const int NUM_DELIVERIES = static_cast<int>(deliveries.size());
    const int INIT_TEMP = sqrt(NUM_DELIVERIES);
    
    // store old distance-as-the-crow-flies between all locations
    oldCrowDistance = getCrowDistance(deliveries, depot);
    
    // vectors that will be used to store states of the delivery request order
        // current: the vector representing the algorithm's current ordering of the deliveries,
            // will search away from local minimums during the loop
        // lowest: vector representing the algorithm's lowest-distance ordering so far,
            // essentially the lowest local minimum found so far
        // modified: the proposed new ordering of the deliveries created for every new loop iteration after a swap
    vector<DeliveryRequest> current = deliveries;
    vector<DeliveryRequest> lowest = current;
    vector<DeliveryRequest> modified;
    
    // initial temp is the square root of the destination count - average change in distance upon swap
        // refers to number of potential swaps that can occur with the vector's current ordering
    double temp = sqrt(INIT_TEMP);
    double minTemp = INIT_TEMP * pow(PCT_HEAT_RETAINED, 20 * log(NUM_DELIVERIES) - 1);
    // no matter if a loop accepts a change or not, each temp will have a number of attempts matching the
        // number of destinations
    int attemptsPerTemp = 0;
    
    // random number generator that will be used for random operations
    std::default_random_engine generator;
    // randomly shuffles the order of the deliveries
    std::shuffle(std::begin(current), std::end(current), generator);
    
    // sets up random number generator for integers that are valid indices for the delivery vector
    std::uniform_int_distribution<> randVectorIndex(0, static_cast<int>(current.size() - 1));
    // sets up random number generator for doubles between 0 and 1
    std::uniform_real_distribution<double> randZeroToOne(0, 1);
    
    // variables that keep track of our three vectors' distances (direct between coordinates in vector's order)
    double currentDistance = oldCrowDistance;
    double modifiedDistance; // will always been initialized before its use in later code
    double lowestDistance = oldCrowDistance;
    
    // while the temp of our delivery vector has not cooled sufficiently
    while (temp > minTemp)
    {
        // reset modified to our algorithm's current accepted solution
        modified = current;
        
        // swap two randomly chosen deliveries
        swap(*(modified.begin() + randVectorIndex(generator)),
             *(modified.begin() + randVectorIndex(generator)));
        
        // calculate the distance of this new ordering
        modifiedDistance = getCrowDistance(modified, depot);
        
        // if this swap shortens the distance below our working version or if it makes the distance larger underneath
            // a margin defined by the temp that the algorithm is at modified by an exponential,
            // use the modified version
        if (modifiedDistance < currentDistance  ||
            exp((currentDistance - modifiedDistance) / temp) > randZeroToOne(generator))
        {
            current = modified;
            currentDistance = getCrowDistance(current, depot);
            ++attemptsPerTemp;
            // if this distance is the shortest found yet, copy it into the lowest vector
            if (currentDistance < lowestDistance)
            {
                lowest = current;
                lowestDistance = getCrowDistance(lowest, depot);
            }
        }
        
        // if we've completed enough attempts to match the number of deliveries during this temperature, lower the
            // temperature and reset the counter
        if (attemptsPerTemp > NUM_DELIVERIES)
        {
            temp *= PCT_HEAT_RETAINED;
            attemptsPerTemp = 0;
        }
    }
    
    // if the lowest distance that we found is lower than the original distance, copy the best-found vector into the
        // deliveries vector argument
    if (lowestDistance < oldCrowDistance)
        deliveries = lowest;
    
    // generate the new distance of the deliveries vector
    newCrowDistance = getCrowDistance(deliveries, depot);
}

/*
 Returns the total distance traveled in a list of deliveries when adding trips to and from an origin point
 */
double DeliveryOptimizerImpl::getCrowDistance(const vector<DeliveryRequest>& deliveries, const GeoCoord& origin) const
{
    // initialize distance variable, first location is the origin
    double distance = 0;
    GeoCoord previous = origin;
    // add up the distances between each delivery + the origin
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it)
    {
        distance += distanceEarthMiles(previous, it->location);
        previous = it->location;
    }
    //add the distance to return to the depot and return the full distance
    distance += distanceEarthMiles(previous, origin);
    return distance;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
