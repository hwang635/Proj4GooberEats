#include "provided.h"
#include <vector>
#include <list>
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
    const StreetMap* m_streetmap;
    string getProceedDirection(StreetSegment first) const;
    string getTurnDirection(StreetSegment first, StreetSegment second) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm) {
    m_streetmap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl() {
}

//Takes in depot, vector of DeliveryRequests ==> produces vector of DeliveryCommands + totalDistTravelled
//If no route = possible || invalid depot/request loc, commands + distanceTravelled can be in any state
DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot, const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands, double& totalDistanceTravelled) const {
    //Clear orig commands/dist
    commands.clear();
    totalDistanceTravelled = 0;

    //Reorder delivery requests w/ DeliveryOptimizer to reduce total travel dist
    vector<DeliveryRequest> optDeliveries = deliveries;
    double oldCrowDist = 0, newCrowDist = 0;
    DeliveryOptimizer opt(m_streetmap);
    opt.optimizeDeliveryOrder(depot, optDeliveries, oldCrowDist, newCrowDist);

    //Generate point to point routes btw depot ==> successive delivery points ==> back to depot
    PointToPointRouter pRouter(m_streetmap);
    GeoCoord startCoord = depot;
    list<StreetSegment> nextDeliveryRoute;
    double distTravelled = 0;

    //Iterate through all DeliveryRequests ==> generate PTP route + DeliveryCommands for each, add route dist
    for (int i = 0; i < optDeliveries.size(); i++) {
        DeliveryRequest nextDelivery = optDeliveries[i];
        DeliveryResult dr = //Generate PTP route from current loc to next DeliveryReq
            pRouter.generatePointToPointRoute(startCoord, nextDelivery.location, nextDeliveryRoute, distTravelled);
        totalDistanceTravelled += distTravelled; //add dist travelled to total

        if (dr == NO_ROUTE || dr == BAD_COORD) //If no route || depot/delivery loc = invalid, return
            return dr;

        //Generate sequence of DeliveryCommands from the StreetSegments route
        //This means the delivery is at current location ==> deliver immediately + go to next iteration
        if (nextDeliveryRoute.size() == 0) {
            DeliveryCommand dc;
            dc.initAsDeliverCommand(nextDelivery.item);
            commands.push_back(dc);
            continue;
        }

        //Otherwise, generate directions then deliver
        list<StreetSegment>::iterator itr = nextDeliveryRoute.begin();
        StreetSegment firstSeg = *itr;
        StreetSegment secondSeg;
        itr++;

        DeliveryCommand proceedCommand;
        proceedCommand.initAsProceedCommand(getProceedDirection(firstSeg), firstSeg.name,
            distanceEarthMiles(firstSeg.start, firstSeg.end));
        bool proceedCommandPushed = false;

        while (itr != nextDeliveryRoute.end()) { //Iterate through each StreetSegment in route, starts @ 2nd seg
            secondSeg = *itr; //secondSeg = StreetSegment in route following firstSeg
            
            //If 2 segments have same streetName, sb proceed Command ==> generate/add dist to proceed Command
            if (firstSeg.name == secondSeg.name) {
                proceedCommandPushed = false;

                if (proceedCommand.streetName() == firstSeg.name) //If same name ==> same proceed command
                    proceedCommand.increaseDistance(distanceEarthMiles(secondSeg.start, secondSeg.end));
                else { //Otherwise ==> init as new proceed command 
                    proceedCommand.initAsProceedCommand(getProceedDirection(firstSeg), firstSeg.name,
                        distanceEarthMiles(firstSeg.start, secondSeg.end));
                }
            }
            //Otherwise, the streets have diff names ==> push the proceed command + generate turn DeliveryCommand
            else {
                proceedCommandPushed = true;
                commands.push_back(proceedCommand);

                string dir = getTurnDirection(firstSeg, secondSeg);
                if (dir == "left" || dir == "right") {
                    DeliveryCommand turnCommand;
                    turnCommand.initAsTurnCommand(dir, secondSeg.name);
                    commands.push_back(turnCommand);
                }
                else { //Actually proceed command ==> continue new proceed command
                    proceedCommandPushed = false;
                    proceedCommand.initAsProceedCommand(getProceedDirection(secondSeg), secondSeg.name,
                        distanceEarthMiles(secondSeg.start, secondSeg.end));
                }
            }
            itr++;
            firstSeg = secondSeg;
        }
        //Push remaining command
        if(!proceedCommandPushed)
            commands.push_back(proceedCommand);

        //After all StreetSegments in route have been travelled, deliver the food item
        DeliveryCommand deliverCommand;
        deliverCommand.initAsDeliverCommand(nextDelivery.item);
        commands.push_back(deliverCommand);

        startCoord = firstSeg.end; //End of current route is start of next route
    }

    //After all routes have been travelled to, go back to the depot
    pRouter.generatePointToPointRoute(startCoord, depot, nextDeliveryRoute, distTravelled);
    totalDistanceTravelled += distTravelled;

    if (distTravelled == 0) //if distTravelled = 0, already at depot ==> return
        return DELIVERY_SUCCESS;

    list<StreetSegment>::iterator itr = nextDeliveryRoute.begin();
    StreetSegment firstSeg = *itr;
    StreetSegment secondSeg;
    itr++;

    DeliveryCommand proceedCommand;
    proceedCommand.initAsProceedCommand(getProceedDirection(firstSeg), firstSeg.name,
        distanceEarthMiles(firstSeg.start, firstSeg.end));
    bool proceedCommandPushed = false;

    while (itr != nextDeliveryRoute.end()) { //Iterate through each StreetSegment in route, starts @ 2nd seg
        secondSeg = *itr; //secondSeg = StreetSegment in route following firstSeg

        //If 2 segments sb proceed or have same streetName ==> add distance to proceed Command
        if (firstSeg.name == secondSeg.name) {
            proceedCommandPushed = false;

            if (proceedCommand.streetName() == firstSeg.name) //If same name ==> same proceed command
                proceedCommand.increaseDistance(distanceEarthMiles(secondSeg.start, secondSeg.end));
            else { //Otherwise ==> init as new proceed command 
                proceedCommand.initAsProceedCommand(getProceedDirection(firstSeg), firstSeg.name,
                    distanceEarthMiles(firstSeg.start, secondSeg.end));
            }
        }
        //Otherwise, the streets have diff names ==> push the proceed command + generate turn DeliveryCommand
        else {
            proceedCommandPushed = true;
            commands.push_back(proceedCommand);

            string dir = getTurnDirection(firstSeg, secondSeg);
            if (dir == "left" || dir == "right") {
                DeliveryCommand turnCommand;
                turnCommand.initAsTurnCommand(dir, secondSeg.name);
                commands.push_back(turnCommand);
            }
            else { //Actually proceed command ==> continue new proceed command
                proceedCommandPushed = false;
                proceedCommand.initAsProceedCommand(getProceedDirection(secondSeg), secondSeg.name,
                    distanceEarthMiles(secondSeg.start, secondSeg.end));
            }
        }
        itr++;
        firstSeg = secondSeg;
    }
    //Push remaining command
    if (!proceedCommandPushed)
        commands.push_back(proceedCommand); 
    //End of copied code to return to depot

    return DELIVERY_SUCCESS;
}

//Get direction of proceed DeliveryCommand ==> dir based on angle of 1st StreetSeg in command
string DeliveryPlannerImpl::getProceedDirection(StreetSegment first) const {
    double angle = angleOfLine(first);
    if (angle >= 0 && angle < 22.5)
        return "east";
    else if (angle >= 22.5 && angle < 67.5)
        return "northeast";
    else if (angle >= 67.5 && angle < 112.5)
        return "north";
    else if (angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        return "west";
    else if (angle >= 202.5 && angle < 247.5)
        return "southwest";
    else if (angle >= 247.5 && angle < 292.5)
        return "south";
    else if (angle >= 292.5 && angle < 337.5)
        return "southeast";
    else //angle >= 337.5
        return "east";
}

//Get direction of turn DeliveryCommand ==> dir based on angle btw 2 StreetSegments w/ diff names
string DeliveryPlannerImpl::getTurnDirection(StreetSegment first, StreetSegment second) const {
    double angle = angleBetween2Lines(first, second);
    if (angle < 1 || angle > 359)
        return "proceed";
    else if (angle >= 1 && angle < 180)
        return "left";
    else if(angle >= 180 && angle <= 359) //angle >= 180 && <= 359
        return "right";

    return "proceed";
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
