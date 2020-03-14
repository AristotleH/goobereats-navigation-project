#ifndef ExpandableHashMap_h
#define ExpandableHashMap_h

#include <list>
#include <iostream>

// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
    ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();
    void reset();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);

      // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;

      // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
    }

      // C++11 syntax for preventing copying and assignment
    ExpandableHashMap(const ExpandableHashMap&) = delete;
    ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    /*
     Private constants (will be used to reset map)
     */
    const double MAX_LOAD;
    const unsigned int INIT_BUCKETS;

    /*
     Struct with which associations are stored in lists pointed to by buckets.
     */
    struct Pair
    {
        KeyType m_key;
        ValueType m_value;
        Pair(KeyType key, ValueType val) : m_key(key), m_value(val) {}
    };

    /*
     Private data members
     */
    // A pointer that a dynamically allocated array of pointers to lists will be created through.
    // In the dynamically allocated array, each element represents a hash map bucket. Each bucket is a
    // pointer that points to a list of pairs that map to the bucket's number (its array index).
    std::list<Pair>* *m_buckets; //POINTER to ARRAY OF POINTERS to LISTS containing PAIRS

    //Externally held information about the hash map
    unsigned int m_numBuckets;
    unsigned int m_numPairs;

    /*
     Private member functions
     */
    /// Uses hash function for KeyType and modulus to generate a bucket number
    unsigned int getBucketNumber(const KeyType& key) const;
    
    /// Creates another hash map with double the number of buckets, transfers all elements of the current hash map to this new hash map, and deletes the old hash map.
    void doubleBuckets();

    /// Dynamically allocates hash map array and points all bucket pointers to nullptr.
    /// @param numBuckets - number of buckets to create within hash map (size of array)
    void initializeBuckets(unsigned int numBuckets);

    /// Deallocates each bucket's List from memory, then deallocates hash map array.
    /// @param buckets - pointer to array of buckets
    /// @param numBuckets - number of buckets in the array argument
    void deleteBucketArray(std::list<Pair>* *buckets, unsigned int numBuckets);

    /// Potentially temporary function; prints information about hash map.
    void dump() const;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)

    : MAX_LOAD(maximumLoadFactor),
      INIT_BUCKETS(8),
      m_numPairs(0)
{
    // Creates bucket array of default size
    initializeBuckets(INIT_BUCKETS);
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    // deletes hash map array
    deleteBucketArray(m_buckets, m_numBuckets);
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    // deletes hash map array and reallocates it with default size
    deleteBucketArray(m_buckets, m_numBuckets);
    initializeBuckets(INIT_BUCKETS);
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    // size is defined by the number of pairs in the hash map
    return m_numPairs;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    // get a hash of the key (hash needs to defined and implemented in support.h/cpp) and get bucket number through modulus
    int bucketNum = getBucketNumber(key);
    
    // get reference to bucket's pointer to the list that the pair will be sent to
    std::list<Pair>* &bucket = *(m_buckets + bucketNum); //REFERENCE to POINTER to LIST of PAIRS, constant
    
    // if there haven't been any pairs in the bucket yet
    if (bucket == nullptr)
    {
        // point bucket pointer to a new list
        bucket = new std::list<Pair>;
        
        // add passed in pair to bucket's pair list
        // not using new because we want an object not a ptr
        bucket->push_back(Pair(key, value));
    }
    // if there is or was at least one pair in the bucket, meaning that the bucket points to a list
    else
    {
        //typename needed, also iterators are like pointers
        // check if there's a pair with the key passed into the function
        for (auto it = bucket->begin(); it != bucket->end(); ++it)
        {
            // if there is a pair with that key, change the value of that pair to the value passed into the function
            // then return from the function
            if (it->m_key == key)
            {
                it->m_value = value;
                return;
            }
        }
        //there isn't a pair with the passed-in key in the bucket, so add a pair with the key and value to the bucket
        bucket->push_back(Pair(key, value));
    }
    // there's one more pair now! if a pair was not added, the function returned before reaching this operation
    ++m_numPairs;
    
    // if the load of the hash map now exceeds the maximum load defined at construction as a result
    // of the newly-added association, then expand the hash map through doubleBuckets().
    // we do this after adding an association because, even though it results in a larger-than-maximum
    // load factor for a small amount of time during the running of this function, it is only run when a new
    // value is actually associated. if we were to add this check to expand at the beginning of this function,
    // we'd have to check if the pair passed into the function will replace an existing pair or be added as a new
    // pair, which would take more time.
    
    if (static_cast<double>(m_numPairs)/m_numBuckets > MAX_LOAD)
        doubleBuckets();
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    // get a hash of the key (hash needs to defined and implemented in support.h/cpp) and get bucket number through modulus
    int bucketNum = getBucketNumber(key);
    
    // get reference to bucket's pointer to the list that the key could be in
    std::list<Pair>* &bucket = *(m_buckets + bucketNum);
    
    // if the bucket doesn't have a list or if it does and it's empty, the key can't be present
    if (bucket == nullptr  ||  bucket->empty())
        return nullptr;
    
    // loop through bucket's pair list; if one of the keys matches the searched key, return a pointer to the key's value
    for (auto it = bucket->begin(); it != bucket->end(); ++it)
        if (it->m_key == key)
            return &(it->m_value);
    
    // the key was not found
    return nullptr;
}

