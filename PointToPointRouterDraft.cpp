#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <cmath>
#include <map>
using namespace std;

/*class PointToPointRouterImpl
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

    double calculateF(const StreetSegment& sg, const GeoCoord& start, const GeoCoord& end) const;
    double getEuclideanDistance(double x1, double y1, double x2, double y2) const;
    //Holds start + end GeoCoord, distance from route start + end points
    /*struct SegmentNode {
        SegmentNode(const GeoCoord& segStart, const GeoCoord& segEnd, const GeoCoord& start, const GeoCoord& end) {
            m_segStart = segStart;
            m_segEnd = segEnd;
            m_distanceFromStart = getEuclideanDistance(start.latitude, start.longitude,
                segStart.latitude, segStart.longitude);
            m_distanceToEnd = getEuclideanDistance(segEnd.latitude, segEnd.longitude,
                end.latitude, end.longitude);
        }

        GeoCoord m_segStart, m_segEnd;
        double m_distanceFromStart = -1, m_distanceToEnd = -1;

        //Returns euclidean distance btw (x1, y1) and (x1, y1)
        double getEuclideanDistance(double x1, double y1, double x2, double y2) {
            double distance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
            distance = sqrt(distance);

            return distance;
        } 
    }; 

    //Holds start + end GeoCoord of segment, distance from route start + end points = f
    struct StreetSegmentNode {
        StreetSegmentNode(const StreetSegment& seg, const GeoCoord& start, const GeoCoord& end) {
            m_seg = seg;
            m_distanceFromStart = getEuclideanDistance(start.latitude, start.longitude,
                seg.start.latitude, seg.start.longitude);
            m_distanceToEnd = getEuclideanDistance(seg.end.latitude, seg.end.longitude,
                end.latitude, end.longitude);
            f = m_distanceFromStart + m_distanceToEnd;
        }

        StreetSegmentNode() {}
        StreetSegment m_seg;
        double f = -1;
        double m_distanceFromStart = -1, m_distanceToEnd = -1;
    private:
        //Returns euclidean distance btw (x1, y1) and (x1, y1)
        double getEuclideanDistance(double x1, double y1, double x2, double y2) {
            double distance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
            distance = sqrt(distance);

            return distance;
        }
    };

    //Nested class to compare StreetSegment nodes (hold segment + distance from start, end)
    class CompareSegmentNodes {
    public:
        //Returns true if StreetSegment s1 > s2
        //s1 > s2 if s1 has greater f = g + h, f = euclidean dist from start pt to s, h = dist from end pt to s
        bool operator() (const StreetSegmentNode& s1, const StreetSegmentNode& s2) {
            double f1 = s1.m_distanceFromStart + s1.m_distanceToEnd;
            double f2 = s2.m_distanceFromStart + s2.m_distanceToEnd;
            return f1 > f2;
        }
    };
};

//Constructor, set m_streetmap
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) {
    m_streetmap = sm;
}

//Destructor, if needed to dealloc memory
PointToPointRouterImpl::~PointToPointRouterImpl() {
}

//Sets route equal to path including start + end StreetSegments, sb O(N)
//Sets totalDistanceTravelled = total length in miles of segments traversed
DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(const GeoCoord& start, const GeoCoord& end,
        list<StreetSegment>& route, double& totalDistanceTravelled) const {
    //If start + end are the same, clear route + totalDistTravelled = 0
    if (start.latitude == end.latitude && start.longitude == end.longitude) {
        route.clear();
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }

    //Otherwise, do A* alg search ==> create open list (minheap) + closed list (map segment => previous seg)

    //Create minheap of GeoCoordNode(GeoCoord, dist from start + end), minheap.top() always = least node
    priority_queue < StreetSegmentNode, vector<StreetSegmentNode>, CompareSegmentNodes> minheap;
    //Maps current loc to prev loc connecting/leading to it, segments that have been added to minheap
    ExpandableHashMap<GeoCoord, StreetSegmentNode> foundCoordToSegment;
    //Closed list ==> maps GeoCoord to StreetSegmentdc
    ExpandableHashMap<GeoCoord, StreetSegment> closedListPath;

    //Put start node on open list/min heap, f = 0
    StreetSegmentNode startNode = StreetSegmentNode(StreetSegment(start, start, "removelater"), start, end);
    startNode.m_distanceFromStart = 0;
    startNode.m_distanceToEnd = 0;
    minheap.push(startNode);

    //While the open list/minheap isn't empty
    while (!minheap.empty()) {
        //Find node currentSeg w/ least f (dist from start + end) on open list/minheap, remove it from minheap
        //Map StreetSegment start coord to its corresponding node
        StreetSegmentNode currentSeg = minheap.top();
        foundCoordToSegment.associate(currentSeg.m_seg.start, currentSeg);
        minheap.pop();

        //Find all StreetSegments that start w/ the GeoCoord the current segment ends w/
        GeoCoord nextStart = currentSeg.m_seg.end;
        vector<StreetSegment> foundSegments;

        //If found StreetSegments that start w/ this GeoCoord, process them
        if (m_streetmap->getSegmentsThatStartWith(nextStart, foundSegments)) {
            for (int i = 0; i < foundSegments.size(); i++) {
                StreetSegment sg = foundSegments[i];
                //If start/end coord of StreetSeg = destination, path has been found
                if ((sg.start.latitude == end.latitude && sg.start.longitude == end.longitude)
                    || (sg.end.latitude == end.latitude && sg.end.longitude == end.longitude)) {
                    foundCoordToSegment.associate(sg.start, StreetSegmentNode(sg, start, end));
                    closedListPath.associate(sg.start, sg);
                    break;
                }
                //If StreetSeg already in minheap has < f than possible next segment successor sg, skip sg
                StreetSegmentNode* prevFoundNode = foundCoordToSegment.find(sg.start);
                if (prevFoundNode != nullptr && prevFoundNode->f > calculateF(sg, start, end))
                    continue; //skip sg, go to next iteration of loop
                //If StreetSeg already in closed path list has < f than possible successor sg, skip sg
                StreetSegment* nodeInPath = closedListPath.find(sg.start);
                if (nodeInPath != nullptr &&
                    calculateF(*nodeInPath, start, end) > calculateF(sg, start, end))
                    continue; //skip sg, go to next iteration of loop

                //If none of above = true, add segment to open list
                StreetSegmentNode newFoundSegment(sg, start, end);
                minheap.push(newFoundSegment);
                foundCoordToSegment.associate(sg.start, newFoundSegment);
            } //end of successors for
        }
        //If didn't find any, there is no more path
        else
            return NO_ROUTE;

        //After processing all next segments, put current segment node on closed list/map
        closedListPath.associate(currentSeg.m_seg.start, currentSeg.m_seg);
    } //end of minheap for
   
    //Testing minheap, if minheap = GeoCoord
    GeoCoord g1("5", "6");
    GeoCoord g2("50", "6");
    GeoCoord g3("0", "6");
    GeoCoord g4("1", "-1");
    GeoCoord g6("6", "50");
    GeoCoord g7("6", "50.1");
    minheap.push(GeoCoordNode(g1, start, end));
    minheap.push(GeoCoordNode(g2, start, end));
    minheap.push(GeoCoordNode(g3, start, end));
    minheap.push(GeoCoordNode(g4, start, end));
    minheap.push(GeoCoordNode(g6, start, end));
    minheap.push(GeoCoordNode(g7, start, end));

    //Extract each item from minheap
    cerr << "should be smallest to greatest distance" << endl;
    while (!minheap.empty()) {
        GeoCoord g = minheap.top().m_geo;
        cerr << "GeoCoord= (" << g.latitude << ", " << g.longitude << ") ";
        cerr << "distanceFromStart= " << minheap.top().m_distanceFromStart << ", ";
        cerr << "distanceToEnd= " << minheap.top().m_distanceToEnd << endl;
        minheap.pop();

    }

    //Set route equal to closedListPath, closedListPath
    //ExpandableHashMap.find

    return DELIVERY_SUCCESS;
}

//calculate f = g + h = sg distance from start + distance from end
double PointToPointRouterImpl::calculateF(const StreetSegment& sg, 
    const GeoCoord& start, const GeoCoord& end) const {
    double distFromStart = getEuclideanDistance(sg.start.latitude, sg.start.longitude,
        start.latitude, start.longitude);
    double distToEnd = getEuclideanDistance(sg.end.latitude, sg.end.longitude,
        end.latitude, end.longitude);
    
    return distFromStart + distToEnd;
}

//Returns euclidean distance btw (x1, y1) and (x1, y1)
double PointToPointRouterImpl::getEuclideanDistance(double x1, double y1, double x2, double y2) const {
    double distance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
    distance = sqrt(distance);

    return distance;
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
} */
