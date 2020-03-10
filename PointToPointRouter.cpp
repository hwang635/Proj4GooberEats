#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
using namespace std;

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
    const StreetMap* m_streetmap;

    //Maps current loc to prev loc connecting/leading to it
    ExpandableHashMap<StreetSegment, StreetSegment> m_currentToPrevious;
    //Maps StreetSegment loc to bool whether it has been already been visited or not
    ExpandableHashMap<StreetSegment, bool> m_segmentVisited;
};

//Constructor, set m_streetmap
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) {
    m_streetmap = sm;
}

//Destructor, if needed to dealloc memory
PointToPointRouterImpl::~PointToPointRouterImpl() {
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(const GeoCoord& start, const GeoCoord& end,
        list<StreetSegment>& route, double& totalDistanceTravelled) const {
    //If start + end are the same, clear route + totalDistTravelled = 0
    if (start.latitude == end.latitude && start.longitude == end.longitude) {
        route.clear();
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }

    queue<GeoCoord> coords;
    coords.push(start);


    return NO_ROUTE;  // Delete this line and implement this function correctly
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
