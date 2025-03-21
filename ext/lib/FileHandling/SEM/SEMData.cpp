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
 * @file SEMData.cpp
 * Encapsulate SEM almanac file data, including I/O
 */

#include "StringUtils.hpp"
#include "GNSSconstants.hpp"

#include "SEMData.hpp"
#include "SEMStream.hpp"


using namespace gpstk::StringUtils;
using namespace std;

namespace gpstk
{
   SEMData::SEMData()
         : ecc(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           i_offset(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           OMEGAdot(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           Ahalf(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           OMEGA0(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           w(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           M0(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           AF0(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace),
           AF1(0, FFLead::Zero, 15, 4, 0, 'E', FFSign::NegSpace)
   {
   }


   void SEMData::reallyPutRecord(FFStream& ffs) const
   {
      string line;

      SEMStream& strm = dynamic_cast<SEMStream&>(ffs);

      //First output blank line to mark between records
      strm << std::endl;

      //PRN output
      strm << asString<short>(PRN) << endl;

      //SVNnum
      strm << asString<short>(SVNnum) << endl;

      //URAnum
      strm << asString<short>(URAnum) << endl;

      //Ecc, i_offset, OMEGAdot
      strm << ecc << " " << (i_offset / gpstk::PI) << " "
           << (OMEGAdot / gpstk::PI) << endl;

      //Ahalf, OMEGA0, w
      strm << Ahalf << " " << (OMEGA0/gpstk::PI) << " " << (w/gpstk::PI)
           << endl;

      //M0, AF0, AF1
      strm << (M0/gpstk::PI) << " " << AF0 << " " << AF1 << endl;

      //SV_health
      strm << asString<short>(SV_health) << endl;

      //satConfig
      strm << asString<short>(satConfig) << endl;


   }   // end SEMData::reallyPutRecord


   void SEMData::reallyGetRecord(FFStream& ffs)
   {
      string line;

      SEMStream& strm = dynamic_cast<SEMStream&>(ffs);

      //if(!strm.headerRead)
      //    strm >> strm.header;

      SEMHeader& hdr = strm.header;

      //Don't need first line - empty space
      strm.formattedGetLine(line, true);

      // Second line - PRN
      strm.formattedGetLine(line, true);
      PRN = asInt(line);

      // Third line - SVN Number
      // HACKHACKHACK This information might not be here??? Find out more info
      strm.formattedGetLine(line, true);
      SVNnum = (short) asInt(line);

      // Fourth line - Average URA Number as defined in ICD-GPS-200
      strm.formattedGetLine(line, true);
      URAnum = (short) asInt(line);

      string whitespace = " \t\r\n";

      // Fifth line - Eccentricity, Inclination Offset, and Rate of Right Ascension
      strm.formattedGetLine(line, true);
      string::size_type front = line.find_first_not_of(whitespace);
      string::size_type end = line.find_first_of(whitespace,front);
      string::size_type length = end - front;
      ecc = line.substr(front,length);

      front = line.find_first_not_of(whitespace,end);
      end = line.find_first_of(whitespace,front);
      length = end - front;
      i_offset = line.substr(front,length);
      i_total = i_offset + 54.0 * (gpstk::PI / 180.0);

      front = line.find_first_not_of(whitespace,end);
      length = line.length() - front;
      OMEGAdot = line.substr(front,length);
      i_offset *= gpstk::PI;
      OMEGAdot *= gpstk::PI;

      // Sixth line - Sqrt of A, Omega0, and Arg of Perigee
      strm.formattedGetLine(line, true);

      front = line.find_first_not_of(whitespace);
      end = line.find_first_of(whitespace,front);
      length = end - front;
      Ahalf = line.substr(front,length);

      front = line.find_first_not_of(whitespace,end);
      end = line.find_first_of(whitespace,front);
      length = end - front;
      OMEGA0 = line.substr(front,length);

      front = line.find_first_not_of(whitespace,end);
      length = line.length() - front;
      OMEGA0 *= gpstk::PI;
      w = line.substr(front,length);
      w *= gpstk::PI;

      // Seventh Line - M0, AF0, AF1
      strm.formattedGetLine(line, true);

      front = line.find_first_not_of(whitespace);
      end = line.find_first_of(whitespace,front);
      length = end - front;
      M0 = line.substr(front,length);
      M0 *= gpstk::PI;

      front = line.find_first_not_of(whitespace,end);
      end = line.find_first_of(whitespace,front);
      length = end - front;
      AF0 = line.substr(front,length);

      front = line.find_first_not_of(whitespace,end);
      length = line.length() - front;
      AF1 = line.substr(front,length);

      // Eigth line - Satellite Health
      strm.formattedGetLine(line, true);
      SV_health = (short) asInt(line);

      // Ninth line - Satellite Config
      strm.formattedGetLine(line, true);
      satConfig = (short) asInt(line);

      week = hdr.week;
      Toa = hdr.Toa;

      xmit_time = 0;

   } // end of reallyGetRecord()

   void SEMData::dump(ostream& s) const
   {
      s << "PRN = " << PRN << std::endl;
      s << "SVNnum = " << SVNnum << std::endl;
      s << "URAnum = " << URAnum << std::endl;
      s << "ecc = " << ecc << std::endl;
      s << "i_offset = " << i_offset << std::endl;
      s << "OMEGAdot = " << OMEGAdot << std::endl;
      s << "Ahalf = " << Ahalf << std::endl;
      s << "OMEGA0 = " << OMEGA0 << std::endl;
      s << "w = " << w << std::endl;
      s << "M0 = " << M0 << std::endl;
      s << "AF0 = " << AF0 << std::endl;
      s << "AF1 = " << AF1 << std::endl;
      s << "SV_health = " << SV_health << std::endl;
      s << "satConfig = " << satConfig << std::endl;
      s << "xmit_time = " << xmit_time << std::endl;
      s << "week = " << week << std::endl;
      s << "toa = " << Toa << std::endl;
   }

   SEMData::operator AlmOrbit() const
   {

      AlmOrbit ao(PRN, ecc,i_offset, OMEGAdot, Ahalf, OMEGA0,
                   w, M0, AF0, AF1, Toa, xmit_time, week, SV_health);

      return ao;
   }

   SEMData::operator OrbAlmGen() const
   {
     OrbAlmGen oag;

     oag.AHalf    = Ahalf; 
     oag.A        = Ahalf * Ahalf; 
     oag.af1      = AF1;
     oag.af0      = AF0;
     oag.OMEGA0   = OMEGA0; 
     oag.ecc      = ecc;
     oag.deltai   = i_offset;
     oag.i0       = i_total;
     oag.OMEGAdot = OMEGAdot;
     oag.w        = w;
     oag.M0       = M0;
     oag.toa      = Toa;
     oag.health   = SV_health; 
     
     // At this writing Yuma almanacs only exist for GPS
     oag.subjectSV = SatID(PRN, SatelliteSystem::GPS); 

     // Unfortunately, we've NO IDEA which SV transmitted 
     // these data.
     oag.satID = SatID(0,SatelliteSystem::GPS); 

     // 
     oag.ctToe = GPSWeekSecond(week,Toa,TimeSystem::GPS);

     // There is no transmit time in the SEM alamanc format.  
     // Therefore, beginValid and endvalid are estimated.  The
     // estimate is based on IS-GPS-200 Table 20-XIII.  
     oag.beginValid = oag.ctToe - (70 * 3600.0);
     oag.endValid   = oag.beginValid + (144 * 3600.0);

     oag.dataLoadedFlag = true; 
     oag.setHealthy(false);
     if (oag.health==0) 
        oag.setHealthy(true);

        // It is assumed that the data were broadcast on
        // each of L1 C/A, L1 P(Y), and L2 P(Y).   We'll
        // load obsID with L1 C/A for the sake of completeness,
        // but this will probably never be examined.
     oag.obsID = ObsID(ObservationType::NavMsg,CarrierBand::L1,TrackingCode::CA);

     return oag;       
   }


} // namespace
