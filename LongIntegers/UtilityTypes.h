#pragma once

#ifndef _UTILITYTYPES_H_
#define _UTILITYTYPES_H_

// Copying from the code of Roman Pasechnik. Modifying it for my purposes, mostly to help me understand how it works

#include <vector>
#include <set>
#include <map>
class LongInteger; // Forward definition to avoid circular includes. Will generitisate this once I understand what is happening

/** Vector of integers */
typedef std::vector<int> IntVec;

/** Vector of booleans */
typedef std::vector<bool> BoolVec;

/** Set of big integers */
typedef std::multiset<LongInteger> LongIntSet;

/** Vector of LongIntegers */
typedef std::vector<LongInteger> LongIntVec;

#endif
