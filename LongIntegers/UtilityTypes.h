#pragma once

#ifndef _UTILITYTYPES_H_
#define _UTILITYTYPES_H_

// Based on the code of Roman Pasechnik. Modified to work with my LongInteger class

#include <vector>
#include <set>
#include <map>
class LongInteger; // Forward definition to avoid circular includes. Will generitisate this once I understand what is happening

/** Vector of integers */
typedef std::vector<int> IntVec;

/** Vector of booleans */
typedef std::vector<bool> BoolVec;

/** Set of big integers */
typedef std::multiset<LongInteger> LongIntSet; // Multiset needed as it can contain duplicate values. std::set doesn't allow duplicates

/** Vector of LongIntegers */
typedef std::vector<LongInteger> LongIntVec;

#endif
