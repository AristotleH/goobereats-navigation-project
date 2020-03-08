#include "provided.h"
#include <vector>
#include <algorithm>
#include <random>
#include <utility>
using namespace std;

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

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
    : STREET_MAP(sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = getCrowDistance(deliveries, depot);
    
    vector<DeliveryRequest> current = deliveries;
    vector<DeliveryRequest> lowest = current;
    vector<DeliveryRequest> modified;
    
    double temperature = pow(current.size(), 2);
    double pctHeatRetained = 0.9;
    const int MAX_LOOPS_TEMP = 100;
    int numLoopsForTemp = MAX_LOOPS_TEMP;
    
    std::default_random_engine generator;
    std::shuffle(std::begin(current), std::end(current), generator);
    
    std::uniform_int_distribution<> randVectorIndex(0, static_cast<int>(current.size() - 1));
    std::uniform_real_distribution<double> randZeroToOne(0, 1);
    double currentEnergy;
    double modifiedEnergy;
    
    while (temperature > 1)
    {
        modified = current;
        
        iter_swap(modified.begin() + randVectorIndex(generator),
                  modified.begin() + randVectorIndex(generator));
        
        currentEnergy = getCrowDistance(current, depot);
        modifiedEnergy = getCrowDistance(modified, depot);
        
        if (modifiedEnergy < currentEnergy)
            lowest = current = modified;
        else if (exp((currentEnergy - modifiedEnergy) / temperature) > randZeroToOne(generator))
            current = modified;
       
        numLoopsForTemp--;
        if (numLoopsForTemp < 1)
        {
            temperature *= pctHeatRetained;
            numLoopsForTemp = MAX_LOOPS_TEMP;
        }
    }
    
    if (getCrowDistance(current, depot) < oldCrowDistance)
        deliveries = current;
    else if (getCrowDistance(lowest, depot) < oldCrowDistance)
        deliveries = lowest;
    
    newCrowDistance = getCrowDistance(deliveries, depot);
}

double DeliveryOptimizerImpl::getCrowDistance(const vector<DeliveryRequest>& deliveries, const GeoCoord& origin) const
{
    double distance = 0;
    GeoCoord previous = origin;
    for (auto it = deliveries.begin(); it != deliveries.end(); ++it)
    {
        distance += distanceEarthMiles(previous, it->location);
        previous = it->location;
    }
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
