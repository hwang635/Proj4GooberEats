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
    GeoCoord start = depot;
    GeoCoord next;
    oldCrowDistance = 0;
    vector<DeliveryRequest> oldDeliveries = deliveries;

    //Calculate origi crow/straight-line dist from depot to 1st delivery ==> next delivery
    for (int i = 0; i < deliveries.size(); i++) {
        next = deliveries[i].location; //Next delivery loc
        oldCrowDistance += distanceEarthMiles(start, next);
        start = next;
    }
    //Add distance from last delivery ==> back to depot
    oldCrowDistance += distanceEarthMiles(start, depot);

    deliveries.clear();
    double distFromDepot;
    vector<DeliveryRequest>::iterator oldItr = oldDeliveries.begin();
    vector<DeliveryRequest>::iterator searchItr;
    //Loop through each DeliveryReq in oldDeliveries
    while (!oldDeliveries.empty() && oldItr != oldDeliveries.end()) {
        DeliveryRequest current = *oldItr;
        distFromDepot = distanceEarthMiles(depot, current.location);

        deliveries.push_back(current); //Add to deliveries + erase from oldDeliveries
        oldItr = oldDeliveries.erase(oldItr);

        searchItr = oldDeliveries.begin();
        //Search through other DeliveryRequests to see if they are located near current req
        while (!oldDeliveries.empty() && searchItr != oldDeliveries.end()) {
            //If the req is w/in 1/5 of the dist from depot to current req, from the current req it is "close"
            if (distanceEarthMiles(searchItr->location, current.location) < distFromDepot / 5) {
                deliveries.push_back(current);
                searchItr = oldDeliveries.erase(searchItr);
            }
            else
                searchItr++;
        }
    }

    //Compute new crow distance from re-ordered deliveries
    for (int i = 0; i < deliveries.size(); i++) {
        next = deliveries[i].location; //Next delivery loc
        newCrowDistance += distanceEarthMiles(start, next);
        start = next;
    }
    newCrowDistance += distanceEarthMiles(start, depot);
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
