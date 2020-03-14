#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "ExpandableHashMap.h" //COMMENT OUT LATER
void testExpandableHashMap(); //COMMENT OUT LATER
void testStreetMap(); //COMMENT OUT LATER
void testPointToPointRouter(); //COMMENT OUT LATER
void testDeliveryOptimizer(); //COMMENT OUT LATER
void testDeliveryPlanner(); //COMMENT OUT LATER

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

int main(int argc, char *argv[])
{
    /*if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " mapdata.txt deliveries.txt" << endl;
        return 1;
    }

    StreetMap sm;
        
    if (!sm.load(argv[1]))
    {
        cout << "Unable to load map data file " << argv[1] << endl;
        return 1;
    }

    GeoCoord depot;
    vector<DeliveryRequest> deliveries;
    if (!loadDeliveryRequests(argv[2], depot, deliveries))
    {
        cout << "Unable to load delivery request file " << argv[2] << endl;
        return 1;
    }

    cout << "Generating route...\n\n";

    DeliveryPlanner dp(&sm);
    vector<DeliveryCommand> dcs;
    double totalMiles;
    DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
    if (result == BAD_COORD)
    {
        cout << "One or more depot or delivery coordinates are invalid." << endl;
        return 1;
    }
    if (result == NO_ROUTE)
    {
        cout << "No route can be found to deliver all items." << endl;
        return 1;
    }
    cout << "Starting at the depot...\n";
    for (const auto& dc : dcs)
        cout << dc.description() << endl;
    cout << "You are back at the depot and your deliveries are done!\n";
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << totalMiles << " miles travelled for all deliveries." << endl; */

    //testExpandableHashMap();
    cerr.setf(ios::fixed);
    cerr.precision(7);

    //testStreetMap();
    //testPointToPointRouter();

    //testDeliveryOptimizer();
    testDeliveryPlanner();
}

void testDeliveryPlanner() {
    StreetMap sm;
    sm.load("mapdata.txt");

    GeoCoord depot("34.0625329", "-118.4470263");
    GeoCoord chicken("34.0712323", "-118.4505969");
    GeoCoord salmon("34.0687443", "-118.4449195");
    GeoCoord beer("34.0685657", "-118.4489289");

    DeliveryRequest depotReq("CHICKEN tenders", depot);
    DeliveryRequest chickenReq("CHICKEN tenders", chicken);
    DeliveryRequest salmonReq("B-Plate SLAMON", salmon);
    DeliveryRequest beerReq("Pabst Blue Ribbon BEER", beer);

    vector<DeliveryRequest> requests;
    requests.push_back(salmonReq);
    requests.push_back(chickenReq);
    requests.push_back(salmonReq);
    vector<DeliveryCommand> commands;
    double totalDist = 0;

    DeliveryPlanner planner(&sm);
    DeliveryResult result = planner.generateDeliveryPlan(depot, requests, commands, totalDist);
    if (result == BAD_COORD)
        cerr << "BAD COORD" << endl;
    else if (result == NO_ROUTE)
        cerr << "NO ROUTE" << endl;
    else
        cerr << "DELIVERY SUCCESSFUL" << endl;
    for (int i = 0; i < commands.size(); i++) {
        cerr << commands[i].description() << endl;
    }
    cerr << "totalDistTravelled = " << totalDist << endl;

    /*requests.clear();
    requests.push_back(salmonReq);
    result = planner.generateDeliveryPlan(depot, requests, commands, totalDist);
    if (result == BAD_COORD)
        cerr << "BAD COORD" << endl;
    else if (result == NO_ROUTE)
        cerr << "NO ROUTE" << endl;
    else
        cerr << "DELIVERY SUCCESSFUL" << endl;
    for (int i = 0; i < commands.size(); i++) {
        cerr << commands[i].description() << endl;
    }
    cerr << "totalDistTravelled = " << totalDist << endl; */

}

void testDeliveryOptimizer() {
    GeoCoord depot("0", "0");
    GeoCoord start("34", "118");
    GeoCoord end("35", "120");
    DeliveryRequest req1("yum", start);
    DeliveryRequest req2("yum", end);
    DeliveryRequest dep("yum", depot);

    StreetMap sm;
    sm.load("mapdata.txt");

    vector<DeliveryRequest> deliveries;
    deliveries.push_back(req1);
    deliveries.push_back(req2);
    deliveries.push_back(dep);
    double oldCrow, newCrow;
    DeliveryOptimizer opt(&sm);
    opt.optimizeDeliveryOrder(depot, deliveries, oldCrow, newCrow);
    cerr << "oldCrow= " << oldCrow << endl;
    cerr << "newCrow= " << newCrow << endl;
}


