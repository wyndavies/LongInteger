#pragma once

#ifndef _UTILITYTYPES_H_
#define _UTILITYTYPES_H_

// Based on the code of Roman Pasechnik. Modified to work with my LongInteger class

#include <vector>
#include <set>
#include <map>
#include <queue>

class LongInteger; // Forward definition to avoid circular includes. Will generitisate this once I understand what is happening

/** Vector of integers */
typedef std::vector<int> IntVec;

/** Vector of booleans */
typedef std::vector<bool> BoolVec;

/** Set of big integers */
typedef std::multiset<LongInteger> LongIntSet; // Multiset needed as it can contain duplicate values. std::set doesn't allow duplicates

/** Vector of LongIntegers */
typedef std::vector<LongInteger> LongIntVec;

// Attempting to add replacement types that can handle more than 4 billion entries
// A map can handle 8^8 entries, which is a start, although I would like to go to something that could potentially hold 2^32^8 entries
// even though 8^8 entries would exceed the RAM capacity on any normal machine
/** Map of LongIntegers */
typedef std::map<LongInteger, LongInteger> LongIntMap;

typedef std::queue<LongInteger> LongIntQueue;

#endif
