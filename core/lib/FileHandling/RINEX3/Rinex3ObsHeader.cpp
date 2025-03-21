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
 * @file Rinex3ObsHeader.cpp
 * Encapsulate header of Rinex observation file, including I/O
 */

#include <sstream>
#include <algorithm>
#include <set>
#include <string.h>

#include "StringUtils.hpp"
#include "SystemTime.hpp"
#include "TimeString.hpp"
#include "Rinex3ObsStream.hpp"
#include "Rinex3ObsHeader.hpp"

using namespace std;
using namespace gpstk::StringUtils;

namespace gpstk
{
   const string Rinex3ObsHeader::hsVersion           = "RINEX VERSION / TYPE";
   const string Rinex3ObsHeader::hsRunBy             = "PGM / RUN BY / DATE";
   const string Rinex3ObsHeader::hsComment           = "COMMENT";
   const string Rinex3ObsHeader::hsMarkerName        = "MARKER NAME";
   const string Rinex3ObsHeader::hsMarkerNumber      = "MARKER NUMBER";
   const string Rinex3ObsHeader::hsMarkerType        = "MARKER TYPE";
   const string Rinex3ObsHeader::hsObserver          = "OBSERVER / AGENCY";
   const string Rinex3ObsHeader::hsReceiver          = "REC # / TYPE / VERS";
   const string Rinex3ObsHeader::hsAntennaType       = "ANT # / TYPE";
   const string Rinex3ObsHeader::hsAntennaPosition   = "APPROX POSITION XYZ";
   const string Rinex3ObsHeader::hsAntennaDeltaHEN   = "ANTENNA: DELTA H/E/N";
   const string Rinex3ObsHeader::hsAntennaDeltaXYZ   = "ANTENNA: DELTA X/Y/Z";
   const string Rinex3ObsHeader::hsAntennaPhaseCtr   = "ANTENNA: PHASECENTER";
   const string Rinex3ObsHeader::hsAntennaBsightXYZ  = "ANTENNA: B.SIGHT XYZ";
   const string Rinex3ObsHeader::hsAntennaZeroDirAzi = "ANTENNA: ZERODIR AZI";
   const string Rinex3ObsHeader::hsAntennaZeroDirXYZ = "ANTENNA: ZERODIR XYZ";
   const string Rinex3ObsHeader::hsCenterOfMass      = "CENTER OF MASS: XYZ";
   const string Rinex3ObsHeader::hsNumObs            = "# / TYPES OF OBSERV";
   const string Rinex3ObsHeader::hsSystemNumObs      = "SYS / # / OBS TYPES";
   const string Rinex3ObsHeader::hsWaveFact          = "WAVELENGTH FACT L1/2";
   const string Rinex3ObsHeader::hsSigStrengthUnit   = "SIGNAL STRENGTH UNIT";
   const string Rinex3ObsHeader::hsInterval          = "INTERVAL";
   const string Rinex3ObsHeader::hsFirstTime         = "TIME OF FIRST OBS";
   const string Rinex3ObsHeader::hsLastTime          = "TIME OF LAST OBS";
   const string Rinex3ObsHeader::hsReceiverOffset    = "RCV CLOCK OFFS APPL";
   const string Rinex3ObsHeader::hsSystemDCBSapplied = "SYS / DCBS APPLIED";
   const string Rinex3ObsHeader::hsSystemPCVSapplied = "SYS / PCVS APPLIED";
   const string Rinex3ObsHeader::hsSystemScaleFac    = "SYS / SCALE FACTOR";
   const string Rinex3ObsHeader::hsSystemPhaseShift  = "SYS / PHASE SHIFT";
   const string Rinex3ObsHeader::hsGlonassSlotFreqNo = "GLONASS SLOT / FRQ #";
   const string Rinex3ObsHeader::hsGlonassCodPhsBias = "GLONASS COD/PHS/BIS";
   const string Rinex3ObsHeader::hsLeapSeconds       = "LEAP SECONDS";
   const string Rinex3ObsHeader::hsNumSats           = "# OF SATELLITES";
   const string Rinex3ObsHeader::hsPrnObs            = "PRN / # OF OBS";
   const string Rinex3ObsHeader::hsEoH               = "END OF HEADER";

   int Rinex3ObsHeader::debug = 0;

  const Rinex3ObsHeader::Fields Rinex3ObsHeader::allValid2({
         Rinex3ObsHeader::validVersion,
            Rinex3ObsHeader::validRunBy,
            Rinex3ObsHeader::validMarkerName,
            Rinex3ObsHeader::validObserver,
            Rinex3ObsHeader::validReceiver,
            Rinex3ObsHeader::validAntennaType,
            Rinex3ObsHeader::validAntennaPosition,
            Rinex3ObsHeader::validAntennaDeltaHEN,
            Rinex3ObsHeader::validNumObs,
            Rinex3ObsHeader::validFirstTime
            });
   const Rinex3ObsHeader::Fields Rinex3ObsHeader::allValid30({
         Rinex3ObsHeader::validVersion,
            Rinex3ObsHeader::validRunBy,
            Rinex3ObsHeader::validMarkerName,
               /** @note MARKER TYPE is actually required but in the
                * interest of supporting the invalid IGS data files,
                * we make it optional */
               //Rinex3ObsHeader::validMarkerType,
            Rinex3ObsHeader::validObserver,
            Rinex3ObsHeader::validReceiver,
            Rinex3ObsHeader::validAntennaType,
            Rinex3ObsHeader::validAntennaPosition,
            Rinex3ObsHeader::validAntennaDeltaHEN,
            Rinex3ObsHeader::validSystemNumObs,
            Rinex3ObsHeader::validFirstTime
            });
   const Rinex3ObsHeader::Fields Rinex3ObsHeader::allValid301({
         Rinex3ObsHeader::validVersion,
            Rinex3ObsHeader::validRunBy,
            Rinex3ObsHeader::validMarkerName,
               //Rinex3ObsHeader::validMarkerType,
            Rinex3ObsHeader::validObserver,
            Rinex3ObsHeader::validReceiver,
            Rinex3ObsHeader::validAntennaType,
            Rinex3ObsHeader::validAntennaPosition,
            Rinex3ObsHeader::validAntennaDeltaHEN,
            Rinex3ObsHeader::validSystemNumObs,
            Rinex3ObsHeader::validFirstTime,
            Rinex3ObsHeader::validSystemPhaseShift
            });
   const Rinex3ObsHeader::Fields Rinex3ObsHeader::allValid302({
         Rinex3ObsHeader::validVersion,
            Rinex3ObsHeader::validRunBy,
            Rinex3ObsHeader::validMarkerName,
               //Rinex3ObsHeader::validMarkerType,
            Rinex3ObsHeader::validObserver,
            Rinex3ObsHeader::validReceiver,
            Rinex3ObsHeader::validAntennaType,
            Rinex3ObsHeader::validAntennaPosition,
            Rinex3ObsHeader::validAntennaDeltaHEN,
            Rinex3ObsHeader::validSystemNumObs,
            Rinex3ObsHeader::validFirstTime,
            Rinex3ObsHeader::validSystemPhaseShift
            });
   const Rinex3ObsHeader::Fields Rinex3ObsHeader::allValid303({
         Rinex3ObsHeader::validVersion,
            Rinex3ObsHeader::validRunBy,
            Rinex3ObsHeader::validMarkerName,
               //Rinex3ObsHeader::validMarkerType,
            Rinex3ObsHeader::validObserver,
            Rinex3ObsHeader::validReceiver,
            Rinex3ObsHeader::validAntennaType,
            Rinex3ObsHeader::validAntennaPosition,
            Rinex3ObsHeader::validAntennaDeltaHEN,
            Rinex3ObsHeader::validSystemNumObs,
            Rinex3ObsHeader::validFirstTime,
            Rinex3ObsHeader::validSystemPhaseShift
            });

   Rinex3ObsHeader::Rinex3ObsHeader()
         : PisY(false)
   {
      clear();

      // Set Sqm Variable
      _bIsPossiblySqm = false;
   }

   Rinex3ObsHeader::Rinex3ObsHeader(bool bIsPossiblySqm)
   {
       clear();

       // Set Sqm Variable
       _bIsPossiblySqm = bIsPossiblySqm;
   }


   void Rinex3ObsHeader::clear()
   {
      R2ObsTypes.clear();
      mapSysR2toR3ObsID.clear();
      version = currentVersion;
      fileType = "O";          // observation data
      fileSys = "G";           // GPS only by default
      preserveVerType = false; // let the write methods chose the above
      fileSysSat = SatID(-1,SatelliteSystem::GPS);
      fileProgram.clear();
      fileAgency.clear();
      date.clear();
      preserveDate = false;
      commentList.clear();
      markerName.clear();
      markerNumber.clear();
      markerType.clear();
      observer.clear();
      agency.clear();
      recNo.clear();
      recType.clear();
      recVers.clear();
      antNo.clear();
      antType.clear();
      antennaPosition = Triple();
      antennaDeltaHEN = Triple();
      antennaDeltaXYZ = Triple();
      antennaSatSys.clear();
      antennaObsCode.clear();
      antennaPhaseCtr = Triple();
      antennaBsightXYZ = Triple();
      antennaZeroDirAzi = 0.;
      antennaZeroDirXYZ = Triple();
      centerOfMass = Triple();
      mapObsTypes.clear();
      wavelengthFactor[0] = wavelengthFactor[1] = 1;
      extraWaveFactList.clear();
      sigStrengthUnit.clear();
      interval = 0.;
      firstObs = CivilTime();
      lastObs = CivilTime();
      receiverOffset = 0;
      infoDCBS.clear();
      infoPCVS.clear();
      sysSfacMap.clear();
      sysPhaseShift.clear();
      glonassFreqNo.clear();
      glonassCodPhsBias.clear();
      leapSeconds = 0;
      numSVs = 0;
      numObsForSat.clear();
      obsTypeList.clear();
      valid.clear();
      validEoH = false;
         // Only do this in the constructor so the desired handling of
         // "P" code in RINEX 2 stays the same.
         //PisY = false;
      sysPhaseShiftObsID = RinexObsID();
      satSysTemp.clear();
      satSysPrev.clear();
      numObs = 0;
      numObsPrev = 0;
      lastPRN = RinexSatID();
      factor = 0;
      factorPrev = 0;
   }


   void Rinex3ObsHeader::reallyPutRecord(FFStream& ffs) const
   {
      using gpstk::StringUtils::asString;
      Rinex3ObsStream& strm = dynamic_cast<Rinex3ObsStream&>(ffs);

      strm.header = *this;

      Fields allValid = Fields::getRequired(version);
      if (allValid.empty())
      {
         FFStreamError err("Unknown RINEX version: " +
                           gpstk::StringUtils::asString(version,2));
         err.addText("Make sure to set the version correctly.");
         GPSTK_THROW(err);
      }

      if((valid & allValid) != allValid)
      {
         FFStreamError err("Incomplete or invalid header.");
         err.addText("Make sure you set all header valid bits for all of the"
                     " available data.");
         allValid.describeMissing(valid, err);
         GPSTK_THROW(err);
      }

      try
      {
         writeHeaderRecords(strm);
      }
      catch(FFStreamError& e)
      {
         GPSTK_RETHROW(e);
      }
      catch(StringException& e)
      {
         GPSTK_RETHROW(e);
      }

   }  // end reallyPutRecord


      // This function computes the number of valid header records
      // which writeHeaderRecords will write.
      // NB not used in Rinex3Obs....
   int Rinex3ObsHeader::numberHeaderRecordsToBeWritten(void) const throw()
   {
      int n = 0;

      if(valid & validVersion          ) n++;
      if(valid & validRunBy            ) n++;
      if(valid & validComment          ) n += commentList.size();
      if(valid & validMarkerName       ) n++;
      if(valid & validMarkerNumber     ) n++;
      if(version >= 3 && (valid & validMarkerType)) n++;
      if(valid & validObserver         ) n++;
      if(valid & validReceiver         ) n++;
      if(valid & validAntennaType      ) n++;
      if(valid & validAntennaPosition  ) n++;
      if(valid & validAntennaDeltaHEN  ) n++;
      if(version >= 3 && (valid & validAntennaDeltaXYZ)) n++;
      if(version >= 3 && (valid & validAntennaPhaseCtr)) n++;
      if(version >= 3 && (valid & validAntennaBsightXYZ)) n++;
      if(version >= 3 && (valid & validAntennaZeroDirAzi)) n++;
      if(version >= 3 && (valid & validAntennaZeroDirXYZ)) n++;
      if(version >= 3 && (valid & validCenterOfMass)) n++;
      if(version < 3 && (valid & validNumObs) && R2ObsTypes.size() != 0)
         n += 1 + (R2ObsTypes.size()-1)/9;
      if(version >= 3 && (valid & validSystemNumObs) && numObs != 0)
         n += 1 + (numObs-1)/9;
      if(version < 3 && (valid & validWaveFact))
      {
         n++;
         if(extraWaveFactList.size() != 0) n += extraWaveFactList.size();
      }
      if(version >= 3 && (valid & validSigStrengthUnit)) n++;
      if(valid & validInterval         ) n++;
      if(valid & validFirstTime        ) n++;
      if(valid & validLastTime         ) n++;
      if(valid & validReceiverOffset   ) n++;
      if(version >= 3 && (valid & validSystemDCBSapplied)) n++;
      if(version >= 3 && (valid & validSystemPCVSapplied)) n++;
      if(version >= 3 && (valid & validSystemScaleFac)) n++;
      if(version >= 3.01 && (valid & validSystemPhaseShift)) n++;        // one per system at least
      if(version >= 3.01 && (valid & validGlonassSlotFreqNo)) n++;  // TODO: continuation lines...
      if(version >= 3.02 && (valid & validGlonassCodPhsBias)) n++;
      if(valid & validLeapSeconds      ) n++;
      if(valid & validNumSats          ) n++;
      if(valid & validPrnObs           )
         n += numObsForSat.size() * (1+numObsForSat.begin()->second.size()/9);
      if(validEoH                      ) n++;

      return n;
   }  // end numberHeaderRecordsToBeWritten