void testPointToPointRouter() {
    StreetMap sm;
    sm.load("mapdata.txt");
    PointToPointRouter prouter(&sm);

    GeoCoord start("34.0625329", "-118.4470263");
    GeoCoord end("34.0712323", "-118.4505969");
    list<StreetSegment> route;
    double dist = 0;
    DeliveryResult dr = prouter.generatePointToPointRoute(start, end, route, dist);
    if (dr == BAD_COORD)
        cerr << "BAD coord" << endl;
    else if (dr == NO_ROUTE)
        cerr << "NO route!" << endl;
    else
        cerr << "found route" << endl;
    list<StreetSegment>::iterator itr = route.begin();
    int count = 0;
    cerr << "route from start: (" << start.latitude << ", " << start.longitude << ") to "
        << "end: (" << end.latitude << ", " << end.longitude << ")" << endl;
    while (itr != route.end()) {
        itr->printSegment();
        count++;
        itr++;
    }
    cerr << "route finished printing, # of segments = " << route.size()
        << ", totalDistTravelled = " << dist << endl;

    start = GeoCoord("34.0712323", "-118.4505969");
    DeliveryResult dr2 = prouter.generatePointToPointRoute(start, end, route, dist);
    if (dr2 == BAD_COORD)
        cerr << "BAD coord" << endl;
    else if (dr2 == NO_ROUTE)
        cerr << "NO route!" << endl;
    else
        cerr << "found route" << endl;
    itr = route.begin();
    count = 0;
    cerr << "route from start: (" << start.latitude << ", " << start.longitude << ") to "
        << "end: (" << end.latitude << ", " << end.longitude << ")" << endl;
    while (itr != route.end()) {
        itr->printSegment();
        count++;
        itr++;
    }
    cerr << "route finished printing, # of segments sb 0 = " << route.size()
        << ", totalDistTravelled sb 0 = " << dist << endl;

    start = GeoCoord("34.0666168", "-118.4395786");
    end = GeoCoord("34.0683189", "-118.4536522");
    DeliveryResult dr3 = prouter.generatePointToPointRoute(start, end, route, dist);
    if (dr3 == BAD_COORD)
        cerr << "BAD coord" << endl;
    else if (dr3 == NO_ROUTE)
        cerr << "NO route!" << endl;
    else
        cerr << "found route" << endl;
    itr = route.begin();
    count = 0;
    cerr << "route from start: (" << start.latitude << ", " << start.longitude << ") to "
        << "end: (" << end.latitude << ", " << end.longitude << ")" << endl;
    while (itr != route.end()) {
        itr->printSegment();
        count++;
        itr++;
    }
    cerr << "route finished printing, # of segments = " << route.size()
        << ", totalDistTravelled = " << dist << endl;
}

void testStreetMap() {
    StreetMap streetmap;

    streetmap.load("minimapdata.txt");

    /*vector<StreetSegment> segs;
    GeoCoord g("34.0732851", "-118.4931016");
    streetmap.getSegmentsThatStartWith(g, segs);
    for (int i = 0; i < segs.size(); i++) {
        cerr << "(" << segs[i].start.latitude << ", " << segs[i].start.longitude << "), "
            << "(" << segs[i].end.latitude << ", " << segs[i].end.longitude << "), "
            << segs[i].name << endl;
    }
    GeoCoord g1("34.0724746", "-118.4923463");
    cerr << endl;
    streetmap.getSegmentsThatStartWith(g1, segs);
    for (int i = 0; i < segs.size(); i++) {
        cerr << "(" << segs[i].start.latitude << ", " << segs[i].start.longitude << "), "
            << "(" << segs[i].end.latitude << ", " << segs[i].end.longitude << "), "
            << segs[i].name << endl;
    }
    GeoCoord g2("123", "-118.4923463");
    GeoCoord g3("34.0724746", "118.4923463");
    streetmap.getSegmentsThatStartWith(g2, segs);
    cerr << "same thing should follow this line 2x" << endl;
    for (int i = 0; i < segs.size(); i++) {
        cerr << "(" << segs[i].start.latitude << ", " << segs[i].start.longitude << "), "
            << "(" << segs[i].end.latitude << ", " << segs[i].end.longitude << "), "
            << segs[i].name << endl;
    }
    vector<StreetSegment> empty;
    streetmap.getSegmentsThatStartWith(g3, segs);
    streetmap.getSegmentsThatStartWith(g, empty);
    for (int i = 0; i < segs.size(); i++) {
        cerr << "(" << segs[i].start.latitude << ", " << segs[i].start.longitude << "), "
            << "(" << segs[i].end.latitude << ", " << segs[i].end.longitude << "), "
            << segs[i].name << endl;
    } */
}

