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
 * @file ObsEpochMap.hpp
 * A class encapsulating observation data
 */

#ifndef GPSTK_OBSEPOCHMAP_HPP
#define GPSTK_OBSEPOCHMAP_HPP

#include <map>
#include <iostream>

#include "CommonTime.hpp"
#include "SatID.hpp"
#include "ObsID.hpp"
#include "ValidType.hpp"

namespace gpstk
{
      /// @ingroup ClockModel
      //@{

      /// All the observations collected from a single SV at a single epoch
   struct SvObsEpoch : public std::map<ObsID, double>
   {
      gpstk::SatID svid;
      vfloat azimuth, elevation;
   };

      /** All the observations collected from a single receiver at a
       * single epoch */
   struct ObsEpoch : public std::map<SatID, SvObsEpoch>
   {
      gpstk::CommonTime time;
      vdouble rxClock;
   };

      /// A time history of the observations collected from a single receiver.
   typedef std::map<CommonTime, ObsEpoch> ObsEpochMap;

   std::ostream& operator<<(std::ostream& s, const SvObsEpoch& obs) throw();
   std::ostream& operator<<(std::ostream& s, const ObsEpoch& oe) throw();

      //@}

} // namespace 

#endif 