      // This function writes all valid header records.
   void Rinex3ObsHeader::writeHeaderRecords(FFStream& ffs) const
   {
      using gpstk::StringUtils::asString;
      Rinex3ObsStream& strm = dynamic_cast<Rinex3ObsStream&>(ffs);
      string line;

      if(valid & validVersion)
      {
         line  = rightJustify(asString(version,2), 9);
         line += string(11, ' ');

         if((fileType[0] != 'O') && (fileType[0] != 'o'))
         {
            FFStreamError err("File type is not Observation: " + fileType);
            GPSTK_THROW(err);
         }

         if (preserveVerType)
         {
            line += leftJustify(fileType, 20);
            line += leftJustify(fileSys, 20);
         }
         else
         {
            if(fileSysSat.system == SatelliteSystem::Unknown)
            {
               FFStreamError err("Invalid satellite system");
               GPSTK_THROW(err);
            }

            line += leftJustify(string("OBSERVATION DATA"), 20);
            string str;
            if(fileSysSat.system == SatelliteSystem::Mixed)
               str = "MIXED";
            else
            {
               RinexSatID sat(fileSysSat);
               str = sat.systemChar();
               str = str + " (" + sat.systemString() + ")";
            }
            line += leftJustify(str, 20);
         }
         line += hsVersion;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validRunBy)
      {
         line  = leftJustify(fileProgram, 20);
         line += leftJustify(fileAgency , 20);
         if (preserveDate)
         {
            line += leftJustify(date, 20);
         }
         else
         {
            SystemTime sysTime;
            string curDate;
            curDate = printTime(sysTime,"%04Y%02m%02d %02H%02M%02S %P");
            line += leftJustify(curDate, 20);
         }
         line += hsRunBy;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validComment)
      {
         vector<string>::const_iterator itr = commentList.begin();
         while (itr != commentList.end())
         {
            line  = leftJustify((*itr), 60);
            line += hsComment;
            strm << line << endl;
            strm.lineNumber++;
            itr++;
         }
      }
      if(!R2DisambiguityMap.empty())
      {
         DisAmbMap::const_iterator itr = R2DisambiguityMap.begin();
         while (itr != R2DisambiguityMap.end())
         {
            line  = leftJustify(itr->first.substr(0,1) + " Obs Type " + itr->first.substr(1) + " originated from  " + itr->second, 60);
            line += hsComment;
            strm << line << endl;
            strm.lineNumber++;
            itr++;
         }
      }
      if(valid & validMarkerName)
      {
         line  = leftJustify(markerName, 60);
         line += hsMarkerName;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validMarkerNumber)
      {
         line  = leftJustify(markerNumber, 20);
         line += string(40, ' ');
         line += hsMarkerNumber;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validMarkerType))
      {
         line  = leftJustify(markerType, 20);
         line += string(40, ' ');
         line += hsMarkerType;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validObserver)
      {
         line  = leftJustify(observer, 20);
         line += leftJustify(agency  , 40);
         line += hsObserver;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validReceiver)
      {
         line  = leftJustify(recNo  , 20);
         line += leftJustify(recType, 20);
         line += leftJustify(recVers, 20);
         line += hsReceiver;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validAntennaType)
      {
         line  = leftJustify(antNo  , 20);
         line += leftJustify(antType, 20);
         line += string(20, ' ');
         line += hsAntennaType;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validAntennaPosition)
      {
         line  = rightJustify(asString(antennaPosition[0], 4), 14);
         line += rightJustify(asString(antennaPosition[1], 4), 14);
         line += rightJustify(asString(antennaPosition[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaPosition;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validAntennaDeltaHEN)
      {
         line  = rightJustify(asString(antennaDeltaHEN[0], 4), 14);
         line += rightJustify(asString(antennaDeltaHEN[1], 4), 14);
         line += rightJustify(asString(antennaDeltaHEN[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaDeltaHEN;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validAntennaDeltaXYZ))
      {
         line  = rightJustify(asString(antennaDeltaXYZ[0], 4), 14);
         line += rightJustify(asString(antennaDeltaXYZ[1], 4), 14);
         line += rightJustify(asString(antennaDeltaXYZ[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaDeltaXYZ;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validAntennaPhaseCtr))
      {
         line  =  leftJustify(antennaSatSys , 1);
         line += string(1, ' ');
         line += rightJustify(antennaObsCode, 3);
         line += rightJustify(asString(antennaPhaseCtr[0], 4),  9);
         line += rightJustify(asString(antennaPhaseCtr[1], 4), 14);
         line += rightJustify(asString(antennaPhaseCtr[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaPhaseCtr;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validAntennaBsightXYZ))
      {
         line  = rightJustify(asString(antennaBsightXYZ[0], 4), 14);
         line += rightJustify(asString(antennaBsightXYZ[1], 4), 14);
         line += rightJustify(asString(antennaBsightXYZ[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaBsightXYZ;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validAntennaZeroDirAzi))
      {
         line  = rightJustify(asString(antennaZeroDirAzi, 4), 14);
         line += string(46, ' ');
         line += hsAntennaZeroDirAzi;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validAntennaZeroDirXYZ))
      {
         line  = rightJustify(asString(antennaZeroDirXYZ[0], 4), 14);
         line += rightJustify(asString(antennaZeroDirXYZ[1], 4), 14);
         line += rightJustify(asString(antennaZeroDirXYZ[2], 4), 14);
         line += string(18, ' ');
         line += hsAntennaZeroDirXYZ;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validCenterOfMass))
      {
         line  = rightJustify(asString(centerOfMass[0], 4), 14);
         line += rightJustify(asString(centerOfMass[1], 4), 14);
         line += rightJustify(asString(centerOfMass[2], 4), 14);
         line += string(18, ' ');
         line += hsCenterOfMass;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version < 3 && (valid & validNumObs))          // R2 only
      {
         if(R2ObsTypes.empty())
         {
            InvalidRequest er("Header contains no R2ObsTypes. "
                                 "You must run prepareVer2Write before outputting an R2 file");
            GPSTK_THROW(er);
         }
            // write out RinexObsTypes
         const int maxObsPerLine = 9;
         int obsWritten = 0;
         line = ""; // make sure the line contents are reset.

         for(size_t i=0; i<R2ObsTypes.size(); i++)
         {
            string val;
               // the first line needs to have the # of obs
            if(obsWritten == 0)
               line  = rightJustify(asString(R2ObsTypes.size()), 6);
               // if you hit 9, write out the line and start a new one
            else if((obsWritten % maxObsPerLine) == 0)
            {
               line += hsNumObs;
               strm << line << endl;
               strm.lineNumber++;
               line  = string(6, ' ');
            }
            val = R2ObsTypes[i];
            line += rightJustify(val, 6);
            obsWritten++;
         }

         line += string(60 - line.size(), ' ');
         line += hsNumObs;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validSystemNumObs))
      {
         static const int maxObsPerLine = 13;

         map<string,vector<RinexObsID> >::const_iterator mapIter;

            // because of the way pseudo-observables are mapped from
            // ObsID/RinexObsID to the RINEX file, we have to manually
            // count the observables first.
         map<string,unsigned> obsCount;
         RinexObsMap dummy;
         remapObsTypes(dummy, obsCount);
            // now do the actual writing
         for(mapIter = mapObsTypes.begin(); mapIter != mapObsTypes.end();
             mapIter++)
         {
            bool addedChannel = false;
            std::set<CarrierBand> addedIono;
            int obsWritten = 0;
            line = ""; // make sure the line contents are reset

            vector<RinexObsID> ObsTypeList = mapIter->second;

            for(size_t i = 0; i < ObsTypeList.size(); i++)
            {
               if (ObsTypeList[i].type == ObservationType::Iono)
               {
                  if (addedIono.count(ObsTypeList[i].band) > 0)
                     continue; // only write this pseudo-obs once
                  addedIono.insert(ObsTypeList[i].band);
               }
               else if (ObsTypeList[i].type == ObservationType::Channel)
               {
                  if (addedChannel)
                     continue; // only write this pseudo-obs once
                  addedChannel = true;
               }
                  // the first line needs to have the GNSS type and # of obs
               if(obsWritten == 0)
               {
                  line  =  leftJustify(mapIter->first, 1);
                  line += string(2, ' ');
                  line += rightJustify(asString(obsCount[mapIter->first]), 3);
               }
                  // if you hit 13, write out the line and start a new one
               else if((obsWritten % maxObsPerLine) == 0)
               {
                  line += string(2, ' ');
                  line += hsSystemNumObs;
                  strm << line << endl;
                  strm.lineNumber++;
                  line  = string(6, ' ');
               }
               line += string(1, ' ');
               line += rightJustify(ObsTypeList[i].asString(version), 3);
               obsWritten++;
            }
            line += string(60 - line.size(), ' ');
            line += hsSystemNumObs;
            strm << line << endl;
            strm.lineNumber++;
         }

      }
      if(version < 3 && (valid & validWaveFact))
      {
         line  = rightJustify(asString<short>(wavelengthFactor[0]),6);
         line += rightJustify(asString<short>(wavelengthFactor[1]),6);
         line += string(48, ' ');
         line += hsWaveFact;
         strm << line << endl;
         strm.lineNumber++;
         
            // handle continuation lines
         if(!extraWaveFactList.empty())
         {
            vector<ExtraWaveFact>::const_iterator itr = extraWaveFactList.begin();
            
            while (itr != extraWaveFactList.end())
            {
               const int maxSatsPerLine = 7;
               short satsWritten = 0, satsLeft = (*itr).satList.size(), satsThisLine;
               vector<SatID>::const_iterator vecItr = (*itr).satList.begin();

               while ((vecItr != (*itr).satList.end()))
               {
                  if(satsWritten == 0)
                  {
                     line  = rightJustify(asString<short>((*itr).wavelengthFactor[0]),6);
                     line += rightJustify(asString<short>((*itr).wavelengthFactor[1]),6);
                     satsThisLine = (satsLeft > maxSatsPerLine ? maxSatsPerLine : satsLeft);
                     line += rightJustify(asString<short>(satsThisLine),6);
                  }
                  try
                  {
                     line += string(3, ' ') + RinexSatID(*vecItr).toString();
                  }
                  catch (Exception& e)
                  {
                     FFStreamError ffse(e);
                     GPSTK_THROW(ffse);
                  }
                  satsWritten++;
                  satsLeft--;
                  if(satsWritten==maxSatsPerLine || satsLeft==0)
                  {
                        // output a complete line
                     line += string(60 - line.size(), ' ');
                     line += hsWaveFact;
                     strm << line << endl;
                     strm.lineNumber++;
                     satsWritten = 0;
                  }
                  vecItr++;
               }
               itr++;
            }
         }
      }
      if(version >= 3 && valid & validSigStrengthUnit)
      {
         line  = leftJustify(sigStrengthUnit, 20);
         line += string(40, ' ');
         line += hsSigStrengthUnit;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validInterval)
      {
         line  = rightJustify(asString(interval, 3), 10);
         line += string(50, ' ');
         line += hsInterval;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validFirstTime)
      {
         line  = writeTime(firstObs);
         line += string(60 - line.size(), ' ');
         line += hsFirstTime;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validLastTime)
      {
         line  = writeTime(lastObs);
         line += string(60 - line.size(), ' ');
         line += hsLastTime;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validReceiverOffset)
      {
         line  = rightJustify(asString(receiverOffset), 6);
         line += string(54, ' ');
         line += hsReceiverOffset;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(version >= 3 && (valid & validSystemDCBSapplied))
      {
         for(size_t i = 0; i < infoDCBS.size(); i++)
         {
            line  = leftJustify(infoDCBS[i].satSys,  1);
            line += string(1, ' ');
            line += leftJustify(infoDCBS[i].name  , 17);
            line += string(1, ' ');
            line += leftJustify(infoDCBS[i].source, 40);
            line += hsSystemDCBSapplied;
            strm << line << endl;
            strm.lineNumber++;
         }
      }
      if(version >= 3 && (valid & validSystemPCVSapplied))
      {
         for(size_t i = 0; i < infoPCVS.size(); i++)
         {
            line  = leftJustify(infoPCVS[i].satSys,  1);
            line += string(1, ' ');
            line += leftJustify(infoPCVS[i].name  , 17);
            line += string(1, ' ');
            line += leftJustify(infoPCVS[i].source, 40);
            line += hsSystemPCVSapplied;
            strm << line << endl;
            strm.lineNumber++;
         }
      }
      if(version >= 3 && (valid & validSystemScaleFac))
      {
         static const int maxObsPerLine = 12;

         static const int size = 4;
         static const int factors[size] = {1,10,100,1000};
         vector<string> obsTypes;

            // loop over GNSSes
         map<string, ScaleFacMap>::const_iterator mapIter;
         for(mapIter = sysSfacMap.begin(); mapIter != sysSfacMap.end(); mapIter++)
         {
            map<RinexObsID, int>::const_iterator iter;

            for(int i = 0; i < size; i++) // loop over possible factors (above)
            {
               int count = 0;
               obsTypes.clear(); // clear the list of Obs Types we're going to make

               for(iter = mapIter->second.begin();      // loop over scale factor map
                   iter != mapIter->second.end(); iter++)
               {
                  if(iter->second == factors[i] )
                  {
                     count++;
                     obsTypes.push_back(iter->first.asString(version));
                  }
               }

               if(count == 0 ) continue;

               line  =  leftJustify(mapIter->first      , 1);
               line += string(1, ' ');
               line += rightJustify(asString(factors[i]), 4);
               line += string(2, ' ');
               line += rightJustify(asString(count     ), 2);

               for(int j = 0; j < count; j++)
               {
                  if(j > maxObsPerLine-1 && (j % maxObsPerLine) == 0 )
                  {
                        // need continuation; end current line
                     line += string(2, ' ');
                     line += hsSystemScaleFac;
                     strm << line << endl;
                     strm.lineNumber++;
                     line  = string(10, ' ');
                  }
                  line += string(1, ' ');
                  line += rightJustify(obsTypes[j], 3);
               }
               int space = 60 - 10 - 4*(count % maxObsPerLine);
               line += string(space, ' ');
               line += hsSystemScaleFac;
               strm << line << endl;
               strm.lineNumber++;
            }
         }
      }
      if(version >= 3.01 && (valid & validSystemPhaseShift))
      {
            //map<string, map<RinexObsID, map<RinexSatID,double> > > sysPhaseShift;
         map<string, map<RinexObsID, map<RinexSatID,double> > >::const_iterator it;
         for(it=sysPhaseShift.begin(); it!=sysPhaseShift.end(); ++it)
         {
            string sys(it->first);
            map<RinexObsID, map<RinexSatID,double> >::const_iterator jt(it->second.begin());
            if(jt == it->second.end())
            {
               line  = sys;
               line += string(60-line.length(), ' ');
               line += hsSystemPhaseShift;
               strm << line << endl;
               strm.lineNumber++;
            }
            else
            {
               for( ; jt!=it->second.end(); ++jt)
               {
                  RinexSatID sat(jt->second.begin()->first);
                  double corr(jt->second.begin()->second);
                  if (jt->first.type != ObservationType::Phase)
                  {
                        // Phase shift only makes sense for phase measurements.
                     continue;
                  }
                  line = sys + " ";
                  line += leftJustify(jt->first.asString(version),3) + " ";
                  line += rightJustify(asString(corr,5),8);
                  if(sat.id == -1)
                  {
                     line += string(60-line.length(), ' ');
                     line += hsSystemPhaseShift;
                     strm << line << endl;
                     strm.lineNumber++;
                  }
                  else
                  {
                        // list of sats
                     setfill('0');
                     line += string("  ") + rightJustify(asString(jt->second.size()),2);
                     setfill(' ');

                     int n(0);
                     map<RinexSatID,double>::const_iterator kt,lt;
                     for(kt=jt->second.begin(); kt!=jt->second.end(); ++kt)
                     {
                        line += string(" ") + kt->first.toString();
                        if(++n == 10 || ++(lt=kt) == jt->second.end())
                        {
                              // end this line
                           line += string(60-line.length(), ' ');
                           line += hsSystemPhaseShift;
                           strm << line << endl;
                           strm.lineNumber++;
                           n = 0;
                              // are there more for a continuation line?
                           if(lt != jt->second.end())
                              line = string(18,' ');
                        } // if(++n == 10 || ++(lt=kt) == jt->second.end())
                     } // for(kt=jt->second.begin(); kt!=jt->second.end(); ++kt)
                  } // else
               } // for( ; jt!=it->second.end(); ++jt)
            } // else
         } // for(it=sysPhaseShift.begin(); it!=sysPhaseShift.end(); ++it)
      } // if(version >= 3.01 && (valid & validSystemPhaseShift))
      if(version >= 3.01)
      {
         if ((valid & validGlonassSlotFreqNo))
         {
            size_t n(0), nsat(glonassFreqNo.size());
            line = rightJustify(asString(nsat), 3) + " ";
            GLOFreqNumMap::const_iterator it, kt;
            for (it = glonassFreqNo.begin(); it != glonassFreqNo.end(); ++it) {
               line += it->first.toString();
               line += rightJustify(asString(it->second), 3) + " ";
               if (++n == 8 || ++(kt = it) == glonassFreqNo.end()) {
                  // write it
                  line += string(60 - line.length(), ' ');
                  line += hsGlonassSlotFreqNo;
                  strm << line << endl;
                  strm.lineNumber++;
                  n = 0;
                  // are there more for a continuation line?
                  if (kt != glonassFreqNo.end())
                     line = string(4, ' ');
               }
            }
         }
         else if(mapObsTypes.find("R") != mapObsTypes.end())
         {
            FFStreamError err("Glonass Slot Freq No required for files containing Glonass Observations ");
            GPSTK_THROW(err);
         }
      }
      if(version >= 3.02)
      {
         if ((valid & validGlonassCodPhsBias))
         {
            line.clear();
            GLOCodPhsBias::const_iterator it;
            const string labs[4] = {"C1C", "C1P", "C2C", "C2P"};
            for (int i = 0; i < 4; i++) {
               RinexObsID obsid(RinexObsID("R" + labs[i], version, _bIsPossiblySqm));
               it = glonassCodPhsBias.find(obsid);
               double bias = 0.0;
               if (it != glonassCodPhsBias.end())
                  bias = it->second;
               line += " " + labs[i] + " " + rightJustify(asString(bias, 3), 8);
            }
            line += string(60 - line.length(), ' ');
            line += hsGlonassCodPhsBias;
            strm << line << endl;
            strm.lineNumber++;
         }
         else if(mapObsTypes.find("R") != mapObsTypes.end())
         {
            FFStreamError err("Glonass Code Phase Bias required for files containing Glonass Observations ");
            GPSTK_THROW(err);
         }
      }
      if(valid & validLeapSeconds)
      {
         line  = rightJustify(asString(leapSeconds), 6);
         line += string(54, ' ');
         line += hsLeapSeconds;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validNumSats)
      {
         line  = rightJustify(asString(numSVs), 6);
         line += string(54, ' ');
         line += hsNumSats;
         strm << line << endl;
         strm.lineNumber++;
      }
      if(valid & validPrnObs)
      {
         static const int maxObsPerLine = 9;
         map<RinexSatID, vector<int> >::const_iterator itr(numObsForSat.begin());
            // loop over satellites
         while(itr != numObsForSat.end())
         {
            int numObsWritten = 0;                                // # of counts written for this sat
            RinexSatID sat(itr->first);                           // the sat
            const vector<int>& numObs(itr->second);               // the vector of ints stored
            vector<int> vec;                                      // the vector of ints to write

            if(version >= 3)
               vec = numObs;
            else
            {
                  // fill in zeros for version 2
               int j;
               size_t i;
               string sys(string(1,sat.systemChar()));
               map<string, map<string, RinexObsID> >::const_iterator jt(mapSysR2toR3ObsID.find(sys));
               const map<string, RinexObsID> mapVec(jt->second);
               map<string, RinexObsID>::const_iterator kt;
               for(i=0,j=0; i<R2ObsTypes.size(); i++)
               {
                  kt = mapVec.find(R2ObsTypes[i]);
                  string obsid(kt->second.asString(version));
                  if(obsid == string("   ")) vec.push_back(0.0);
                  else                       vec.push_back(numObs[j++]);
               }
            }

            vector<int>::const_iterator vecItr(vec.begin());
            while (vecItr != vec.end())
            {
               if(numObsWritten == 0)
               {
                     // start of line
                  try
                  {
                     line = string(3, ' ') + sat.toString();      // '   G01'
                  }
                  catch (Exception& e)
                  {
                     FFStreamError ffse(e);
                     GPSTK_RETHROW(ffse); 
                  }
               }
               else if((numObsWritten % maxObsPerLine) == 0)
               {
                     // end of line
                  line += hsPrnObs;
                  strm << line << endl;
                  strm.lineNumber++;
                  line  = string(6, ' ');
               }

               line += rightJustify(asString(*vecItr), 6);        // add num obs to line
               ++vecItr;
               ++numObsWritten;
            }

               // finish last line
            line += string(60 - line.size(), ' ');
            line += hsPrnObs;
            strm << line << endl;
            strm.lineNumber++;
            itr++;
         }
      }
      if(validEoH)
      {
         line  = string(60, ' ');
         line += hsEoH;
         strm << line << endl;
         strm.lineNumber++;
      }
   } // end writeHeaderRecords


      // This function parses a single header record.
   void Rinex3ObsHeader::parseHeaderRecord(string& line)
   {
      int i;
      string label(line, 60, 20);
         
      if(label == hsVersion)
      {
         version  = asDouble(line.substr( 0,20));
         fileType = strip(   line.substr(20,20));
         fileSys  = strip(   line.substr(40,20));

         if(fileSys[0] != 'M' && fileSys[0] != 'm')
         {
            RinexSatID sat;
            sat.fromString(fileSys);
            fileSysSat = SatID(sat);
         }
         else
            fileSysSat = SatID(-1,SatelliteSystem::Mixed);

         if(fileType[0] != 'O' && fileType[0] != 'o')
         {
            FFStreamError e("This isn't a RINEX 3 Obs file.");
            GPSTK_THROW(e);
         }

         valid |= validVersion;
      }
      else if(label == hsRunBy)
      {
         fileProgram = strip(line.substr( 0,20));
         fileAgency  = strip(line.substr(20,20));
         date        = strip(line.substr(40,20));
         valid |= validRunBy;
      }
      else if(label == hsComment)
      {
         commentList.push_back(strip(line.substr(0,60)));
         valid |= validComment;
      }
      else if(label == hsMarkerName)
      {
         markerName = strip(line.substr(0,60));
         valid |= validMarkerName;
      }
      else if(label == hsMarkerNumber)
      {
         markerNumber = strip(line.substr(0,20));
         valid |= validMarkerNumber;
      }
      else if(label == hsMarkerType)
      {
         markerType = strip(line.substr(0,20));
         valid |= validMarkerType;
      }
      else if(label == hsObserver)
      {
         observer = strip(line.substr( 0,20));
         agency   = strip(line.substr(20,40));
         valid |= validObserver;
      }
      else if(label == hsReceiver)
      {
         recNo   = strip(line.substr( 0,20));
         recType = strip(line.substr(20,20));
         recVers = strip(line.substr(40,20));
         valid |= validReceiver;
      }
      else if(label ==hsAntennaType)
      {
         antNo   = strip(line.substr( 0,20));
         antType = strip(line.substr(20,20));
         valid |= validAntennaType;
      }
      else if(label == hsAntennaPosition)
      {
         antennaPosition[0] = asDouble(line.substr( 0,14));
         antennaPosition[1] = asDouble(line.substr(14,14));
         antennaPosition[2] = asDouble(line.substr(28,14));
         valid |= validAntennaPosition;
      }
      else if(label == hsAntennaDeltaHEN)
      {
         antennaDeltaHEN[0] = asDouble(line.substr( 0,14));
         antennaDeltaHEN[1] = asDouble(line.substr(14,14));
         antennaDeltaHEN[2] = asDouble(line.substr(28,14));
         valid |= validAntennaDeltaHEN;
      }
      else if(label == hsAntennaDeltaXYZ)
      {
         antennaDeltaXYZ[0] = asDouble(line.substr( 0,14));
         antennaDeltaXYZ[1] = asDouble(line.substr(14,14));
         antennaDeltaXYZ[2] = asDouble(line.substr(28,14));
         valid |= validAntennaDeltaXYZ;
      }
      else if(label == hsAntennaPhaseCtr)
      {
         antennaSatSys  = strip(line.substr(0,2));
         antennaObsCode = strip(line.substr(2,3));
         antennaPhaseCtr[0] = asDouble(line.substr( 5, 9));
         antennaPhaseCtr[1] = asDouble(line.substr(14,14));
         antennaPhaseCtr[2] = asDouble(line.substr(28,14));
         valid |= validAntennaPhaseCtr;
      }
      else if(label == hsAntennaBsightXYZ)
      {
         antennaBsightXYZ[0] = asDouble(line.substr( 0,14));
         antennaBsightXYZ[1] = asDouble(line.substr(14,14));
         antennaBsightXYZ[2] = asDouble(line.substr(28,14));
         valid |= validAntennaBsightXYZ;
      }
      else if(label == hsAntennaZeroDirAzi)
      {
         antennaZeroDirAzi = asDouble(line.substr(0,14));
         valid |= validAntennaBsightXYZ;
      }
      else if(label == hsAntennaZeroDirXYZ)
      {
         antennaZeroDirXYZ[0] = asDouble(line.substr( 0,14));
         antennaZeroDirXYZ[1] = asDouble(line.substr(14,14));
         antennaZeroDirXYZ[2] = asDouble(line.substr(28,14));
         valid |= validAntennaBsightXYZ;
      }
      else if(label == hsCenterOfMass)
      {
         centerOfMass[0] = asDouble(line.substr( 0,14));
         centerOfMass[1] = asDouble(line.substr(14,14));
         centerOfMass[2] = asDouble(line.substr(28,14));
         valid |= validCenterOfMass;
      }
      else if(label == hsNumObs)        // R2 only
      {
         if(version >= 3)
         {
            FFStreamError e("RINEX 2 record in RINEX 3 file: " + label);
            GPSTK_THROW(e);
         }

            // process the first line
         if(!(valid & validNumObs))
         {
            numObs = asInt(line.substr(0,6));
            valid |= validNumObs;
         }
         
         const int maxObsPerLine = 9;
         for(i = 0; (R2ObsTypes.size() < numObs) && (i < maxObsPerLine); i++)
            R2ObsTypes.push_back(line.substr(i * 6 + 10, 2));
      }
      else if(label == hsSystemNumObs)
      {
         if(version < 3)
         {
            FFStreamError e("RINEX 3 record in RINEX 2 file: " + label);
            GPSTK_THROW(e);
         }

         string satSys = strip(line.substr(0,1));
         if (satSys != "")
         {
            numObs = asInt(line.substr(3,3));
            valid |= validSystemNumObs;
            satSysPrev = satSys;
         }
         else
            satSys = satSysPrev;
         
         try
         {
            const int maxObsPerLine = 13;
            for (int i=0;
                 i < maxObsPerLine && mapObsTypes[satSys].size() < numObs; i++)
            {
               string obstype(line.substr(4 * i + 7, 3));
               mapObsTypes[satSys].push_back(
                  RinexObsID(satSys+obstype,version, _bIsPossiblySqm));
            }
         }
         catch(InvalidParameter& ip)
         {
            FFStreamError fse("InvalidParameter: "+ip.what());
            GPSTK_THROW(fse);
         }
      }
      else if(label == hsWaveFact)         // R2 only
      {
            // first time reading this
         if(!(valid & validWaveFact))
         {
            wavelengthFactor[0] = asInt(line.substr(0,6));
            wavelengthFactor[1] = asInt(line.substr(6,6));
            valid |= validWaveFact;
         }
         else
         {
               // additional wave fact lines
            const int maxSatsPerLine = 7;
            int Nsats;
            ExtraWaveFact ewf;
            ewf.wavelengthFactor[0] = asInt(line.substr(0,6));
            ewf.wavelengthFactor[1] = asInt(line.substr(6,6));
            Nsats = asInt(line.substr(12,6));
               
            if(Nsats > maxSatsPerLine)   // > not >=
            {
               FFStreamError e("Invalid number of Sats for " + hsWaveFact);
               GPSTK_THROW(e);
            }
               
            for(i = 0; i < Nsats; i++)
            {
               try
               {
                  RinexSatID prn(line.substr(21+i*6,3));
                  ewf.satList.push_back(prn); 
               }
               catch (Exception& e)
               {
                  FFStreamError ffse(e);
                  GPSTK_RETHROW(ffse);
               }
            }
               
            extraWaveFactList.push_back(ewf);
         }
      }
      else if(label == hsSigStrengthUnit)
      {
         sigStrengthUnit = strip(line.substr(0,20));
         valid |= validSigStrengthUnit;
      }
      else if(label == hsInterval)
      {
         interval = asDouble(line.substr(0,10));
         valid |= validInterval;
      }
      else if(label == hsFirstTime)
      {
         firstObs = parseTime(line);
         valid |= validFirstTime;
      }
      else if(label == hsLastTime)
      {
         lastObs = parseTime(line);
         valid |= validLastTime;
      }
      else if(label == hsReceiverOffset)
      {
         receiverOffset = asInt(line.substr(0,6));
         valid |= validReceiverOffset;
      }

      else if(label == hsSystemDCBSapplied)
      {
         Rinex3CorrInfo tempInfo;
         tempInfo.satSys = strip(line.substr( 0, 1));
         tempInfo.name   = strip(line.substr( 2,17));
         tempInfo.source = strip(line.substr(20,40));
         infoDCBS.push_back(tempInfo);
         valid |= validSystemDCBSapplied;
      }
      else if(label == hsSystemPCVSapplied)
      {
         Rinex3CorrInfo tempInfo;
         tempInfo.satSys = strip(line.substr( 0, 1));
         tempInfo.name   = strip(line.substr( 2,17));
         tempInfo.source = strip(line.substr(20,40));
         infoPCVS.push_back(tempInfo);
         valid |= validSystemPCVSapplied;
      }
      else if(label == hsSystemScaleFac)
      {
         static const int maxObsPerLine = 12;

         satSysTemp = strip(line.substr(0,1));
         factor     = asInt(line.substr(2,4));
         numObs     = asInt(line.substr(8,2));

         int startPosition = 0;

         if(satSysTemp == "" )
         {
               // it's a continuation line; use prev. info., end pt. to start
            satSysTemp = satSysPrev;
            factor     = factorPrev;
            numObs     = numObsPrev;

            startPosition = sysSfacMap[satSysTemp].size();
         }

            // 0/blank numObs means factor applies to all obs types
            // in appropriate obsTypeList
         if(numObs == 0)
            numObs = mapObsTypes[satSysTemp].size();

         ScaleFacMap tempSfacMap = sysSfacMap[satSysTemp];
         for(i = startPosition;
             (i < numObs) && ((i % maxObsPerLine) < maxObsPerLine); i++)
         {
            int position = 4*(i % maxObsPerLine) + 10 + 1;
            RinexObsID tempType(satSysTemp+strip(line.substr(position,3)),
                                version);
            tempSfacMap.insert(make_pair(tempType,factor));
         }
         sysSfacMap[satSysTemp] = tempSfacMap;

         ScaleFacMap::const_iterator iter;
         ScaleFacMap tempmap;
         tempmap = sysSfacMap[satSysTemp];

            // save values in case next line is a continuation line
         satSysPrev = satSysTemp;
         factorPrev = factor;
         numObsPrev = numObs;

         valid |= validSystemScaleFac;
      }
      else if(label == hsSystemPhaseShift || label == hsSystemPhaseShift+"S") ///< "SYS / PHASE SHIFT"    R3.01
      {
            // The above +"S" is that some files pluralize this...
         RinexSatID sat;
            // system
         satSysTemp = strip(line.substr(0,1));

         if(satSysTemp.empty())
         {
               // continuation line
            satSysTemp = satSysPrev;

            if(sysPhaseShift[satSysTemp].find(sysPhaseShiftObsID)
               == sysPhaseShift[satSysTemp].end())
            {
               //FFStreamError e("SYS / PHASE SHIFT: unexpected continuation line");
               //GPSTK_THROW(e);
               // lenient - some writers have only a single blank record
               return;
            }

            map<RinexSatID,double>& satcorrmap(sysPhaseShift[satSysTemp][sysPhaseShiftObsID]);
            double cor(sysPhaseShift[satSysTemp][sysPhaseShiftObsID].begin()->second);
            for(i=0; i<10; i++)
            {
               string str = strip(line.substr(19+4*i,3));
               if(str.empty()) break;
               sat = RinexSatID(str);
               satcorrmap.insert(make_pair(sat,cor));
            }
         }
         else
         {
               // not a cont. line
            sat.fromString(satSysTemp);
            if(sysPhaseShift.find(satSysTemp) == sysPhaseShift.end())
            {
               map<RinexObsID, map<RinexSatID, double> > obssatcormap;
               sysPhaseShift.insert(make_pair(satSysTemp,obssatcormap));
            }

               // obs id
            string str = strip(line.substr(2,3));

               // obsid and correction may be blank <=> unknown: ignore this
            if(!str.empty())
            {
               RinexObsID obsid(satSysTemp+str, version);
               double cor(asDouble(strip(line.substr(6,8))));
               int nsat(asInt(strip(line.substr(16,2))));
               if(nsat > 0)
               {
                     // list of sats
                  map<RinexSatID,double> satcorrmap;
                  for(i=0; i<(nsat < 10 ? nsat : 10); i++)
                  {
                     sat = RinexSatID(strip(line.substr(19+4*i,3)));
                     satcorrmap.insert(make_pair(sat,cor));
                  }
                  sysPhaseShift[satSysTemp].insert(make_pair(obsid,satcorrmap));
                  if(nsat > 10)        // expect continuation
                     sysPhaseShiftObsID = obsid;
               }
               else
               {
                     // no sat, just system
                  map<RinexSatID,double> satcorrmap;
                  satcorrmap.insert(make_pair(sat,cor));
                  sysPhaseShift[satSysTemp].insert(make_pair(obsid,satcorrmap));
               }
            }

               // save for continuation lines
            satSysPrev = satSysTemp;

            valid |= validSystemPhaseShift;
         }
      }
      else if(label == hsGlonassSlotFreqNo)
      {
            //map<RinexSatID,int> glonassFreqNo;
         int tmp;
         RinexSatID sat;
         string str(strip(line.substr(0,3)));

         for(i=0; i<8; i++)
         {
            str = strip(line.substr(4+i*7,3));
            if(str.empty()) break;
            sat = RinexSatID(str);
            str = strip(line.substr(8+i*7,2));
            tmp = asInt(str);
            glonassFreqNo.insert(make_pair(sat,tmp));
         }

         valid |= validGlonassSlotFreqNo;
      }
      else if(label == hsGlonassCodPhsBias || label == hsGlonassCodPhsBias+"#")
      {
            // several IGS MGEX 3.02 files do the extra "#"
            //std::map<RinexObsID,double> glonassCodPhsBias; ///< "GLONASS COD/PHS/BIS"            R3.02
         for(i=0; i<4; i++)
         {
            string str(strip(line.substr(i*13+1,3)));
            if(str.empty()) continue;
            RinexObsID obsid("R"+str, version);
            double bias(asDouble(strip(line.substr(i*13+5,8))));
            glonassCodPhsBias[obsid] = bias;
         }
         valid |= validGlonassCodPhsBias;
      }
      else if(label == hsLeapSeconds)
      {
         leapSeconds = asInt(line.substr(0,6));
         valid |= validLeapSeconds;
      }
      else if(label == hsNumSats)
      {
         numSVs = asInt(line.substr(0,6)) ;
         valid |= validNumSats;
      }
      else if(label == hsPrnObs)
      {
            // this assumes 'PRN / # OF OBS' comes after '# / TYPES OF OBSERV' or 'SYS / # / OBS TYPES'
            // NOT a good assumption for auxiliary header... ignore in that case
         if(version >= 3.0 && mapObsTypes.size() == 0)
         {
            commentList.push_back(
               string("Warning - can't read PRN/OBS in auxHeader: no"
                      " mapObsTypes"));
            return;
         }

         static const int maxObsPerLine = 9;

         int j,otmax;
         RinexSatID PRN;
         string prn, GNSS;
         vector<int> numObsList;

         prn = strip(line.substr(3,3));

         if(prn == "" ) // this is a continuation line; use last PRN
         {
            PRN = lastPRN;
            GNSS = PRN.systemChar();
            if(version < 3)
               otmax = R2ObsTypes.size();
            else
            {
               if(mapObsTypes.find(GNSS) == mapObsTypes.end())
               {
                  Exception e("PRN/#OBS for system "+PRN.toString()+" not found in SYS/#/OBS");
                  GPSTK_THROW(e);
               }
               otmax = mapObsTypes[GNSS].size();
            }

            numObsList = numObsForSat[PRN]; // grab the existing list

            for(j=0,i=numObsList.size(); j<maxObsPerLine && i<otmax; i++,j++)
               numObsList.push_back(asInt(line.substr(6*j+6,6)));

            numObsForSat[PRN] = numObsList;
         }
         else             // this is a new PRN line
         {
            PRN = RinexSatID(prn);
            GNSS = PRN.systemChar();
            if(version < 3)
               otmax = R2ObsTypes.size();
            else
            {
               if(mapObsTypes.find(GNSS) == mapObsTypes.end())
               {
                  Exception e("PRN/#OBS for system "+PRN.toString()+" not found in SYS/#/OBS");
                  GPSTK_THROW(e);
               }
               otmax = mapObsTypes[GNSS].size();
            }

            for(i=0; i<maxObsPerLine && i<otmax; i++)
               numObsList.push_back(asInt(line.substr(6*i+6,6)));

            numObsForSat[PRN] = numObsList;

            lastPRN = PRN;
         }

         valid |= validPrnObs;
      }
      else if(label == hsEoH)
      {
         validEoH = true;
      }
      else
      {
         FFStreamError e("Unidentified label: >" + label + "<");
         GPSTK_THROW(e);
      }
   } // end of parseHeaderRecord


      // This function parses the entire header from the given stream
   void Rinex3ObsHeader::reallyGetRecord(FFStream& ffs)
   {
      using gpstk::StringUtils::asString;
      Rinex3ObsStream& strm = dynamic_cast<Rinex3ObsStream&>(ffs);

         // If already read, just return.
      if(strm.headerRead == true) return;

         // Since we're reading a new header, we need to reinitialize
         // all our list structures. All the other objects should be
         // ok.  This also applies if we threw an exception the first
         // time we read the header and are now re-reading it.  Some
         // of these could be full and we need to empty them.
      clear();

      string line;

      while (!validEoH)
      {
         strm.formattedGetLine(line);
         StringUtils::stripTrailing(line);

         if(line.length() == 0)
         {
            FFStreamError e("No data read");
            GPSTK_THROW(e);
         }
         else if(line.length() < 60 || line.length() > 80)
         {
            FFStreamError e("Invalid line length");
            GPSTK_THROW(e);
         }

         try
         {
            parseHeaderRecord(line);
         }
         catch(FFStreamError& e)
         {
            GPSTK_RETHROW(e);
         }
         catch(Exception& e)
         {
            FFStreamError fse("Exception: "+e.what());
            GPSTK_THROW(fse);
         }

      } // end while(not end of header)

         // if RINEX 2, define mapObsTypes from R2ObsTypes and
         // system(s) this may have to be corrected later using
         // wavelengthFactor also define mapSysR2toR3ObsID in case
         // version 2 is written out later
      if(version < 3)
      {
            // try to determine systems included in the file
         vector<string> syss;                // 1-char strings "G" "R" "E" ...
         if(numObsForSat.size() > 0)
         {
               // get syss from PRN/#OBS
            map<RinexSatID, vector<int> >::const_iterator it;
            for(it=numObsForSat.begin(); it != numObsForSat.end(); ++it)
            {
               string sys(string(1,(it->first).systemChar()));
               if(find(syss.begin(),syss.end(),sys) == syss.end())
                  syss.push_back(sys);
            }
         }
         else if(fileSysSat.system != SatelliteSystem::Mixed)
         {
               // only one system in this file
            syss.push_back(string(1,RinexSatID(fileSysSat).systemChar()));
         }
         else
         {
               // have to replicate obs type list for all RINEX2 systems
            syss.push_back("G");
            syss.push_back("R");
            syss.push_back("S");    // ??
            syss.push_back("E");
         }

            // given systems and list of R2ObsTypes, compute
            // mapObsTypes and mapSysR2toR3ObsID
         mapSysR2toR3ObsID.clear();
         for(size_t i=0; i<syss.size(); i++)
         {
            const string s(syss[i]);
            vector<RinexObsID> obsids;
            
            try 
            {
               if      (s=="G")
                  obsids = mapR2ObsToR3Obs_G(); 
               else if (s=="R") 
                  obsids = mapR2ObsToR3Obs_R();
               else if (s=="E") 
                  obsids = mapR2ObsToR3Obs_E();
               else if (s=="S") 
                  obsids = mapR2ObsToR3Obs_S();
            }
            catch(FFStreamError fse)
            {
               GPSTK_RETHROW(fse); 
            }

               // TD if GPS and have wavelengthFactors, add more
               // ObsIDs with tc=N

            mapObsTypes[syss[i]] = obsids;
         }

            // modify numObsForSat if necessary
         map<RinexSatID, vector<int> >::const_iterator it(numObsForSat.begin());
         for( ; it != numObsForSat.end(); ++it)
         {
            RinexSatID sat(it->first);
            string sys;
            sys = sat.systemChar();
            vector<int> vec;
            for(size_t i=0; i<R2ObsTypes.size(); i++)
            {
               if(mapSysR2toR3ObsID[sys][R2ObsTypes[i]].asString(version) == string("   "))
                  ;
               else
                  vec.push_back(it->second[i]);
            }
            numObsForSat[sat] = vec;
         }
      }

         // Since technically the Phase Shift record is required in ver 3.01,
         // create SystemPhaseShift record(s) if not present.
         //map<string, map<RinexObsID, map<RinexSatID,double> > > sysPhaseShift;
      if(version >= 3.01 && (valid & validSystemNumObs)
         && !(valid & validSystemPhaseShift))
      {
            // loop over obs types to get systems
         map<string,vector<RinexObsID> >::const_iterator iter;
         for(iter=mapObsTypes.begin(); iter != mapObsTypes.end(); iter++)
         {
            string sys(iter->first);
            if(sysPhaseShift.find(sys) == sysPhaseShift.end())
            {
               map<RinexObsID, map<RinexSatID, double> > dummy;
               sysPhaseShift.insert(make_pair(sys,dummy));
            }
         }
         valid |= validSystemPhaseShift;
      }

         // is the header valid?
      Fields allValid = Fields::getRequired(version);
      if (allValid.empty())
      {
         FFStreamError e("Unknown or unsupported RINEX version " + 
                         asString(version,2));
         GPSTK_THROW(e);
      }

      if((valid & allValid) != allValid)
      {
         FFStreamError e("Incomplete or invalid header");
         allValid.describeMissing(valid, e);
         GPSTK_THROW(e);
      }

         // If we get here, we should have reached the end of header line.
      strm.header = *this;
      strm.headerRead = true;

         // determine the time system of epochs in this file; cf. R3.02 Table A2
         // 1.determine time system from time tag in TIME OF FIRST OBS record
         // 2.if not given, determine from type in RINEX VERSION / TYPE record
         // 3.(if the type is MIXED, the time system in firstObs is required by RINEX)
      strm.timesystem = firstObs.getTimeSystem();
      if(strm.timesystem == TimeSystem::Any ||
         strm.timesystem == TimeSystem::Unknown)
      {
         if(fileSysSat.system == SatelliteSystem::GPS)
         {
            strm.timesystem = TimeSystem::GPS;
            firstObs.setTimeSystem(TimeSystem::GPS);
         }
         else if(fileSysSat.system == SatelliteSystem::Glonass)
         {
            strm.timesystem = TimeSystem::UTC;
            firstObs.setTimeSystem(TimeSystem::UTC);
         }
         else if(fileSysSat.system == SatelliteSystem::Galileo)
         {
            strm.timesystem = TimeSystem::GAL;
            firstObs.setTimeSystem(TimeSystem::GAL);
         }
         else if(fileSysSat.system == SatelliteSystem::QZSS)
         {
            strm.timesystem = TimeSystem::QZS;
            firstObs.setTimeSystem(TimeSystem::QZS);
         }
         else if(fileSysSat.system == SatelliteSystem::BeiDou)
         {
            strm.timesystem = TimeSystem::BDT;
            firstObs.setTimeSystem(TimeSystem::BDT);
         }
         else if(fileSysSat.system == SatelliteSystem::Mixed)
         {
            // lenient
            strm.timesystem = TimeSystem::GPS;
            firstObs.setTimeSystem(TimeSystem::GPS);
            // RINEX 3 requires this
            //FFStreamError e("TimeSystem in MIXED files must be given by first obs");
            //GPSTK_THROW(e);
         }
         else
         {
            FFStreamError e("Unknown file system type");
            GPSTK_THROW(e);
         }
      }

   } // end reallyGetRecord

      // This method maps v2.11 GPS observation types to the v3 equivalent.
      // Since only GPS and only v2.11 are of interest, only L1/L2/L5
      // are considered.
   vector<RinexObsID> Rinex3ObsHeader::mapR2ObsToR3Obs_G()
   {
      vector<RinexObsID> obsids;
       
         // Assume D1, S1, and L1 come from C/A unless P is being treated as Y and P1 is present
         // Furthermore, if P1 is present and P is NOT being treated as Y, assume that P1 
         // is some Z-mode or equivalent "smart" codeless process.
         //
         // Condition           Result
         // PisY   P1?
         //    N    Y     L1,D1,S1 considered C,  P1 becomes C1W
         //    N    N     L1,D1,S1 considered C
         //    Y    Y     L1,D1,S1 considered Y,  P1 becomes C1Y
         //    Y    N     L1,D1,S1 considered C  
         //
      bool hasL1P = find(R2ObsTypes.begin(),R2ObsTypes.end(), string("P1")) != R2ObsTypes.end();
      string code1 = "C";
      string code1P = "W";
      if (PisY && hasL1P)
      { 
         code1 = "Y";
         code1P = "Y";
      }
            
         // Assume D2, S2, and L2 come from Y if P is being treated as Y and P2 is present
         // codeless unless L2C is tracked.
         // If BOTH C2 and P2 are present, and P is NOT being treated as Y, assume C2 
         // is code tracking the open signal and that P2 is codelessly tracking an
         // authorized signal.
         //
         // Condition           Result
         // PisY   C2?   P2? 
         //    N    Y     N     L2,D2,S2 considered X,
         //    N    Y     Y     L2,D2,S2 considered W,  P2 becomes C2W**
         //    N    N     Y     L2,D2,S2 considered W,  P2 becomes C2W
         //    N    N     N     L2,D2,S2 considered X*
         //    Y    Y     N     L2,D2,S2 considered X
         //    Y    Y     Y     L2,D2,S2 considered Y,  P2 becomes C2Y
         //    Y    N     Y     L2,D2,S2 considered Y,  P2 becomes C2Y
         //    Y    N     N     L2,D2,S2 considered X*
         // * - Probably not a reasonable set of conditions.  It implies no L2 pseudoranges
         //     were collected on any tracking code.
         // **- Interesting case.  Currently presence of C2 in the header means
         //     that the data MAY be present.  However, since only some of the GPS
         //     SVs have L2C, the C2 data field will frequently be empty.
         //     Therefore, we'll go with "W" if P2 is present.  The other option
         //     would be to add smarts to the SV-by-SV record reading process to
         //     coerce this to X if there are actually data in the C2 field at
         //     the time the observations are read.  That would really do violence
         //     to the existing logic.  Better to hope for a transition to Rinex 3
         //     before this becomes a real issue. 
         // 
         // N.B.:  This logic (both for P1 and P2) assumes P is NEVER P.  If we want to allow for 
         // live sky (or simulator capture) P code, we'll have to add more logic
         // to differentate between PisY, PisW, and PisP. That will have to be 
         // "beyond RINEX v2.11" extra-special handling.
         //
      bool hasL2P = find(R2ObsTypes.begin(),R2ObsTypes.end(), string("P2")) != R2ObsTypes.end();
      bool hasL2C = find(R2ObsTypes.begin(),R2ObsTypes.end(), string("C2")) != R2ObsTypes.end();

      string code2 = "X";   // Correct condition as long as P2 is not in the list
      string code2P = "X";  // Condition is irrelvant unless P2 is in the list
      if (hasL2P)
      {
         if (PisY)
         {
            code2  = "Y";
            code2P = "Y";
         }
         else
         {
            code2 = "W";
            code2P = "W";
         }
      }
      string syss("G");
      for(size_t j=0; j<R2ObsTypes.size(); ++j)
      {
         string ot(R2ObsTypes[j]);
         string obsid(syss);

         if      (ot == "C1") obsid += "C1C";
         else if (ot == "P1") obsid += "C1" + code1P;
         else if (ot == "L1") obsid += "L1" + code1;
         else if (ot == "D1") obsid += "D1" + code1;
         else if (ot == "S1") obsid += "S1" + code1;
               
         else if (ot == "C2") obsid += "C2X";
         else if (ot == "P2") obsid += "C2" + code2P;
         else if (ot == "L2") obsid += "L2" + code2;
         else if (ot == "D2") obsid += "D2" + code2;
         else if (ot == "S2") obsid += "S2" + code2;
               
         else if (ot == "C5") obsid += "C5X";
         else if (ot == "L5") obsid += "L5X";
         else if (ot == "D5") obsid += "D5X";
         else if (ot == "S5") obsid += "S5X";

            // If the obs type isn't valid for GPS, skip it.
         else continue;
               
         try
         {
            RinexObsID OT(obsid, version);
            obsids.push_back(OT);
            mapSysR2toR3ObsID[syss][ot] = OT; //map<string, map<string, RinexObsID> >
         }
         catch(InvalidParameter& ip)
         {
            FFStreamError fse("InvalidParameter: "+ip.what());
            GPSTK_THROW(fse);
         }
      }
      return obsids;
   }

      // This method maps v2.11 GLONASS observation types to the v3 equivalent.
      // Since only GLONASS and only v2.11 are of interest, only L1/L2
      // are considered.
   vector<RinexObsID> Rinex3ObsHeader::mapR2ObsToR3Obs_R( )
   {
      vector<RinexObsID> obsids;
      
         // Assume D1, S1, and L1 come from C/A
         // This assumes that any files claiming to track GLONASS P1 is 
         // actually doing so with a codeless technique.  There is no RINEX V3
         // "C1W" for GLONASS, so we'll leave P1 as C1P as the closest approximation.
      bool hasL1P = find(R2ObsTypes.begin(),R2ObsTypes.end(), string("P1")) != R2ObsTypes.end();
      string code1 = "C";

         // Assume D2, S2, and L2 come from C/A.  Same logic as above.
      bool hasL2P = find(R2ObsTypes.begin(),R2ObsTypes.end(), string("P2")) != R2ObsTypes.end();
      string code2 = "C";

      string syss("R");     
      for(size_t j=0; j<R2ObsTypes.size(); ++j)
      {
         string ot(R2ObsTypes[j]);
         string obsid(syss);
         
         if      (ot == "C1") obsid += "C1C";
         else if (ot == "P1") obsid += "C1P";
         else if (ot == "L1") obsid += "L1" + code1;
         else if (ot == "D1") obsid += "D1" + code1;
         else if (ot == "S1") obsid += "S1" + code1;
            
         else if (ot == "C2") obsid += "C2C";
         else if (ot == "P2") obsid += "C2P";
         else if (ot == "L2") obsid += "L2" + code2;
         else if (ot == "D2") obsid += "D2" + code2;
         else if (ot == "S2") obsid += "S2" + code2;

            // If the obs type isn't valid for GLONASS, skip it.
         else continue;
         
         try
         {
            RinexObsID OT(obsid, version);
            obsids.push_back(OT);
            mapSysR2toR3ObsID[syss][ot] = OT; //map<string, map<string, RinexObsID> >
         }
         catch(InvalidParameter& ip)
         {
            FFStreamError fse("InvalidParameter: "+ip.what());
            GPSTK_THROW(fse);
         }
      }
      return obsids;
   }

      // This method maps v2.11 Galileo observation types to the v3 equivalent.
      // Since only Galileo and only v2.11 are of interest no L2 types
      // are considered.  Furthermore, Rinex v2.11 states that there is no
      // P for Galileo.  (Where that leaves the PRS is a good question.)
      //
      // In RINEX v3, there are 3-5 tracking codes defined for each carrier. 
      // Given the current lack of experience, the code makes some 
      // guesses on what the v2.11 translations should mean.   
   vector<RinexObsID> Rinex3ObsHeader::mapR2ObsToR3Obs_E()
   {
      vector<RinexObsID> obsids;

      string code1 = "B";  // Corresponds to the open service
      string code5 = "I";  // Corresponds to the open service
      string code7 = "X";  // Corresponds to I + Q tracking
      string code8 = "X";  // Corresponds to I + Q tracking
      string code6 = "X";  // Corresponds to B + C tracking

      string syss("E");     
      for(size_t j=0; j<R2ObsTypes.size(); ++j)
      {
         string ot(R2ObsTypes[j]);
         string obsid(syss);
         if      (ot == "C1") obsid += "C1" + code1;
         else if (ot == "L1") obsid += "L1" + code1;
         else if (ot == "D1") obsid += "D1" + code1;
         else if (ot == "S1") obsid += "S1" + code1;
               
         else if (ot == "C5") obsid += "C5" + code5;
         else if (ot == "L5") obsid += "L5" + code5;
         else if (ot == "D5") obsid += "D5" + code5;
         else if (ot == "S5") obsid += "S5" + code5;

         else if (ot == "C6") obsid += "C6" + code6;
         else if (ot == "L6") obsid += "L6" + code6;
         else if (ot == "D6") obsid += "D6" + code6;
         else if (ot == "S6") obsid += "S6" + code6;
               
         else if (ot == "C7") obsid += "C7" + code7;
         else if (ot == "L7") obsid += "L7" + code7;
         else if (ot == "D7") obsid += "D7" + code7;
         else if (ot == "S7") obsid += "S7" + code7;
               
         else if (ot == "C8") obsid += "C8" + code8;
         else if (ot == "L8") obsid += "L8" + code8;
         else if (ot == "D8") obsid += "D8" + code8;
         else if (ot == "S8") obsid += "S8" + code8;

            // If the obs type isn't valid for Galileo, skip it.
         else continue;
         
         try
         {
            RinexObsID OT(obsid, version);
            obsids.push_back(OT);
            mapSysR2toR3ObsID[syss][ot] = OT; //map<string, map<string, RinexObsID> >
         }
         catch(InvalidParameter& ip)
         {
            FFStreamError fse("InvalidParameter: "+ip.what());
            GPSTK_THROW(fse);
         }
      }
      return obsids;
   }


      // This method maps v2.11 SBAS observation types to the v3 equivalent.
      // Since only SBAS and only v2.11 are of interest only L1/L5
      // are considered.
   vector<RinexObsID> Rinex3ObsHeader::mapR2ObsToR3Obs_S()
   {
      vector<RinexObsID> obsids;

      string code1 = "C";  // Only option
      string code5 = "X";  // Corresponds to I + Q tracking

      string syss("S");     
      for(size_t j=0; j<R2ObsTypes.size(); ++j)
      {
         string ot(R2ObsTypes[j]);
         string obsid(syss);
         if      (ot == "C1") obsid += "C1" + code1;
         else if (ot == "L1") obsid += "L1" + code1;
         else if (ot == "D1") obsid += "D1" + code1;
         else if (ot == "S1") obsid += "S1" + code1;
               
         else if (ot == "C5") obsid += "C5" + code5;
         else if (ot == "L5") obsid += "L5" + code5;
         else if (ot == "D5") obsid += "D5" + code5;
         else if (ot == "S5") obsid += "S5" + code5;

            // If the obs type isn't valid for SBAS, skip it.
         else continue;
         
         try
         {
            RinexObsID OT(obsid, version);
            obsids.push_back(OT);
            mapSysR2toR3ObsID[syss][ot] = OT; //map<string, map<string, RinexObsID> >
         }
         catch(InvalidParameter& ip)
         {
            FFStreamError fse("InvalidParameter: "+ip.what());
            GPSTK_THROW(fse);
         }
      }
      return obsids;
   }


   CivilTime Rinex3ObsHeader::parseTime(const string& line) const
   {
      int year, month, day, hour, min;
      double sec;
      string tsys;
      TimeSystem ts;
   
      year  = asInt(   line.substr(0,   6));
      month = asInt(   line.substr(6,   6));
      day   = asInt(   line.substr(12,  6));
      hour  = asInt(   line.substr(18,  6));
      min   = asInt(   line.substr(24,  6));
      sec   = asDouble(line.substr(30, 13));
      tsys  =          line.substr(48,  3) ;

      ts = gpstk::StringUtils::asTimeSystem(tsys);

      return CivilTime(year, month, day, hour, min, sec, ts);
   } // end parseTime


   string Rinex3ObsHeader::writeTime(const CivilTime& civtime) const
   {
      using gpstk::StringUtils::asString;
      string line, tsStr(gpstk::StringUtils::asString(civtime.getTimeSystem()));

      line  = rightJustify(asString<short>(civtime.year    )   ,  6);
      line += rightJustify(asString<short>(civtime.month   )   ,  6);
      line += rightJustify(asString<short>(civtime.day     )   ,  6);
      line += rightJustify(asString<short>(civtime.hour    )   ,  6);
      line += rightJustify(asString<short>(civtime.minute  )   ,  6);
      line += rightJustify(asString(       civtime.second,7)   , 13);
      line += rightJustify(tsStr,  8);

      return line;
   } // end writeTime

      // Compute map of obs types for use in writing version 2 header
      // and data, call before writing
   void Rinex3ObsHeader::prepareVer2Write(void)
   {
      if(version > 3)
      {
         version = 2.11;
      }
      valid |= Rinex3ObsHeader::validWaveFact;
      if (valid.isSet(Rinex3ObsHeader::validSystemNumObs))
      {
            // change from RINEX 3 header to RINEX 2 equivalent
         valid.clear(Rinex3ObsHeader::validSystemNumObs);
         valid.set(Rinex3ObsHeader::validNumObs);
      }
         /// @todo unset R3-specific header members?
      
         // make a list of R2 obstype strings, and a map R3ObsIDs <=
         // R2 obstypes for each system
      R2ObsTypes.clear();
      map<string,vector<RinexObsID> >::const_iterator mit;
      for (mit = mapObsTypes.begin(); mit != mapObsTypes.end(); mit++)
      {
         string sysString = mit->first;
            // Compass/BeiDou and QZSS and IRNSS are unsupported by RINEX 2.11.
            // Transit was deprecated in 2.11.
            // Skip them.
         if ((sysString == "I") || (sysString == "J") || (sysString == "C") ||
             (sysString == "T"))
         {
            continue;
         }
            // 2.11 supports only GPS, GLONASS, Geo/SBAS and Galileo
         if(sysString!="G" && sysString!="R" && sysString!="E" &&
            sysString!="S")
         {
            FFStreamError er(
               "Invalid system char string in header.mapObsTypes: "+sysString);
            GPSTK_THROW(er);
         }
            // mit->first is system char as a 1-char string
         map<string, RinexObsID> mapR2toR3ObsID;

         vector<string> uniqueCheck;
            // loop over all ObsIDs for this system
         for(size_t i=0; i<mit->second.size(); i++)
         {
            string R2ot, lab(mit->second[i].asString(version));
               // the list of all tracking code characters for this sys, freq
            string allCodes(
               RinexObsID::validRinexTrackingCodes[mit->first[0]][lab[1]]);

            if (lab == string("C1C"))
               R2ot = string("C1");
            else if (lab == string("C2X") && mit->first == "G")
               R2ot = string("C2");
            else if (lab == string("C2C") && mit->first == "R")
               R2ot = string("C2");
               // R2 has C5 but not P5
            else if (lab.substr(0,2) == "C5")
               R2ot = string("C5");
            else if (lab[0] == 'C')
               R2ot = string("P")+string(1,lab[1]);
            else
               R2ot = lab.substr(0,2);
               // add to list, if not already there
            vector<string>::iterator it;
            it = find(R2ObsTypes.begin(),R2ObsTypes.end(),R2ot);
            if (it == R2ObsTypes.end())
            {
                  // its not there - add it
               R2ObsTypes.push_back(R2ot);
               mapR2toR3ObsID[R2ot] = mit->second[i];
            }
            else
            {
                  // its already there - in list of R2 ots
               if (mapR2toR3ObsID.find(R2ot) == mapR2toR3ObsID.end())
               {
                     // must also add to sys map
                  mapR2toR3ObsID[R2ot] = mit->second[i];
               }
               else
               {
                     // its already in sys map ...
                     // .. but is the new tc 'better'?
                  string::size_type posold,posnew;
                  posold = allCodes.find((mapR2toR3ObsID[R2ot].asString(version))[2]);
                  posnew = allCodes.find(lab[2]);
                  if(posnew < posold)
                  {
                        // replace the R3ObsID in the map
                     mapR2toR3ObsID[R2ot] = mit->second[i];
                  }
                  if (R2DisambiguityMap.find(sysString + R2ot) ==
                      R2DisambiguityMap.end())
                  {
                     R2DisambiguityMap.insert(
                        std::pair<string,string>(
                           sysString + R2ot,
                           mapR2toR3ObsID[R2ot].asString(version)));
                  }
                  else
                  {
                     R2DisambiguityMap[sysString + R2ot] = lab;
                  }
               }
            }
         }
            // save for this system
         mapSysR2toR3ObsID[mit->first] = mapR2toR3ObsID;
      }
   }  // end prepareVer2Write()

   void Rinex3ObsHeader::dump(ostream& s, double dumpVersion) const
   {
      using gpstk::StringUtils::asString;
      size_t i;

      string str;
      if(fileSysSat.system == SatelliteSystem::Mixed)
         str = "MIXED";
      else
      {
         RinexSatID sat(fileSysSat);
         str = sat.systemChar();
         str = str + " (" + sat.systemString() + ")";
      }

      s << "---------------------------------- REQUIRED "
        << "----------------------------------" << endl;
      s << "Rinex Version " << fixed << setw(5) << setprecision(2) << dumpVersion
        << ",  File type " << fileType << ",  System " << str << "." << endl;
      s << "Prgm: " << fileProgram << ",  Run: " << date
        << ",  By: " << fileAgency << endl;
      s << "Marker name: " << markerName << ", ";
      s << "Marker type: " << markerType << "." << endl;
      s << "Observer : " << observer << ",  Agency: " << agency << endl;
      s << "Rec#: " << recNo << ",  Type: " << recType
        << ",  Vers: " << recVers << endl;
      s << "Antenna # : " << antNo << ",  Type : " << antType << endl;
      s << "Position      (XYZ,m) : " << setprecision(4) << antennaPosition
        << "." << endl;
      s << "Antenna Delta (HEN,m) : " << setprecision(4) << antennaDeltaHEN
        << "." << endl;
      map<string,vector<RinexObsID> >::const_iterator iter;
      for(iter = mapObsTypes.begin(); iter != mapObsTypes.end(); iter++)
      {
         RinexSatID rsid;
         rsid.fromString(iter->first);
         s << rsid.systemString() << " Observation types ("
           << iter->second.size() << "):" << endl;
         for(i = 0; i < iter->second.size(); i++)
         {
            s << " Type #" << setw(2) << setfill('0') << i+1 << setfill(' ')
              << " (" << iter->second[i].asString(dumpVersion) << ") "
              << asString(static_cast<ObsID>(iter->second[i])) << endl;
         }
      }

      s << "R2ObsTypes: ";
      for (StringVec::const_iterator i=R2ObsTypes.begin(); i != R2ObsTypes.end(); i++)
         s << *i << " ";
      s << endl;
      for (VersionObsMap::const_iterator i = mapSysR2toR3ObsID.begin(); i != mapSysR2toR3ObsID.end(); i++)
      {
         s << "mapSysR2toR3ObsID[" << i->first << "] ";
         for (ObsIDMap::const_iterator j = i->second.begin(); j != i->second.end(); j++)
            s << j->first << ":" << j->second.asString(dumpVersion) << " ";
         s << endl;
      }
      
      s << "Time of first obs "
        << printTime(firstObs,"%04Y/%02m/%02d %02H:%02M:%06.3f %P") << endl;

      s << "(This header is ";
      if (Fields::isValid(dumpVersion, valid))
         s << "VALID)" << endl;
      else
      {
         s << "NOT VALID";
         s << " RINEX " << setprecision(2) << dumpVersion << ")" << endl;
         s << "valid    = " << valid << endl;
         Fields required = Fields::getRequired(dumpVersion);
         s << "allValid = " << required << endl;

         s << "Invalid or missing header records:" << endl;
            // print all header fields in required not in valid
         for (const auto& reqi : required.fieldsSet)
         {
            if (valid.isSet(reqi) == 0)
            {
               s << "  " << setw(2) << reqi << " "
                 << Rinex3ObsHeader::asString(reqi) << endl;
            }
         }
         s << "END Invalid header records." << endl;
      }

      s << "---------------------------------- OPTIONAL "
        << "----------------------------------" << endl;
      if(valid & validMarkerNumber     )
         s << "Marker number : " << markerNumber << endl;
      if(valid & validMarkerType       )
         s << "Marker type : " << markerType << endl;
      if(valid & validAntennaDeltaXYZ  )
         s << "Antenna Delta    (XYZ,m) : "
           << setprecision(4) << antennaDeltaXYZ   << endl;
      if(valid & validAntennaPhaseCtr  )
         s << "Antenna PhaseCtr (XYZ,m) : "
           << setprecision(4) << antennaPhaseCtr   << endl;
      if(valid & validAntennaBsightXYZ )
         s << "Antenna B.sight  (XYZ,m) : "
           << setprecision(4) << antennaBsightXYZ  << endl;
      if(valid & validAntennaZeroDirAzi)
         s << "Antenna ZeroDir  (deg)   : "
           << setprecision(4) << antennaZeroDirAzi << endl;
      if(valid & validAntennaZeroDirXYZ)
         s << "Antenna ZeroDir  (XYZ,m) : "
           << setprecision(4) << antennaZeroDirXYZ << endl;
      if(valid & validCenterOfMass     )
         s << "Center of Mass   (XYZ,m) : "
           << setprecision(4) << antennaPhaseCtr   << endl;
      if(valid & validSigStrengthUnit  )
         s << "Signal Strength Unit = " << sigStrengthUnit << endl;
      if(valid & validInterval         )
         s << "Interval = "
           << fixed << setw(7) << setprecision(3) << interval << endl;
      if(valid & validLastTime         )
         s << "Time of Last Obs "
           << printTime(lastObs,"%04Y/%02m/%02d %02H:%02M:%06.3f %P") << endl;
      if(valid & validReceiverOffset   )
         s << "Clock offset record is present and offsets "
           << (receiverOffset ? "ARE" : "are NOT") << " applied." << endl;
      if(dumpVersion < 3 && (valid & validWaveFact)) // TD extraWaveFactList
         s << "Wavelength factor L1: " << wavelengthFactor[0]
           << " L2: " << wavelengthFactor[1] << endl;
      if(valid & validSystemDCBSapplied)
      {
         for(i = 0; i < infoDCBS.size(); i++)
         {
            RinexSatID rsid;
            rsid.fromString(infoDCBS[i].satSys);
            s << "System DCBS Correction Applied to " << rsid.systemString()
              << " data using program " << infoDCBS[i].name << endl;
            s << " from source " << infoDCBS[i].source << "." << endl;
         }
      }
      if(valid & validSystemPCVSapplied)
      {
         for(i = 0; i < infoPCVS.size(); i++)
         {
            RinexSatID rsid;
            rsid.fromString(infoPCVS[i].satSys);
            s << "System PCVS Correction Applied to " << rsid.systemString()
              << " data using program " << infoPCVS[i].name << endl;
            s << " from source " << infoPCVS[i].source << "." << endl;
         }
      }
      if(valid & validSystemScaleFac   )
      {
         map<string, ScaleFacMap>::const_iterator mapIter;
            // loop over GNSSes
         for(mapIter = sysSfacMap.begin(); mapIter != sysSfacMap.end(); mapIter++)
         {
            RinexSatID rsid;
            rsid.fromString(mapIter->first);
            s << rsid.systemString() << " scale factors applied:" << endl;
            map<RinexObsID,int>::const_iterator iter;
               // loop over scale factor map
            for(iter = mapIter->second.begin(); iter != mapIter->second.end(); iter++)
               s << "   " << iter->first.asString(dumpVersion) << " " << iter->second << endl;
         }
      }
      if(valid & validSystemPhaseShift )
      {
         map<string, map<RinexObsID, map<RinexSatID,double> > >::const_iterator it;
         for(it=sysPhaseShift.begin(); it!=sysPhaseShift.end(); ++it)
         {
            string sys(it->first);
            map<RinexObsID, map<RinexSatID, double> >::const_iterator jt;
            jt = it->second.begin();
            if(jt == it->second.end())
               s << "Phase shift correction for system " << sys << " is empty." << endl;
            for( ; jt!=it->second.end(); ++jt)
            {
               map<RinexSatID,double>::const_iterator kt;
               for(kt=jt->second.begin(); kt!=jt->second.end(); ++kt)
                  s << "Phase shift correction for system " << sys << ": "
                    << fixed << setprecision(5)
                    << setw(8) << kt->second << " cycles applied to obs type "
                    << jt->first.asString(dumpVersion) << " "
                    << RinexSatID(sys).systemString() << endl;
            }
         }
      }
      if(valid & validGlonassSlotFreqNo)
      {
         int n(0);
         map<RinexSatID,int>::const_iterator it;
         s << "GLONASS frequency channels:\n";
         for(it=glonassFreqNo.begin(); it!=glonassFreqNo.end(); ++it)
         {
            s << " " << it->first.toString() << " " << setw(2) << it->second;
            if(++n > 1 && (n%8)==0) s << endl;
         }
         if((n%8) != 0) s << endl;
      }
      if(valid & validGlonassCodPhsBias)
      {
         map<RinexObsID,double>::const_iterator it;
         s << "GLONASS Code-phase biases:\n" << fixed << setprecision(3);
         for(it=glonassCodPhsBias.begin(); it!=glonassCodPhsBias.end(); ++it)
            s << " " << it->first.asString(dumpVersion) << " " << setw(8) << it->second;
         s << endl;
      }
      if(valid & validLeapSeconds)
         s << "Leap seconds: " << leapSeconds << endl;
      if(valid & validNumSats)
         s << "Number of Satellites with data : " << numSVs << endl;
      if(valid & validPrnObs)
      {
         RinexSatID sat, sys(-1,SatelliteSystem::Unknown);
         s << " PRN and number of observations for each obs type:" << endl;
         map<RinexSatID, vector<int> >::const_iterator it = numObsForSat.begin();
         while (it != numObsForSat.end())
         {
            sat = it->first;
            if(sat.system != sys.system)
            {
                  // print a header: SYS  OT  OT  OT ...
               s << " " << sat.systemString3() << " ";
               iter = mapObsTypes.find(string(1,sat.systemChar()));
               const vector<RinexObsID>& vec(iter->second);
               for(i=0; i<vec.size(); i++)
                  s << setw(7) << vec[i].asString(dumpVersion);
               s << endl;
               sys = sat;
            }
            vector<int> obsvec = it->second;
            s << " " << sat.toString() << " ";
            for(i = 0; i < obsvec.size(); i++)           // print the numbers of obss
               s << " " << setw(6) << obsvec[i];
            s << endl;
            it++;
         }
      }
      if(commentList.size())
      {
         if(!(valid & validComment)) s << " Comment list is NOT valid" << endl;
         s << "Comments (" << commentList.size() << ") :" << endl;
         for(i=0; i<commentList.size(); i++) s << commentList[i] << endl;
      }

      s << "-------------------------------- END OF HEADER "
        << "--------------------------------" << endl;
   } // end dump


      /* This method returns the numerical index of a given observation
       *
       * @param type String representing the observation type.
       */
   size_t Rinex3ObsHeader::getObsIndex( const string& type ) const
   {
      string newType(type);

         // 'old-style' type: Let's change it to 'new style'.
      if( newType.size() == 2 )
      {
         if( newType == "C1" ) newType = "C1C";
         else if( newType == "P1" ) newType = "C1P";
         else if( newType == "L1" ) newType = "L1P";
         else if( newType == "D1" ) newType = "D1P";
         else if( newType == "S1" ) newType = "S1P";
         else if( newType == "C2" ) newType = "C2C";
         else if( newType == "P2" ) newType = "C2P";
         else if( newType == "L2" ) newType = "L2P";
         else if( newType == "D2" ) newType = "D2P";
         else if( newType == "S2" ) newType = "S2P";
         else
         {
            InvalidRequest exc("Invalid type.");
            GPSTK_THROW(exc);
         }
      }

         // Add GNSS code. By default the system is GPS
      if( newType.size() == 3 )
      {
         newType = "G" + newType;
      }

         // Check if resulting 'newType' is valid
      if( !isValidRinexObsID(newType, _bIsPossiblySqm) )
      {
         InvalidRequest ir(newType + " is not a valid RinexObsID!.");
         GPSTK_THROW(ir);
      }

         // Extract the GNSS from the newType
      string sys( newType, 0, 1 );
      return getObsIndex(sys, RinexObsID(newType, version, _bIsPossiblySqm));
   }

   
   size_t Rinex3ObsHeader::getObsIndex(const string& sys,
                                       const RinexObsID& obsID ) const
   {
         /// typedef std::vector<RinexObsID> RinexObsVec;
         /// typedef std::map<std::string, RinexObsVec> RinexObsMap;
         /// RinexObsMap mapObsTypes;         ///< SYS / # / OBS TYPES

         // find the GNSS in the map
      RinexObsMap remapped;
      std::map<std::string,unsigned> obsCount;
      remapObsTypes(remapped, obsCount);
      RinexObsMap::const_iterator it = remapped.find(sys);

      if (it == remapped.end())
      {
         InvalidRequest ir("GNSS system " + sys + " not stored.");
         GPSTK_THROW(ir);
      }

      const RinexObsVec& rov = it->second;
      for (size_t i=0; i<rov.size(); i++)
      {
         if (rov[i].equalIndex(obsID))
            return i;
      }
      
      InvalidRequest ir(obsID.asString(version) + " is not stored in system " + sys + ".");
      GPSTK_THROW(ir);
      return 0;
   }


   bool Rinex3ObsHeader::compare(const Rinex3ObsHeader& right,
                                 std::vector<std::string>& diffs,
                                 const std::vector<std::string>& inclExclList,
                                 bool incl)
   {
         // map header token to comparison result
      std::map<std::string,bool> lineMap;
      std::map<std::string,bool>::const_iterator lmi;
         // Put the comments in a sorted set, we don't really care
         // about the ordering.
      std::set<std::string>
         lcomments(commentList.begin(), commentList.end()),
         rcomments(right.commentList.begin(), right.commentList.end());
      std::set<RinexObsID>
         lobs(obsTypeList.begin(), obsTypeList.end()),
         robs(right.obsTypeList.begin(), right.obsTypeList.end());
         // Compare everything first...
         // deliberately ignoring valid flags

         // only comparing first character of file type because that's
         // all that matters according to RINEX
      lineMap[hsVersion] =
         ((version == right.version) &&
          (fileType[0] == right.fileType[0]) &&
          (fileSysSat.system == right.fileSysSat.system));
      lineMap[hsRunBy] =
         ((fileProgram == right.fileProgram) &&
          (fileAgency == right.fileAgency) &&
          (date == right.date));
      lineMap[hsComment] = (lcomments == rcomments);
      lineMap[hsMarkerName] = (markerName == right.markerName);
      lineMap[hsMarkerNumber] = (markerNumber == right.markerNumber);
      lineMap[hsMarkerType] = (markerType == right.markerType);
      lineMap[hsObserver] =
         ((observer == right.observer) &&
          (agency == right.agency));
      lineMap[hsReceiver] =
         ((recNo == right.recNo) &&
          (recType == right.recType) &&
          (recVers == right.recVers));
      lineMap[hsAntennaType] =
         ((antNo == right.antNo) &&
          (antType == right.antType));
      lineMap[hsAntennaPosition] =
         (antennaPosition == right.antennaPosition);
      lineMap[hsAntennaDeltaHEN] =
         (antennaDeltaHEN == right.antennaDeltaHEN);
      lineMap[hsAntennaDeltaXYZ] =
         (antennaDeltaXYZ == right.antennaDeltaXYZ);
      lineMap[hsAntennaPhaseCtr] =
         (antennaPhaseCtr == right.antennaPhaseCtr);
      lineMap[hsAntennaBsightXYZ] =
         (antennaBsightXYZ == right.antennaBsightXYZ);
      lineMap[hsAntennaZeroDirAzi] =
         (antennaZeroDirAzi == right.antennaZeroDirAzi);
      lineMap[hsAntennaZeroDirXYZ] =
         (antennaZeroDirXYZ == right.antennaZeroDirXYZ);
      lineMap[hsCenterOfMass] = (centerOfMass == right.centerOfMass);
      lineMap[hsNumObs] = (lobs == robs);
      lineMap[hsSystemNumObs] = true;
      lineMap[hsWaveFact] =
         (memcmp(wavelengthFactor, right.wavelengthFactor,
                 sizeof(wavelengthFactor)) == 0);
      lineMap[hsSigStrengthUnit] =
         (sigStrengthUnit == right.sigStrengthUnit);
      lineMap[hsInterval] = (interval == right.interval);
      lineMap[hsFirstTime] = (firstObs == right.firstObs);
      lineMap[hsLastTime] = (lastObs == right.lastObs);
      lineMap[hsReceiverOffset] = (receiverOffset == right.receiverOffset);
      lineMap[hsSystemDCBSapplied] = true;
      lineMap[hsSystemPCVSapplied] = true;
      lineMap[hsSystemScaleFac] = true;
      lineMap[hsSystemPhaseShift] = true;
      lineMap[hsGlonassSlotFreqNo] = true;
      lineMap[hsGlonassCodPhsBias] = true;
      lineMap[hsLeapSeconds] = (leapSeconds == right.leapSeconds);
      lineMap[hsNumSats] = (numSVs == right.numSVs);
      lineMap[hsPrnObs] = true;
         // ...then filter by inclExclList later
      if (incl)
      {
         std::map<std::string,bool> oldLineMap(lineMap);
         std::map<std::string,bool>::const_iterator olmi;
         lineMap.clear();
         for (unsigned i = 0; i < inclExclList.size(); i++)
         {
            if ((olmi = oldLineMap.find(inclExclList[i])) != oldLineMap.end())
            {
               lineMap[olmi->first] = olmi->second;
            }
         }
      }
      else
      {
            // exclude, remove items in inclExclList
         for (unsigned i = 0; i < inclExclList.size(); i++)
         {
            lineMap.erase(inclExclList[i]);
         }
      }
         // check the equality of the final remaining set of header lines
      bool rv = true;
      for (lmi = lineMap.begin(); lmi != lineMap.end(); lmi++)
      {
         if (!lmi->second)
         {
            diffs.push_back(lmi->first);
            rv = false;
         }
      }
      return rv;
   } // bool Rinex3ObsHeader::compare


   Rinex3ObsHeader::Fields Rinex3ObsHeader::Fields ::
   getRequired(double version)
   {
      if (version < 3.00)     return allValid2;
      else if(version < 3.01) return allValid30;
      else if(version < 3.02) return allValid301;
      else if(version < 3.03) return allValid302;
      else if(version < 3.04) return allValid303;
      else if(version < 3.05) return allValid303;
      return Fields();
   }


   Rinex3ObsHeader::Fields Rinex3ObsHeader::Fields ::
   operator&(const Rinex3ObsHeader::Fields& rhs) const
   {
      FieldSet results;
      set_intersection(fieldsSet.begin(), fieldsSet.end(),
                       rhs.fieldsSet.begin(), rhs.fieldsSet.end(),
                       inserter(results, results.begin()));
      return Rinex3ObsHeader::Fields(results);
   }

   Rinex3ObsHeader::Fields Rinex3ObsHeader::Fields ::
   operator|(const Rinex3ObsHeader::Fields& rhs) const
   {
      FieldSet results;
      set_union(fieldsSet.begin(), fieldsSet.end(),
                rhs.fieldsSet.begin(), rhs.fieldsSet.end(),
                inserter(results, results.begin()));
       return Rinex3ObsHeader::Fields(results);
   }


   Rinex3ObsHeader::Field Rinex3ObsHeader::Fields ::
   operator&(Rinex3ObsHeader::Field rhs) const
   {
      if (fieldsSet.count(rhs))
         return rhs;
      return validInvalid;
   }


   bool Rinex3ObsHeader::Fields ::
   isValid(const Rinex3ObsHeader::Fields& present) const
   {
      FieldSet results;
      set_difference(fieldsSet.begin(), fieldsSet.end(),
                     present.fieldsSet.begin(), present.fieldsSet.end(),
                     inserter(results, results.begin()));
      return results.empty();
   }


   void Rinex3ObsHeader::Fields ::
   describeMissing(const Rinex3ObsHeader::Fields& valid,
                   Exception& exc)
   {
      for (const auto& f : fieldsSet)
      {
         if (!valid.isSet(f))
         {
            exc.addText("Missing required header field: " + asString(f));
         }
      }
   }


   std::string Rinex3ObsHeader ::
   asString(Rinex3ObsHeader::Field b)
   {
      switch (b)
      {
         case validVersion:           return hsVersion;
         case validRunBy:             return hsRunBy;
         case validComment:           return hsComment;
         case validMarkerName:        return hsMarkerName;
         case validMarkerNumber:      return hsMarkerNumber;
         case validMarkerType:        return hsMarkerType;
         case validObserver:          return hsObserver;
         case validReceiver:          return hsReceiver;
         case validAntennaType:       return hsAntennaType;
         case validAntennaPosition:   return hsAntennaPosition;
         case validAntennaDeltaHEN:   return hsAntennaDeltaHEN;
         case validAntennaDeltaXYZ:   return hsAntennaDeltaXYZ;
         case validAntennaPhaseCtr:   return hsAntennaPhaseCtr;
         case validAntennaBsightXYZ:  return hsAntennaBsightXYZ;
         case validAntennaZeroDirAzi: return hsAntennaZeroDirAzi;
         case validAntennaZeroDirXYZ: return hsAntennaZeroDirXYZ;
         case validCenterOfMass:      return hsCenterOfMass;
         case validNumObs:            return hsNumObs;
         case validSystemNumObs:      return hsSystemNumObs;
         case validWaveFact:          return hsWaveFact;
         case validSigStrengthUnit:   return hsSigStrengthUnit;
         case validInterval:          return hsInterval;
         case validFirstTime:         return hsFirstTime;
         case validLastTime:          return hsLastTime;
         case validReceiverOffset:    return hsReceiverOffset;
         case validSystemDCBSapplied: return hsSystemDCBSapplied;
         case validSystemPCVSapplied: return hsSystemPCVSapplied;
         case validSystemScaleFac:    return hsSystemScaleFac;
         case validSystemPhaseShift:  return hsSystemPhaseShift;
         case validGlonassSlotFreqNo: return hsGlonassSlotFreqNo;
         case validGlonassCodPhsBias: return hsGlonassCodPhsBias;
         case validLeapSeconds:       return hsLeapSeconds;
         case validNumSats:           return hsNumSats;
         case validPrnObs:            return hsPrnObs;
         default: return "???";
      }
   }


   Rinex3ObsHeader::Field Rinex3ObsHeader ::
   asField(const std::string& s)
   {
      if (s == hsVersion)           return validVersion;
      if (s == hsRunBy)             return validRunBy;
      if (s == hsComment)           return validComment;
      if (s == hsMarkerName)        return validMarkerName;
      if (s == hsMarkerNumber)      return validMarkerNumber;
      if (s == hsMarkerType)        return validMarkerType;
      if (s == hsObserver)          return validObserver;
      if (s == hsReceiver)          return validReceiver;
      if (s == hsAntennaType)       return validAntennaType;
      if (s == hsAntennaPosition)   return validAntennaPosition;
      if (s == hsAntennaDeltaHEN)   return validAntennaDeltaHEN;
      if (s == hsAntennaDeltaXYZ)   return validAntennaDeltaXYZ;
      if (s == hsAntennaPhaseCtr)   return validAntennaPhaseCtr;
      if (s == hsAntennaBsightXYZ)  return validAntennaBsightXYZ;
      if (s == hsAntennaZeroDirAzi) return validAntennaZeroDirAzi;
      if (s == hsAntennaZeroDirXYZ) return validAntennaZeroDirXYZ;
      if (s == hsCenterOfMass)      return validCenterOfMass;
      if (s == hsNumObs)            return validNumObs;
      if (s == hsSystemNumObs)      return validSystemNumObs;
      if (s == hsWaveFact)          return validWaveFact;
      if (s == hsSigStrengthUnit)   return validSigStrengthUnit;
      if (s == hsInterval)          return validInterval;
      if (s == hsFirstTime)         return validFirstTime;
      if (s == hsLastTime)          return validLastTime;
      if (s == hsReceiverOffset)    return validReceiverOffset;
      if (s == hsSystemDCBSapplied) return validSystemDCBSapplied;
      if (s == hsSystemPCVSapplied) return validSystemPCVSapplied;
      if (s == hsSystemScaleFac)    return validSystemScaleFac;
      if (s == hsSystemPhaseShift)  return validSystemPhaseShift;
      if (s == hsGlonassSlotFreqNo) return validGlonassSlotFreqNo;
      if (s == hsGlonassCodPhsBias) return validGlonassCodPhsBias;
      if (s == hsLeapSeconds)       return validLeapSeconds;
      if (s == hsNumSats)           return validNumSats;
      if (s == hsPrnObs)            return validPrnObs;
      return validInvalid;
   }

   std::ostream& operator<<(std::ostream& s, const Rinex3ObsHeader::Fields& v)
   {
      Rinex3ObsHeader::FieldSet::const_iterator i;
      for (i = v.fieldsSet.begin(); i != v.fieldsSet.end(); i++)
      {
         if (i != v.fieldsSet.begin())
            s << ",";
         s << *i;
      }
      return s;
   }


   void Rinex3ObsHeader ::
   remapObsTypes(RinexObsMap& remapped, map<string,unsigned>& obsCount)
      const
   {
      for (const auto& mapIter : mapObsTypes)
      {
            // I and X pseudo-observables are special cases, and can
            // only be listed once (or once per band in the case of
            // the ionospheric delay) in any sane manner.
         bool addedChannel = false;
         std::set<CarrierBand> addedIono;
         for(size_t i = 0; i < mapIter.second.size(); i++)
         {
            if (mapIter.second[i].type == ObservationType::Iono)
            {
               if (addedIono.count(mapIter.second[i].band) > 0)
                  continue; // only write this pseudo-obs once
               addedIono.insert(mapIter.second[i].band);
            }
            else if (mapIter.second[i].type == ObservationType::Channel)
            {
               if (addedChannel)
                  continue; // only write this pseudo-obs once
               addedChannel = true;
            }
            remapped[mapIter.first].push_back(mapIter.second[i]);
            obsCount[mapIter.first]++;
         }
      }
   }

} // namespace gpstk