/*
 Private member function implementations   ----------------------------------------------------------------------------------
 */

template <typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNumber(const KeyType& key) const
{
    unsigned int hasher(const KeyType& k); // prototype
    unsigned int h = hasher(key);
    return h % m_numBuckets;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::doubleBuckets()
{
    //expanded hash map will have double the size of the current hash map
    const unsigned int OLD_NUM_BUCKETS = m_numBuckets;
    
    //allocate new hash map array and save its pointer in a new variable; new array has above size
    std::list<Pair>* *oldBucketArray = m_buckets;
    
    initializeBuckets(m_numBuckets * 2);
    
    // bucketNum and pairToSplice will be used multiple times, so we create it once before the loops
    int bucketNum;
    typename std::list<Pair>::const_iterator pairToSplice;
    
    // loop through each bucket in original hash map array
    for (int b = 0; b < OLD_NUM_BUCKETS; ++b)
    {
        // get reference to current bucket's pointer to its list
        std::list<Pair>* &currentBucket = *(oldBucketArray + b); //REFERENCE to POINTER to LIST of PAIRS, constant
        
        // if there's no list of pairs in the bucket to transfer or if the bucket has no pairs in its list,
        // move on to next bucket
        if (currentBucket == nullptr  ||  currentBucket->empty())
            continue;
        
        // while there are pairs left in the bucket that we have yet to transfer
        while (!currentBucket->empty())
        {
            // get an iterator to the first pair in the bucket's pair list
            pairToSplice = currentBucket->begin();
            
            // get a hash of the key (hash needs to defined and implemented in support.h/cpp) and get bucket number through modulus
            bucketNum = getBucketNumber(pairToSplice->m_key);
            
            // get reference to new bucket's pointer to its list (bucket that pair belongs to)
            std::list<Pair>* &newBucket = *(m_buckets + bucketNum); //REFERENCE to POINTER to LIST of PAIRS, constant
            
            // if a list at the new bucket has yet to be created, allocate one
            if (newBucket == nullptr)
                newBucket = new std::list<Pair>;
            
            // splice the pair into its appropriate bucket, removing it from the original bucket pair list while keeping
            // existing pointers valid
            newBucket->splice(newBucket->end(), *currentBucket, pairToSplice);
        }
     }
    
    // deallocate all original buckets and the original hash map array
    deleteBucketArray(oldBucketArray, OLD_NUM_BUCKETS);
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::initializeBuckets(unsigned int numBuckets)
{
    // allocate new hash map array of a specified size
    m_buckets = new std::list<Pair>* [numBuckets];
    // set the number of buckets in the array to the number specified
    m_numBuckets = numBuckets;
    // point each bucket's pointer in the hash map array to nullptr
    for (int i = 0; i < m_numBuckets; ++i)
        *(m_buckets + i) = nullptr;
}

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::deleteBucketArray(std::list<Pair>* *buckets, unsigned int numBuckets)
{
    // deallocate each list that each bucket points to
    for (int i = 0; i < numBuckets; ++i)
        delete *(buckets + i); //m_buckets[i] is also usable here b/c [] dereferences
    // deallocate the hash map array itself
    delete [] buckets;
}

#include <iostream>

template <typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::dump() const
{
    // print number of buckets to cerr
    std::cerr << m_numBuckets << " buckets" << std::endl;
    
    // print information about each bucket and their pairs to cerr
    for (int i = 0; i < m_numBuckets; ++i)
    {
        std::cerr << "Bucket " << i << ": ";
        // if there's a list at the bucket's pointer
        if ((*(m_buckets + i)) != nullptr)
        {
            std::cerr << "size of " << (*(m_buckets + i))->size() << std::endl;
            typename std::list<Pair>::iterator it;
            for (it = (*(m_buckets + i))->begin(); it != (*(m_buckets + i))->end(); ++it)
                std::cerr << "\t" << &(it->m_key) << ", " << &(it->m_value) << std::endl;
        }
        else
            std::cerr << "None (no list)" << std::endl;
    }
}


#endif /* ExpandableHashMap_h */
