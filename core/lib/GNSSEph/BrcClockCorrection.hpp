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
 * @file BrcClockCorrection.hpp
 * GNSS Broadcast clock correction data encapsulated in engineering terms
 * BrcClockCorrection is designed to address all the navigation message
 * formats that are based on a second-order clock model.
 */

#ifndef GPSTK_BRCLOCKCORRECTION_HPP
#define GPSTK_BRCLOCKCORRECTION_HPP

#include "EngNav.hpp"
#include "Exception.hpp"
#include "CommonTime.hpp"
#include "ObsID.hpp"
#include "CivilTime.hpp"
#include "GNSSconstants.hpp"
#include "TimeSystem.hpp"
#include "GPSWeekSecond.hpp"
#include "YDSTime.hpp"

class BrcClockCorrection_T;

namespace gpstk
{
      /// @ingroup GNSSEph
      //@{

      /**
       * Clock correction information for a single satellite.  This class
       * encapsulates the clock correction parameters in any of several
       * navigation message formats (basically those that use
       * Keplerian elements for the orbits), provides functions to decode the
       * as-broadcast bit-encodings, and generate SV clock corrections
       * as a function of time.
       *
       *Note: Relativistic correction is NOT included, see BrcKeplerOrbit.
       */
   class BrcClockCorrection : public EngNav
   {
   public:
         /// Default constructor
      BrcClockCorrection() throw();

	 /// General purpose constructor
      BrcClockCorrection( const std::string satSysArg, const ObsID obsIDArg,
                          const short PRNIDArg, const CommonTime TocArg,
                          const CommonTime TopArg, const short URAocArg,
                          const short URAoc1Arg, const short URAoc2Arg,
                          const bool healthyArg, const double af0Arg,
                          const double af1Arg, const double af2Arg );

         /// Legacy GPS Subframe 1
      BrcClockCorrection( const ObsID obsIDArg, const short PRNID,
                          const short fullweeknum, const long subframe1[10] );

      	 // Add other constructors for other navigation message formats here...

         /// Destructor
      virtual ~BrcClockCorrection() {}

      bool operator==(const BrcClockCorrection& right) const throw();
      bool operator!=(const BrcClockCorrection& right) const throw()
      { return !operator==(right); }

         /**
          * Query presence of data in this object.
          * @return true if data has been loaded in this object.
          */
      bool hasData() const;

         /**
          * Returns the epoch time (time of clock) from this
          * ephemeris, correcting for half weeks and HOW time.
          * @throw InvalidRequest
          */
      CommonTime getEpochTime() const;

         /** This function returns the PRN ID of the SV.
          * @throw InvalidRequest
          */
      short getPRNID() const;

         /** This function returns the OBS ID of the clock correction.
          * @throw InvalidRequest
          */
      ObsID getObsID() const;

         /**
          * This function returns the value of the SV accuracy (m)
          * computed from the accuracy flag in the nav message.
          * @throw InvalidRequest
          */
      double getAccuracy(const CommonTime& t) const;

         /**
          * @throw InvalidRequest
          */
      short getURAoc(const short& ndx) const;

         /** Returns SV health status. */
         //NB Determine if this function is needed, as it is never used
         //bool isHealthy() const;

         /**
          * This function return the GPS week number for the
          * orbit. This is the full GPS week (ie > 10 bits).
          * @throw InvalidRequest
          */
      short getFullWeek() const;

         /**
          * This function returns the clock epoch in GPS seconds of
          * week.
          * @throw InvalidRequest
          */
      double getToc() const;

         /** This function returns the SV clock error in seconds.
          * @throw InvalidRequest
          */
      double getAf0() const;

         /**
          * This function returns the SV clock drift in
          * seconds/seconds.
          * @throw InvalidRequest
          */
      double getAf1() const;

         /**
          * This function returns the SV clock rate of change of the
          * drift in seconds/(seconds*seconds).
          * @throw InvalidRequest */
      double getAf2() const;

         /** Compute the satellite clock bias (sec) at the given time
          * @throw InvalidRequest if a required subframe has not been stored.
          */
      double svClockBias(const CommonTime& t) const;

         /** Compute the satellite clock bias (meters) at the given time
          * @throw InvalidRequest if a required subframe has not been stored.
          */
      double svClockBiasM(const CommonTime& t) const;

         /** Compute the satellite clock drift (sec/sec) at the given time
          * @throw InvalidRequest if a required subframe has not been stored.
          */
      double svClockDrift(const CommonTime& t) const;

         /** General purpose means to load data into object. */
      void loadData( const std::string satSysArg, const ObsID obsIDArg,
                     const short PRNIDArg, const CommonTime TocArg,
                     const CommonTime TopArg, const short URAocArg,
                     const short URAoc1Arg, const short URAoc2Arg,
                     const bool healthyArg, const double af0Arg,
                     const double af1Arg, const double af2Arg );

         /** Load data based on the GPS Legacy message. */
      void loadData(const std::string satSysArg, const ObsID obsIDArg,
                    const short PRNIDArg, const CommonTime TocArg,
                    const short URAocArg, const bool healthyArg,
                    const double af0Arg, const double af1Arg,
                    const double af2Arg );

         /** Load data based on the GPS Legacy message.
          * @throw InvalidParameter
          */
      void loadData( const ObsID obsIDArg, const short PRNID,
                     const short fullweeknum, const long subframe1[10] );

         /** Output the contents of this ephemeris to the given stream. */
      void dump(std::ostream& s = std::cout) const;

   protected:
         /// @name Overhead information
         //@{
      bool dataLoaded;	     /**< True if data is present, false otherwise */
      std::string  satSys;   /**< Rinex satellite system ID */
      ObsID obsID;           /**< Defines carrier and tracking code */
      short PRNID;           /**< SV PRN ID */
      CommonTime Toc;
      CommonTime Top;
      short URAoc;
      short URAoc1;
      short URAoc2;
      bool healthy;          /**< SV health */

         //@}

         /// @name Clock information
         //@{
      double af0;           /**< SV clock error (sec) */
      double af1;           /**< SV clock drift (sec/sec) */
      double af2;           /**< SV clock drift rate (sec/sec**2) */
         //@}

      friend class ::BrcClockCorrection_T;
      friend std::ostream& operator<<(std::ostream& s,
                                      const BrcClockCorrection& eph);

   }; // class BrcClockCorrection

      //@}

} // namespace

#endif
