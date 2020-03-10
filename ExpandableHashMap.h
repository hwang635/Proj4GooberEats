// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <iostream>
#include <list>
#include <vector>

template<typename KeyType, typename ValueType>
class ExpandableHashMap {
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5); //Constructor
	~ExpandableHashMap(); //Destructor, delete all items in hashmap
	void reset(); //Resets hashmap to orig 8 buckets, delete all items
	int size() const; //Returns # of assoc in hashmap

	//Associates item key w/ item value ==> no duplicate keys in map
	//If no assoc currently exists w/ key, inserts key+value assoc pair into hashmap
	//If assoc already exists, key is re-assoc w/ new value
	void associate(const KeyType& key, const ValueType& value);

	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key) {
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

	void printHashMap() const; //COMMENT OUT LATER!
private:
	unsigned int getBucketNumber(const KeyType& key) const; //Gets bucket# by calling hash fx in StreetMap
	void insertItem(KeyType k, ValueType v); //Insert item, increment # of associations
	void cleanUp(); //Deletes contents of internal hashmap, called by destructor
	double calcLoadFactor(double numItems) const; //Calculates load of hashtable
	void rehash(); //If load factor > max, rehashes internal hashmap

	double m_maxLoadFactor;
	int m_hashArraySize;
	int m_numAssoc;
	
	struct BucketNode {
		BucketNode(KeyType k, ValueType v) {
			b_key = k;
			b_value = v;
			b_used = true;
		}

		KeyType b_key;
		ValueType b_value;
		bool b_used = false;
	};
	std::list<BucketNode> *m_hashArray; //m_hashArray = pointer to array, holds elements of type std:list, list holds BucketNode
};

//COMMENT OUT LATER!!
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::printHashMap() const {
	for (int i = 0; i < m_hashArraySize; i++) {
		std::cerr << "Bucket # " << i << ": ";
		typename std::list<BucketNode>::iterator itr = m_hashArray[i].begin();
		int count = 0;
		while (itr != m_hashArray[i].end()) {
			std::cerr << "BucketNode" << count 
				<< " key=(" << itr->b_key.latitudeText  << ", " << itr->b_key.longitudeText
				<< "), value=" << itr->b_value << " ";
			count++;
			itr++;
		}
		std::cerr << std::endl;
	}

	std::cerr << "Num of assoc = " << this->size() << std::endl;
}

//------Private fx implementations
//Gets bucket# by calling hash fx in StreetMap, make hashed # = to a slot in internal m_hashArray
template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNumber(const KeyType& key) const {
	unsigned int hasher(const KeyType & key); //fx prototype declaration to prevent errors

	unsigned int hashedNum = hasher(key); //get hashed #
	hashedNum = hashedNum % m_hashArraySize; //get hashed # as number within hashArray's # of buckets

	return hashedNum;
}

//Inserts BucketNode w/ key v and value v into hashmap @ appropriate bucket slot, increments numAssoc
template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::insertItem(KeyType k, ValueType v) {
	int bucketNum = getBucketNumber(k); //Get bucketNum btw 0, hashArraySize-1
	BucketNode b(k, v); //Create BucketNode w/ key k and value v
	
	m_hashArray[bucketNum].push_back(b); //Inserts b at end of list @ slot bucketNum, push_back has ~O(1)
	m_numAssoc++;
}

//Delete contents of internal hashmap, array that holds internal hashmap
//Sets # of assoc to 0
template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::cleanUp() {
	//Loop through each slot in hashmap array
	for (int i = 0; i < m_hashArraySize; i++) {
		//Iterate through all nodes in list ==> if empty, begin will = end so won't do anything
		typename std::list<BucketNode>::iterator itr = m_hashArray[i].begin();
		while (itr != m_hashArray[i].end()) {
			//Erase each linkedlist node from list
			//std::cerr << "deleted " << itr->b_key << " from list " << std::endl;
			itr = m_hashArray[i].erase(itr);
		}
	}

	m_numAssoc = 0;
	delete[] m_hashArray; //delete map
	m_hashArray = nullptr; //set to nullptr in case try to delete later
}

