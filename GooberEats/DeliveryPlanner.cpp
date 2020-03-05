//
//  DeliveryPlanner.cpp
//  GooberEats
//
//  Created by Aristotle Henderson.
//  Copyright © 2020 Aristotle Henderson. All rights reserved.
//

class DeliveryPlanner
{
public:
    DeliveryPlanner(const StreetMap* sm); ~DeliveryPlanner();
    DELIVERY_RESULT generateDeliveryPlan(const GeoCoord& depotLocation,
                                         const std::vector<DeliveryRequest>& deliveries,
                                         std::vector<DeliveryCommand>& commands,
                                         double& totalDistanceTravelled) const;
};
