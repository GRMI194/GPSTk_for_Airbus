#pragma ident "$Id: TimeSystem.cpp 1649 2009-01-27 23:53:20Z raindave $"


//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//  Copyright 2004, The University of Texas at Austin
//
//============================================================================

//============================================================================
//
//This software developed by Applied Research Laboratories at the University of
//Texas at Austin, under contract to an agency or agencies within the U.S. 
//Department of Defense. The U.S. Government retains all rights to use,
//duplicate, distribute, disclose, or release this software. 
//
//Pursuant to DoD Directive 523024 
//
// DISTRIBUTION STATEMENT A: This software has been approved for public 
//                           release, distribution is unlimited.
//
//=============================================================================

#include "TimeSystem.hpp"

namespace Rinex3
{
   void TimeSys::setTimeSystem( const Systems& sys )
      throw()
   {         
      if (sys < 0 || sys > 3)
      {
         ts = Unknown;
      }
      else
      {
         ts = sys;
      }
   }

   std::string TimeSys::asString() const
      throw()
   {
      switch(ts)
      {
         case Unknown:
            return("[Unknown]");
            break;
         case Any:
            return("[Any (wildcard)]");
            break;
         case GPS:
            return("[GPS]");
            break;
         case UTC:
            return("[UTC]");
            break;
         default:
            return("Error: timesystem set to unknown value: " + ts);
            break;
      }
   }

} //namespace