//Calculates load factor hash table, load = max # of values to add / # of buckets in array
template <typename KeyType, typename ValueType>
double ExpandableHashMap<KeyType, ValueType>::calcLoadFactor(double numItems) const {
	return numItems / m_hashArraySize; //numItems should usu be m_numAssoc+1
}

//Creates new internal hashmap w/ 2x current # buckets, rehashes all items into new map
//Replaces orig map w/ new map, frees memory from orig map
template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::rehash() {
	//Create (non-dynam alloc) copy of orig/current hashmap as vector
	//Not array, bc arrays need to be init w/ constant size)
	std::vector <std::list<BucketNode>> tempOrigHash(m_hashArraySize);

	//Loop through orig/curr hashmap, transfer each list from orig hashmap into temp copy, splice(starting post, list to trans)
	for (int i = 0; i < m_hashArraySize; i++)
		tempOrigHash[i].splice(tempOrigHash[i].begin(), m_hashArray[i]);

	cleanUp(); //Deletes orig hashmap, sets # of assoc to 0

	//Set internal hashmap = new 2x hashmap
	m_hashArraySize = m_hashArraySize * 2;
	m_hashArray = new std::list<BucketNode>[m_hashArraySize];

	//Inserts each item from orig hashmap into new hashmap, insertItem increments # of assoc
	for (int i = 0; i < m_hashArraySize/2; i++) {
		typename std::list<BucketNode>::iterator itr = tempOrigHash[i].begin();
		while (itr != tempOrigHash[i].end()) {
			insertItem(itr->b_key, itr->b_value);
			itr++;
		}
	}
}

//------ PUBLIC fx implementations
template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) {
	m_maxLoadFactor = maximumLoadFactor;
	m_hashArraySize = 8; //starts w/ 8 buckets
	m_numAssoc = 0; //hash map is empty, 0 assoc

	m_hashArray = new std::list<BucketNode> [m_hashArraySize]; //hasharray has size m_hashArraySize (starts @ 8), each list empty
}

template <typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap() {
	cleanUp(); //calls cleanUp fx to delete everything
}

//Resets hashmap to orig: 8 buckets, empty, all contents deleted
template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset() {
	cleanUp(); //delete former internal hashmap, set numAssoc to 0

	m_hashArraySize = 8;
	m_hashArray = new std::list<BucketNode>[m_hashArraySize]; //reset m_hashArray to 8 empty buckets
}

//Returns # of assoc in hashmap
template <typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const {
	return m_numAssoc;
}

//Associates item key w/ item value ==> no duplicate keys in map, should have O(1)
//If no assoc currently exists w/ key, inserts key+value assoc pair into hashmap
//If assoc already exists, key is re-assoc w/ new value
template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value) {
	//Call find to search through map + look for key
	ValueType* v = find(key);

	//If returned nullptr, didn't find existing key assoc, so insert new key+value pair
	if (v == nullptr) {
		//If adding new item would go over max load factor, call rehash to rehash new 2x bucket table
		std::cerr << "calcloadfactor, returns " << calcLoadFactor(m_numAssoc + 1.0) << std::endl;
		if (calcLoadFactor(m_numAssoc + 1.0) > m_maxLoadFactor) {
			rehash();
			//std::cerr << "rehash when add " << key << std::endl;
		}
		//Add new item
		insertItem(key, value);
	}
	else //If returned !nullptr, key+value pair already exists ==> replace orig value w/ new value
		*v = value;
}


//Return pointer to ValueType key is assoc w/ if key is in the map, nullptr if key isn't in map ==> O(1)
template <typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const {
	int bucketNum = getBucketNumber(key); //Get bucketNum btw 0, hashArraySize-1

	//Search through list @ slot bucketNum in hashArray for BucketNode w/ matching key
	typename std::list<BucketNode>::iterator itr = m_hashArray[bucketNum].begin();
	while (itr != m_hashArray[bucketNum].end()) {
		if ((*itr).b_key == key) //If found, return value
			return &((*itr).b_value);
		itr++;
	}

    return nullptr; //If didn't find key + no assoc, return nullptr
}

