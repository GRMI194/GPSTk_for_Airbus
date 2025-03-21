//==============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin.
//  Copyright 2004-2020, The Board of Regents of The University of Texas System
//
//==============================================================================

//==============================================================================
//
//  This software was developed by Applied Research Laboratories at the
//  University of Texas at Austin, under contract to an agency or agencies
//  within the U.S. Department of Defense. The U.S. Government retains all
//  rights to use, duplicate, distribute, disclose, or release this software.
//
//  Pursuant to DoD Directive 523024 
//
//  DISTRIBUTION STATEMENT A: This software has been approved for public 
//                            release, distribution is unlimited.
//
//==============================================================================

/**
 * @file Combinations.hpp
 * Find all the combinations of n things taken k at a time.
 */
 
#ifndef COMBINATIONS_INCLUDE
#define COMBINATIONS_INCLUDE

namespace gpstk {

#include "Exception.hpp"

/// Class Combinations will compute C(n,k), all the combinations of n things
/// taken k at a time (where k <= n).
/// Let n 'things' be indexed by i (i=0...n-1), e.g. stored in an array of length n.
/// This class computes C(n,k) as sets of k indexes into the 'things' array.
/// These indexes are accessible via member functions Selection() or isSelected().
/// Next() computes the next combination until there are no more (when it returns -1).
class Combinations {
public:
   /// Default constructor
   Combinations(void) throw()
   {
      nc = n = k = 0;
      Index = std::vector<int>(0);
   }

   /// Constructor for C(n,k) = combinations of n things taken k at a time (k <= n)
   /// @throw Exception on invalid input (k>n).
   Combinations(int N, int K)
   { init(N,K); }

   /// copy constructor
   Combinations(const Combinations& right)
      throw()
   {
      *this = right;
   }

   /// Assignment operator.
   Combinations& operator=(const Combinations& right) throw()
   {
      init(right.n,right.k);
      nc = right.nc;
      for(int j=0; j<k; j++) Index[j] = right.Index[j];
      return *this;
   }

   /// Compute the next combination, returning the number of combinations computed
   /// so far; if there are no more combinations, return -1.
   int Next(void) throw() {
      if(k < 1) return -1;
      if(Increment(k-1) != -1) return ++nc;
      return -1;
   }

   /// Return index i (0 <= i < n) of jth selection (0 <= j < k);
   /// if j is out of range, return -1.
   int Selection(int j) throw()
   {
      if(j < 0 || j >= k) return -1;
      return Index[j];
   }

      /// Return true if the given index j (0 <= j < n) is
      /// currently selected (i.e. if j = Selection(i) for some i)
   bool isSelected(int j) throw()
   {
      for(int i=0; i<k; i++)
         if(Index[i] == j) return true;
      return false;
   }

private:

   /// The initialization routine used by constructors.
   /// @throw Exception on invalid input (k>n or either n or k < 0).
   void init(int N, int K)
   {
      if(K > N || N < 0 || K < 0) {
         Exception e("Combinations(n,k) must have k <= n, with n,k >= 0");
         GPSTK_THROW(e);
      }

      Index = std::vector<int>(K);
      nc = 0;
      k = K;
      n = N;
      for(int j=0; j<k; j++)
         Index[j] = j;
   }

   /// Recursive function to increment Index[j].
   int Increment(int j) throw()
   {
      if(Index[j] < n-k+j) {        // can this index be incremented?
         Index[j]++;
         for(int m=j+1; m<k; m++)
            Index[m]=Index[m-1]+1;
         return 0;
      }
         // is this the last index?
      if(j-1 < 0) return -1;
         // increment the next lower index
      return Increment(j-1);
   }


   int nc;                  ///< number of combinations computed so far
   int k,n;                 ///< combinations of n things taken k at a time
   std::vector<int> Index;  ///< Index[j] = index of jth selection
                            ///<         (j=0...k-1; I[j]=0...n-1)

}; // end class Combinations

} // end namespace gpstk

#endif // COMBINATIONS_INCLUDE
