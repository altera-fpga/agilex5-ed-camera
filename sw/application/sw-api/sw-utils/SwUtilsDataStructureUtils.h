/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <vector>
#include <list>
#include <stdint.h>
#include <unordered_map>
#include <algorithm>

namespace SwUtils
{

    // Useful functions on STL data structures
    template<class KEY, class VALUE>
    bool Lookup(std::unordered_map<KEY, VALUE>& map, KEY key, VALUE& outValue)
    {
        auto i = map.find(key);
        if (i != map.end())
        {
            outValue = i->second;
            return true;
        }
        else
            return false;
    }

    template<class KEY, class VALUE>
    bool Lookup(std::unordered_map<KEY, VALUE>& map, KEY key, VALUE*& outValue)
    {
        auto i = map.find(key);
        if (i != map.end())
        {
            outValue = &i->second;
            return true;
        }
        else
            return false;
    }

    // Remove first item in the vector that matches searchItem. Returns
    // false if no match
    template<typename ITEM_TYPE>
    bool Remove(std::vector<ITEM_TYPE>& theVector, const ITEM_TYPE& searchItem)
    {
        // You need this mess unless you have C++ 20, then you can do the succinct 
        // std::erase(theVector, searchItem) insead
        auto removed = std::remove(theVector.begin(), theVector.end(), searchItem);
        if (removed == theVector.end())
            return false;
        theVector.erase(removed, theVector.end());

        return true;
    }

    // Remove item from map
    template<class KEY, class VALUE>
    bool Remove(std::unordered_map<KEY, VALUE>& map, KEY key)
    {
        auto i = map.find(key);
        if (i != map.end())
        {
            map.erase(i);
            return true;
        }
        else
            return false;
    }

    // Remove items from list
    template<class ENTRY>
    int Remove(std::list<ENTRY>& theList, const ENTRY searchItem)
    {
        int numRemoved = 0;

        auto pos = std::find(theList.begin(), theList.end(), searchItem);
        while (pos != theList.end())
        {
            theList.erase(pos);
            numRemoved++;
            pos = std::find(theList.begin(), theList.end(), searchItem);
        }

        return numRemoved;
    }

} // SwUtils

