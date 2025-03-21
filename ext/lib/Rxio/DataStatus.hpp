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
 * @file DataStatus.hpp
 * gpstk::DataStatus - This class is intended to provide an interface 
   similiar to the iostream status bits (good, eof, bad, & fail) for
   the data.  This is to support 'soft' errors such as a format or crc error. 
 */

#ifndef DATASTATUS_HPP
#define DATASTATUS_HPP

#include <iostream>

namespace gpstk
{
   class DataStatus
   {
      std::ios_base::iostate state;

   public:

      DataStatus() : state(std::ios_base::goodbit) {};

      /// Note that the goodbit isn't really a bit, it is just the lack
      /// of any bits being set.
      bool good() const 
      {return rdstate() == std::ios_base::goodbit;}

      std::ios_base::iostate rdstate() const
      {return state;}

      void clear(std::ios_base::iostate s = std::ios_base::goodbit)
      {state=s;}

      void setstate(std::ios_base::iostate s)
      {clear(rdstate() | s);}

      void clearstate(std::ios_base::iostate s)
      {state &= ~s;}

      bool operator()() const
      {return good();}

      operator void*() const 
      {return good() ? const_cast<DataStatus*>(this) : 0;}
   };

   /// Define additional/other bits are the data requires
   static const std::ios_base::iostate crcbit = std::ios_base::iostate(0x01);
   static const std::ios_base::iostate fmtbit = std::ios_base::iostate(0x02);
   static const std::ios_base::iostate lenbit = std::ios_base::iostate(0x04);
   static const std::ios_base::iostate parbit = std::ios_base::iostate(0x08);

   class CRCDataStatus : public DataStatus
   {
   public:
      bool crcerr() const {return (rdstate() & crcbit) != 0;}
      bool fmterr() const {return (rdstate() & fmtbit) != 0;}
      bool lenerr() const {return (rdstate() & lenbit) != 0;}
      bool parerr() const {return (rdstate() & parbit) != 0;}
   };
} // namespace gpstk

#endif // DATASTATUS_HPP