void testExpandableHashMap() { //COMMENT OUT LATER!
    ExpandableHashMap<GeoCoord, string> geo(0.3);

    GeoCoord g1("100", "200");
    GeoCoord g2("-1231", "300");
    GeoCoord g3("0.33", "33.3");
    geo.associate(g1, "num1");
    geo.associate(g2, "num2");
    geo.associate(g3, "num3!!");

    geo.printHashMap();
    geo.reset();
    cerr << "reset hashmap, sb 8 empty buckets" << endl;
    geo.printHashMap();

     //ExpandableHashMap<string, int>* hashmap = new ExpandableHashMap<string, int>;
    //delete hashmap;

    //Define hashmap that maps strings ==> doubles, max load factor of 0.3, init has 8 empty buckets
    /*ExpandableHashMap<string, double> nameToGPA(0.3);
    nameToGPA.printHashMap(); //sb empty, 8 buckets
    cerr << "size = " << nameToGPA.size() << endl; //sb 8

    //Add new items to hashmap
    //Insert 3rd item should cause hashmap to ++ # of buckets, rehash items
    nameToGPA.associate("Carey", 3.5); // Carey has a 3.5 GPA
    nameToGPA.associate("David", 3.99); // David has a 3.99 GPA

    nameToGPA.printHashMap();

    nameToGPA.associate("Abe", 3.2); // Abe has a 3.2 GPA
    nameToGPA.associate("Carey", 1.05); //Carey now has a 1.0 GPA
    nameToGPA.associate("Bob", 2.222); //Bob has 2.222 GPA
    nameToGPA.associate("verygood", 5.0);
    nameToGPA.associate("verygood", 5.5); //verygood now = 5.5 GPA
    nameToGPA.associate("Cat", 1.5);
    nameToGPA.associate("Dog", 1.5);
    nameToGPA.associate("cat", -2.5);
    nameToGPA.associate("rabbit", -0.1);
    nameToGPA.associate("bird", 3.9);

    nameToGPA.printHashMap(); //should be rehashed

    double* davidsGPA = nameToGPA.find("David");
    if (davidsGPA != nullptr)
        *davidsGPA = 1.5; //David sb 1.5
    nameToGPA.associate("Carey", 4.0); // Carey deserves a 4.0
    double* lindasGPA = nameToGPA.find("Linda");
    if (lindasGPA == nullptr)
        cout << "Linda is not in the roster!" << endl;
    else
        cout << "Linda’s GPA is: " << *lindasGPA << endl;
    if (*(nameToGPA.find("Cat")) == 1.5 && *(nameToGPA.find("rabbit")) == -0.1)
        cout << "Cat has 1.5 GPA, rabbit has -0.1" << endl;

    nameToGPA.reset(); //sb 8 empty buckets
    cerr << "reset map, sb 8 empty buckets" << endl;
    nameToGPA.printHashMap(); */
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v)
{
    ifstream inf(deliveriesFile);
    if (!inf)
        return false;
    string lat;
    string lon;
    inf >> lat >> lon;
    inf.ignore(10000, '\n');
    depot = GeoCoord(lat, lon);
    string line;
    while (getline(inf, line))
    {
        string item;
        if (parseDelivery(line, lat, lon, item))
            v.push_back(DeliveryRequest(item, GeoCoord(lat, lon)));
    }
    return true;
}

bool parseDelivery(string line, string& lat, string& lon, string& item)
{
    const size_t colon = line.find(':');
    if (colon == string::npos)
    {
        cout << "Missing colon in deliveries file line: " << line << endl;
        return false;
    }
    istringstream iss(line.substr(0, colon));
    if (!(iss >> lat >> lon))
    {
        cout << "Bad format in deliveries file line: " << line << endl;
        return false;
    }
    item = line.substr(colon + 1);
    if (item.empty())
    {
        cout << "Missing item in deliveries file line: " << line << endl;
        return false;
    }
    return true;
}
