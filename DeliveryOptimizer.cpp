#include "provided.h"
#include <vector>
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
    const StreetMap* m_streetmap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm) {
    m_streetmap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl() {
}

//Determine orig crow's distance in miles from depot ==> each delivery coord in init order ==> back to depot
//Optimise delivery order in some way to reduce overall travel dist
//Determine new crow's distance of reordered deliveries ==> sb O(N^4)
void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot, vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance, double& newCrowDistance) const {

    GeoCoord current = depot;
    GeoCoord next;
    oldCrowDistance = 0;

    //Calculate orig distance from depot to 1st delivery, then from delivery to next delivery
    for (int i = 0; i < deliveries.size(); i++) {
        next = deliveries[i].location; //Get next delivery loc
        oldCrowDistance += distanceEarthMiles(current, next);

        current = next; //Next segment of GC to GC
    }
    //When all deliveries have been counted, find distance from last delivery back to depot
    oldCrowDistance += distanceEarthMiles(current, depot);

    //For now, optimiseDeliveryOrder doesn't optimise so newCrowDist == old
    newCrowDistance = oldCrowDistance;
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
