//
//  DeliveryOptimizer.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

class DeliveryOptimizer
{
public:
    DeliveryOptimizer(const StreetMap* sm); ~DeliveryOptimizer();
    void optimizeDeliveryOrder(const GeoCoord& depot,
                               std::vector<DeliveryRequest>& deliveries,
                               double& oldCrowDistance,
                               double& newCrowDistance) const;
};

void DeliveryOptimizer::optimizeDeliveryOrder(const GeoCoord& depot,
                           std::vector<DeliveryRequest>& deliveries,
                           double& oldCrowDistance,
                           double& newCrowDistance) const;
