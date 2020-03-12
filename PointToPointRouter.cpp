#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <cmath>
#include <map>
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

    double calculateF(const StreetSegment& sg, const GeoCoord& start, const GeoCoord& end) const;

    //Holds StreetSegment, value f, previous StreetSegment that pointed to it
    struct StreetSegmentNode {
        StreetSegmentNode(const StreetSegment& seg, StreetSegmentNode* prev, double f) {
            m_seg = seg;
            m_prev = prev;
            m_f = f;
        }
        StreetSegment m_seg;
        double m_f;
        StreetSegmentNode* m_prev;
    };

    //Nested class to compare StreetSegment nodes (hold segment + distance from start, end)
    class CompareSegmentNodePtrs {
    public:
        //Returns true if StreetSegment s1 > s2
        //s1 > s2 if s1 has greater f = g + h, f = euclidean dist from start pt to s, h = dist from end pt to s
        bool operator() (const StreetSegmentNode* s1, const StreetSegmentNode* s2) {
            return s1->m_f > s2->m_f;
        }
    };

    void testminheap() const; //COMMENT OUT LATER
};

//Constructor, set m_streetmap
PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) {
    m_streetmap = sm;
    //testminheap(); //COMMENT OUT LATER
}

//Destructor, if needed to dealloc memory
PointToPointRouterImpl::~PointToPointRouterImpl() {
}

//COMMENT OUT LATER
void PointToPointRouterImpl::testminheap() const {
    priority_queue<StreetSegmentNode*, vector<StreetSegmentNode*>, CompareSegmentNodePtrs> minheap;
    StreetSegmentNode s0(StreetSegment(GeoCoord("100", "1"), GeoCoord("0", "2"), "-11.5"), nullptr, -11.5);
    StreetSegmentNode s1(StreetSegment(GeoCoord("0", "1"), GeoCoord("0", "2"), "5"), nullptr, 5);
    StreetSegmentNode s2(StreetSegment(GeoCoord("0", "2"), GeoCoord("0", "2"), "2"), nullptr, 2);
    StreetSegmentNode s3(StreetSegment(GeoCoord("10", "2"), GeoCoord("0", "2"), "8"), nullptr, 8);
    StreetSegmentNode s4(StreetSegment(GeoCoord("100", "1"), GeoCoord("0", "2"), "11.5"), nullptr, 11.5);
    minheap.push(&s4);
    minheap.push(&s0);
    minheap.push(&s2);
    minheap.push(&s3);
    minheap.push(&s1);

    cerr << "test minheap()" << endl;
    while (!minheap.empty()) {
        StreetSegmentNode snode = *(minheap.top());
        snode.m_seg.printSegment();
        minheap.pop();
    }
}

//Sets route equal to path including start + end StreetSegments, use A* alg search ==> sb O(N)
//Sets totalDistanceTravelled = total length in miles of segments traversed
DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(const GeoCoord& start, const GeoCoord& end,
    list<StreetSegment>& route, double& totalDistanceTravelled) const {
    //If start + end are the same, clear route + totalDistTravelled = 0
    if (start == end) {
        route.clear();
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }

    //Create minheap of SSNodes pointers (minheap.top() always = node w/ least f)
    priority_queue<StreetSegmentNode*, vector<StreetSegmentNode*>, CompareSegmentNodePtrs> minheap;
    vector<StreetSegmentNode*> nodesToDelete; //Nodes from minheap to deallocate later

    //Init open + closed list
    vector<StreetSegment> openList;
    ExpandableHashMap<GeoCoord, StreetSegment> closedListPath;

    //Check for invalid coord ==> if no StreetSegments start w/ given start/end, start/end = don't exist in data
    m_streetmap->getSegmentsThatStartWith(start, openList); //Check for start coord
    if (openList.empty())
        return BAD_COORD;
    openList.clear();
    m_streetmap->getSegmentsThatStartWith(end, openList); //Check for end coord
    if (openList.empty())
        return BAD_COORD;
    openList.clear();

    //Put starting StreetSegmentNode pointer onto minheap w/ f = 0
    StreetSegment dummyStart(start, start, "dummyStart");
    StreetSegmentNode* startNode = new StreetSegmentNode(dummyStart, nullptr, 0);
    minheap.push(startNode);

    while (!minheap.empty()) {
        //Get StreetSegmentNode w/ least f + pop it off minheap
        StreetSegmentNode* current = minheap.top(); //fsr, the lat/long text of the GeoCoords are erased here
        minheap.pop();
        nodesToDelete.push_back(current);

        //Associate start GeoCoord of Segment to Segment on closedListPath
        closedListPath.associate(current->m_seg.start, current->m_seg);

        //If the node contains StreetSegment that starts || ends @ the end, have reached destination so exit loop
        if ((current->m_seg.start.latitude == end.latitude && current->m_seg.start.longitude == end.longitude)
            || (current->m_seg.end.latitude == end.latitude && current->m_seg.end.longitude == end.longitude)) {
            route.clear();
            totalDistanceTravelled = 0;

            //Iterate backwards through SSegs stored in closedListPath until reach dummy start SSNode
            //Add each segment to route + distance, then go to the previous segment
            while (!(current->m_f == 0 && current->m_seg.name == "dummyStart" && current->m_prev == nullptr)) {
                StreetSegment temp = current->m_seg;
                route.push_front(temp);
                totalDistanceTravelled += distanceEarthMiles(temp.start, temp.end);
                current = current->m_prev;
            }

            //Deallocate all dynamically alloc StreetSegmentNodes left in minheap or put in nodesToDelete
            while (!minheap.empty()) {
                StreetSegmentNode* deleteNode = minheap.top();
                minheap.pop();
                delete deleteNode;
            }
            vector<StreetSegmentNode*>::iterator itr = nodesToDelete.begin();
            while (itr != nodesToDelete.end()) {
                delete (*itr);
                itr = nodesToDelete.erase(itr);
            }

            return DELIVERY_SUCCESS;
        }
        //Otherwise, look for StreetSegments that follow the current Segment and process them
        m_streetmap->getSegmentsThatStartWith(current->m_seg.end, openList);
        for (int i = 0; i < openList.size(); i++) {
            //Didn't find any Segments in path that start w/ this segment's end (would mean already visited)
            if (closedListPath.find(openList[i].end) == nullptr) {
                //Push ptr onto minheap a SSegNode holding segment, current SSg* as prev, f w/ start = current end
                StreetSegmentNode* nextNode
                    = new StreetSegmentNode(openList[i], current, calculateF(openList[i], current->m_seg.end, end));
                minheap.push(nextNode);
            }
        } //end successor search loop

        openList.clear();
    } //end of minheap while

    //minheap is empty so only need to dealloc nodesToDelete
    vector<StreetSegmentNode*>::iterator itr = nodesToDelete.begin();
    while (itr != nodesToDelete.end()) {
        delete (*itr);
        itr = nodesToDelete.erase(itr);
    }

    return NO_ROUTE;
}

//calculate f = g + h = sg distance from start + distance from end
double PointToPointRouterImpl::calculateF(const StreetSegment& sg, const GeoCoord& start, const GeoCoord& end) const {
    return distanceEarthMiles(start, sg.start) + distanceEarthMiles(sg.end, end);
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
