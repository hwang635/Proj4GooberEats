#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

unsigned int hasher(const string& s) {
    return std::hash<string>()(s); //calls std hash function to hash str s
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile); //mapFile = name of file
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    vector<StreetSegment> m_segments; //Stores all StreetSegments
    ExpandableHashMap<GeoCoord, int> m_startToSegment; //Maps GeoCoord to ector index of seg that starts w/ it
    ExpandableHashMap<GeoCoord, int> m_endToSegment; //Maps GeoCoord to vector index of reverse of seg that ends w/ it

    void addStreetSegment(StreetSegment& seg);
    StreetSegment reverseStreetSegment(StreetSegment& origSegment);
    void printAll() const; //COMMENT OUT LATER
};

//Print all 3 data structures
void StreetMapImpl::printAll() const {
    cerr << "Print contents of m_segments vector" << endl;
    for (int i = 0; i < m_segments.size(); i++) {
        cerr << "Slot#" << i << " Segment: start("
            << m_segments[i].start.latitude << ", " << m_segments[i].start.longitude
            << "), end(" << m_segments[i].end.latitude << ", " << m_segments[i].end.longitude
            << "), " << m_segments[i].name << endl;
    }
    cerr << "Print contents of m_startToSlot, startGeoCoord to vector slot" << endl;
    m_startToSegment.printHashMap();
    cerr << "Print contents of m_endToSlot, endGeoCoord to vector slot" << endl;
    m_endToSegment.printHashMap();
}

StreetMapImpl::StreetMapImpl() {
}

StreetMapImpl::~StreetMapImpl() {
}

//Adds StreetSegment seg to vector holding all StreetSegments, maps start + end to slot in vector
void StreetMapImpl::addStreetSegment(StreetSegment& seg) {
    //Add StreetSegment + map start GeoCoord to slot in vector
    m_segments.push_back(seg);
    m_startToSegment.associate(seg.start, m_segments.size() - 1);

    //Add reversed StreetSegment + map endGeoCoord to slot in vector
    m_segments.push_back(reverseStreetSegment(seg));
    m_endToSegment.associate(seg.end, m_segments.size() - 1);
}

//Returns StreetSegment w/ start, end coordinates reversed from origSegment
StreetSegment StreetMapImpl::reverseStreetSegment(StreetSegment& origSeg) {
    return StreetSegment(origSeg.end, origSeg.start, origSeg.name);
}

//Returns false if can't load data successfully (ie, file not found)
//Otherwise, load all data from mapFile + maps each GeoCoord to all StreetSegments assoc w/ that GeoCoord ==> O(N)
bool StreetMapImpl::load(string mapFile) {
    ifstream infile(mapFile);

    if (!infile) //Failure to open file
        return false;

    string line;
    string streetName;
    string count;
    string l1, l2, l3, l4;
    char dummy;
    //Process each line of file
    while (getline(infile, line)) {
        //To extract info from each line, create input stringstream from it ==> acts like source of input for op >>
        istringstream iss(line);

        /*If there are 4 separate values that are digits + otherwise whitespace, process the StreetSegment
          op >> returns "false" if can't extract 4 strings for the 4 lat/long coord digits
          iss >> dummy succeeds if there is non-whitespace char after the 4 lat/long coord */
        if ((iss >> l1 >> l2 >> l3 >> l4) && !(iss >> dummy)) {
            //Check that all four are #'s, not count or block name ==> either first/second char sb digit, ex: -4, 4
            if ((isdigit(l1[0]) || isdigit(l1[1])) && (isdigit(l2[0]) || isdigit(l2[1]))
                && (isdigit(l3[0]) || isdigit(l3[1])) && (isdigit(l4[0]) || isdigit(l4[1]))) {
                GeoCoord start(l1, l2);
                GeoCoord end(l3, l4);
                StreetSegment seg(start, end, streetName);

                this->addStreetSegment(seg); //Call fx to add segment, update start + end maps
            }
        }
        //If line has a letter, it is a streetName (otherwise, it is a count)
        else {
            if (isalpha(line[0]) || isalpha(line[line.size()-1]))
                streetName = line;
        }
    } //end of while getline

    printAll(); //print contents of vector + maps, COMMENT OUT LATER
    return true;
}

//Gets all StreetSegments + reversed StreetSegments w/ start GeoCoord matching gc ==> O(1)
//If found, segs should only contain found StreetSegments + return true
//Otherwise if no StreetSegments starting w/ gc, leave segs unchanged + return false
bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const {
    bool foundSegment = false;
    vector<StreetSegment> matchingSegments;
    
    const int* startSlot = m_startToSegment.find(gc);
    const int* endSlot = m_endToSegment.find(gc);

    //If find returned value that isn't nullptr, it found a slot containing a matching StreetSeg
    if (!(startSlot == nullptr)) {
        foundSegment = true;
        matchingSegments.push_back(m_segments[*startSlot]); //Add StreetSegment @ that slot in vector
    }
    if (!(endSlot == nullptr)) {
        foundSegment = true;
        matchingSegments.push_back(m_segments[*endSlot]);
    }

    //If found a matching StreetSegment, set segs = only the StreetSegments found
    if (foundSegment)
        segs = matchingSegments;
    
    return foundSegment;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
