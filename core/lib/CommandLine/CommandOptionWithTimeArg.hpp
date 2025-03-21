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
 * @file CommandOptionWithTimeArg.hpp
 * Command line options with time (class CommonTime) arguments
 */

#ifndef COMMANDOPTIONWITHTIMEARG_HPP
#define COMMANDOPTIONWITHTIMEARG_HPP

#include "CommandOption.hpp"
#include "CommonTime.hpp"

namespace gpstk
{
      /// @ingroup CommandLine
      //@{

      /** Command-line option class for processing time strings.
       * This class is allows the programmer to add command-line
       * options to an application that can parse strings containing
       * representations of time. The programmer must specify the
       * format to be accepted.  Refer to CommonTime::printf() for
       * details on the formatting specifications. */
   class CommandOptionWithTimeArg : public gpstk::CommandOptionWithAnyArg
   {
   public:
         /** Constructor
          * @param shOpt The one character command line option.  Set to 0
          *    if unused.
          * @param loOpt The long command option.  Set to std::string() 
          *    if unused.
          * @param timeFormat format for scanning argument into a CommonTime
          *    (\see CommonTime::setToString() for details).
          * @param desc A string describing what this option does.
          * @param required Set to true if this is a required option.
          */
      CommandOptionWithTimeArg(const char shOpt,
                               const std::string& loOpt,
                               const std::string& timeFormat,
                               const std::string& desc,
                               const bool required = false)
            : gpstk::CommandOptionWithAnyArg(shOpt, loOpt, desc, required),
              timeSpec(timeFormat)
      {}

         /// Destructor
      virtual ~CommandOptionWithTimeArg() {}

         /** Returns a string with the argument format (just "TIME",
          * not scanning format). */
      virtual std::string getArgString() const
      { return "TIME"; }

         /// Validate arguments passed using this option (and store them).
      virtual std::string checkArguments();

         /// Return the times scanned in from the command line.
      std::vector<CommonTime> getTime() const { return times; }

   protected:
         /// Collection of times scanned in from the command line.
      std::vector<CommonTime> times;
         /// Format used to scan times in.
      std::string timeSpec;

         /// Default Constructor
      CommandOptionWithTimeArg() {}

         /// Return the appropriate time scanning format for value[index].
      virtual std::string getTimeSpec(std::vector<std::string>::size_type index) const
      { return timeSpec; }
   }; // class CommandOptionWithTimeArg


      /** This class is similar to CommandOptionWithTimeArg, but
       * accepts several different time formats simultaneously.
       * The user can use any of the following time formats with this
       * option:
       * - month/day/year
       * - year day-of-year
       * - year day-of-year seconds-of-day
       */
   class CommandOptionWithSimpleTimeArg : public CommandOptionWithTimeArg
   {
   public:
         /** Constructor
          * @param shOpt The one character command line option.  Set to 0
          *    if unused.
          * @param loOpt The long command option.  Set to std::string() 
          *    if unused.
          * @param desc A string describing what this option does.
          * @param required Set to true if this is a required option.
          */
      CommandOptionWithSimpleTimeArg(const char shOpt,
                               const std::string& loOpt,
                               const std::string& desc,
                               const bool required = false)
            : CommandOptionWithTimeArg(shOpt, loOpt, "", desc, required)
      {}

         /// Destructor
      virtual ~CommandOptionWithSimpleTimeArg() {}

   protected:
         /// Default Constructor
      CommandOptionWithSimpleTimeArg() {}

         /// Return the appropriate time scanning format for value[index].
      virtual std::string getTimeSpec(std::vector<std::string>::size_type index) const;
   }; // class CommandOptionWithSimpleTimeArg

      //@}

} // namespace gpstk

#endif
