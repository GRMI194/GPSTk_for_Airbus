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
 * @file RinexMetHeader.cpp
 * Encapsulates header of RINEX 2 & 3 Met file data, including I/O.
 */

#include <algorithm>       // for find
#include <set>

#include "StringUtils.hpp"
#include "SystemTime.hpp"
#include "CivilTime.hpp"
#include "RinexMetHeader.hpp"
#include "RinexMetStream.hpp"

using namespace gpstk::StringUtils;
using namespace std;


namespace gpstk
{
   const int RinexMetHeader::maxObsPerLine = 9;

   const string RinexMetHeader::stringVersion      = "RINEX VERSION / TYPE";
   const string RinexMetHeader::stringRunBy        = "PGM / RUN BY / DATE";
   const string RinexMetHeader::stringComment      = "COMMENT";
   const string RinexMetHeader::stringMarkerName   = "MARKER NAME";
   const string RinexMetHeader::stringMarkerNumber = "MARKER NUMBER";
   const string RinexMetHeader::stringObsType      = "# / TYPES OF OBSERV";
   const string RinexMetHeader::stringSensorType   = "SENSOR MOD/TYPE/ACC";
   const string RinexMetHeader::stringSensorPos    = "SENSOR POS XYZ/H";
   const string RinexMetHeader::stringEoH          = "END OF HEADER";

   std::string RinexMetHeader::bitString(unsigned long vb, char quote,
                                         std::string sep)
   {
      unsigned long b = 1;
      std::string rv;
      while (b)
      {
         if (vb & b)
         {
            if (rv.length())
               rv += sep;
            if (quote)
               rv += quote + bitsAsString((validBits)b) + quote;
            else
               rv += bitsAsString((validBits)b);
         }
         b <<= 1;
      }
      return rv;
   }

   void RinexMetHeader::reallyPutRecord(FFStream& ffs) const
   {
      RinexMetStream& strm = dynamic_cast<RinexMetStream&>(ffs);

         // Since they want to output this header, let's store it
         // internally for use by the data.
      strm.header = (*this);

      unsigned long allValid;
      if      (version == 2.0 ) allValid = allValid20;
      else if (version == 2.1 ) allValid = allValid21;
      else if (version == 2.11) allValid = allValid211;
      else if (version >= 3.0 ) allValid = allValid211; /// This is the R3.0 version standard.
      else
      {
         FFStreamError err("Unknown RINEX version: " + asString(version,2));
         err.addText("Make sure to set the version correctly.");
         GPSTK_THROW(err);
      }

      if ((valid & allValid) != allValid)
      {
         string errstr("Incomplete or invalid header: missing: ");
         errstr += bitString(allValid & ~valid);
         FFStreamError err(errstr);
         err.addText("Make sure you set all header valid bits for all of the available data.");
         GPSTK_THROW(err);
      }

      string line;

      if (valid & validVersion)
      {
         line  = rightJustify(asString(version,2), 9);
         line += string(11, ' ');
         line += leftJustify(fileType, 40);
         line += stringVersion;
         strm << line << endl;
         strm.lineNumber++;
      }
      if (valid & validRunBy)
      {
         line  = leftJustify(fileProgram,20);
         line += leftJustify(fileAgency,20);
         SystemTime sysTime;
         string curDate;
         if(version < 3)
         {
            curDate = (static_cast<CivilTime>(sysTime)).printf("%02m/%02d/%04Y %02H:%02M:%02S");
         }
         else
         {
            curDate = (static_cast<CivilTime>(sysTime)).printf("%04Y%02m%02d %02H%02M%02S %P");
         }
         line += leftJustify(curDate, 20);
         line += stringRunBy;
         strm << line << endl;
         strm.lineNumber++;
      }
      if (valid & validComment)
      {
         vector<string>::const_iterator itr = commentList.begin();
         while (itr != commentList.end())
         {
            line  = leftJustify((*itr), 60);
            line += stringComment;
            strm << line << endl;
            strm.lineNumber++;
            itr++;
         }
      }
      if (valid & validMarkerName)
      {
         line  = leftJustify(markerName, 60);
         line += stringMarkerName;
         strm << line << endl;
         strm.lineNumber++;
      }
      if (valid & validMarkerNumber)
      {
         line  = leftJustify(markerNumber, 60);
         line += stringMarkerNumber;
         strm << line << endl;
         strm.lineNumber++;
      }
      if (valid & validObsType)
      {
         line  = rightJustify(asString(obsTypeList.size()),6);
         vector<RinexMetType>::const_iterator itr = obsTypeList.begin();
         size_t numWritten = 0;
         while (itr != obsTypeList.end())
         {
            numWritten++;
               // continuation lines
            if ((numWritten % (maxObsPerLine+1)) == 0)
            {
               line += stringObsType;
               strm << line << endl;
               strm.lineNumber++;
               line = string(6,' ');
            }
            line += rightJustify(convertObsType(*itr), 6);
            itr++;
         }
            // pad the line out to 60 chrs and add label
         line += string(60 - line.size(), ' ');
         line += stringObsType;
         strm << line << endl;
         strm.lineNumber++;
      }
      if (valid & validSensorType)
      {
            // Write out only the sensor types that are in the obsTypeList.
         vector<sensorType>::const_iterator itr = sensorTypeList.begin();
         while (itr != sensorTypeList.end())
         {
            if (std::find(obsTypeList.begin(), obsTypeList.end(),
                          (*itr).obsType) != obsTypeList.end())
            {
               line  = leftJustify((*itr).model, 20);
               line += leftJustify((*itr).type, 20);
               line += string(6, ' ');
               line += rightJustify(asString((*itr).accuracy,1),7);
               line += string(4, ' ');
               line += convertObsType((*itr).obsType);
               line += string(1, ' ');
               line += stringSensorType;
               strm << line << endl;
               strm.lineNumber++;
            }
            itr++;
         }
      }
      if (valid & validSensorPos)
      {
            // Write out only the sensor positions that are in the obsTypeList.
         vector<sensorPosType>::const_iterator itr = sensorPosList.begin();
         while (itr != sensorPosList.end())
         {
            if (std::find(obsTypeList.begin(), obsTypeList.end(),
                          (*itr).obsType) != obsTypeList.end())
            {
               line  = rightJustify(asString((*itr).position[0],4),14);
               line += rightJustify(asString((*itr).position[1],4),14);
               line += rightJustify(asString((*itr).position[2],4),14);
               line += rightJustify(asString((*itr).height,4),14);
               line += string(1, ' ');
               line += convertObsType((*itr).obsType);
               line += string(1, ' ');
               line += stringSensorPos;
               strm << line << endl;
               strm.lineNumber++;
            }
            itr++;
         }
      }
      if (valid & validEoH)
      {
         line  = string(60, ' ');
         line += stringEoH;
         strm << line << endl;
         strm.lineNumber++;
      }
   }

   void RinexMetHeader::reallyGetRecord(FFStream& ffs) 
   {
      RinexMetStream& strm = dynamic_cast<RinexMetStream&>(ffs);

         // if already read, just return
      if (strm.headerRead == true)
         return;

      valid = 0;

         // Clear out structures in case the last read was a partial header.
      commentList.clear();
      obsTypeList.clear();
      sensorTypeList.clear();
      sensorPosList.clear();

      int numObs = 0;

      while (!(valid & validEoH))
      {
         string line;
         strm.formattedGetLine(line);

         if (line.length()<60 || line.length()>81)
         {
            FFStreamError e("Bad line length");
            GPSTK_THROW(e);
         }

         string thisLabel = strip(line.substr(60,20));

         if (thisLabel == stringVersion)
         {
            std::string verstr(strip(line.substr(0,20)));
            if ((verstr != "2.0") && (verstr != "2.1") && (verstr != "2.00") &&
               (verstr != "2.10") && (verstr != "2.11") && (verstr != "3.0") &&
               (verstr != "3.01") && (verstr!= "3.02"))
            {
               FFStreamError e("Unknown or unsupported RINEX version " + 
                               asString(version));
               GPSTK_THROW(e);
            }
            version = asDouble(verstr);
            fileType = strip(line.substr(20,20));
            if ( (fileType[0] != 'M') && (fileType[0] != 'm'))
            {
               FFStreamError e("This isn't a Rinex Met file");
               GPSTK_THROW(e);
            }
            valid |= validVersion;
         }
         else if (thisLabel == stringRunBy)
         {
            fileProgram = strip(line.substr(0,20));
            fileAgency = strip(line.substr(20,20));
            date = strip(line.substr(40,20));
            valid |= validRunBy;
         }
         else if (thisLabel == stringComment)
         {
            commentList.push_back(strip(line.substr(0,60)));
            valid |= validComment;
         }
         else if (thisLabel == stringMarkerName)
         {
            markerName = strip(line.substr(0,60));
            valid |= validMarkerName;
         }
         else if (thisLabel == stringMarkerNumber)
         {
            markerNumber = strip(line.substr(0,20));
            valid |= validMarkerNumber;
         }
         else if (thisLabel == stringObsType)
         {
               // read the first line
            if (!(valid & validObsType))
            {
               numObs = gpstk::StringUtils::asInt(line.substr(0,6));
               for (int i = 0; (i < numObs) && (i < maxObsPerLine); i++)
               {
                  int currPos = i * 6 + 6;
                  if (line.substr(currPos, 4) != string(4, ' '))
                  {
                     FFStreamError e("Format error for line type " + stringObsType);
                     GPSTK_THROW(e);
                  }
                  obsTypeList.push_back(convertObsType(line.substr(currPos + 4, 2)));
               }
               valid |= validObsType;
            }
               // read continuation lines
            else
            {
               int currentObsTypes = obsTypeList.size();
               for (int i = currentObsTypes; 
                    (i < numObs) && (i < (maxObsPerLine + currentObsTypes));
                    i++)
               {
                  int currPos = (i % maxObsPerLine) * 6 + 6;
                  if (line.substr(currPos, 4) != string(4,' '))
                  {
                     FFStreamError e("Format error for line type " + stringObsType);
                     GPSTK_THROW(e);
                  }
                  obsTypeList.push_back(convertObsType(line.substr(currPos + 4, 2)));
               }
            }
         }
         else if (thisLabel == stringSensorType)
         {
            if (line.substr(40,6) != string(6, ' '))
            {
               FFStreamError e("Format error for line type " + stringSensorType);
               GPSTK_THROW(e);
            }
            sensorType st;
            st.model = strip(line.substr(0,20));
            st.type = strip(line.substr(20,20));
            st.accuracy = asDouble(line.substr(46,9));
            st.obsType = convertObsType(line.substr(57,2));

            sensorTypeList.push_back(st);

               // only set this valid if there are exactly
               // the same number in both lists
            if (sensorTypeList.size() == obsTypeList.size())
            {
               valid |= validSensorType;
            }
            else
            {
               valid &= ~(long)validSensorType;
            }
         }
         else if (thisLabel == stringSensorPos)
         {
               // read XYZ and H and obs type
            sensorPosType sp;
            sp.position[0] = asDouble(line.substr(0,14));
            sp.position[1] = asDouble(line.substr(14,14));
            sp.position[2] = asDouble(line.substr(28,14));
            sp.height = asDouble(line.substr(42,14));
            
            sp.obsType = convertObsType(line.substr(57,2));
            
            sensorPosList.push_back(sp);
            
               // Only barometer is required, so set it valid only if you see that record.
            if (sp.obsType == PR)
            {
               valid |= validSensorPos;
            }
         }
         else if (thisLabel == stringEoH)
         {
            valid |= validEoH;
         }
         else
         {
            FFStreamError e("Unknown header label " + thisLabel);
            GPSTK_THROW(e);
         }
      }

      unsigned long allValid;
      if      (version == 2.0 ) allValid = allValid20;
      else if (version == 2.1 ) allValid = allValid21;
      else if (version == 2.11) allValid = allValid21;
      else if (version >= 3.0 ) allValid = allValid21;
      else
      {
         FFStreamError e("Unknown or unsupported RINEX version " + 
                         asString(version));
         GPSTK_THROW(e);
      }

      if ( (allValid & valid) != allValid)
      {
         string errstr("Incomplete or invalid header: missing: ");
         errstr += bitString(allValid & ~valid);
         FFStreamError err(errstr);
         GPSTK_THROW(err);
      }

      strm.header = *this;
      strm.headerRead = true;
   }

   void RinexMetHeader::dump(ostream& s) const
   {
      s << "Marker " << markerName << endl;

      if (!obsTypeList.empty())
      {
         s << "Obs types:" << endl;
         vector<RinexMetType>::const_iterator itr = obsTypeList.begin();
         while (itr != obsTypeList.end())
         {
            s << convertObsType(*itr) << " ";
            itr++;
         }
         s << endl;
      }
   }

   RinexMetHeader::RinexMetType
   RinexMetHeader::convertObsType(const string& oneObs)
   {
      if      (oneObs == "PR") return PR;
      else if (oneObs == "TD") return TD;
      else if (oneObs == "HR") return HR;
      else if (oneObs == "ZW") return ZW;
      else if (oneObs == "ZD") return ZD;
      else if (oneObs == "ZT") return ZT;
      else if (oneObs == "WD") return WD;
      else if (oneObs == "WS") return WS;
      else if (oneObs == "RI") return RI;
      else if (oneObs == "HI") return HI;
      else
      {
         FFStreamError e("Bad obs type:" + oneObs);
         GPSTK_THROW(e);
      }
   }

   string RinexMetHeader::convertObsType(const RinexMetHeader::RinexMetType& oneObs)
   {
      if      (oneObs == PR) return "PR";
      else if (oneObs == TD) return "TD";
      else if (oneObs == HR) return "HR";
      else if (oneObs == ZW) return "ZW";
      else if (oneObs == ZD) return "ZD";
      else if (oneObs == ZT) return "ZT";
      else if (oneObs == WD) return "WD";
      else if (oneObs == WS) return "WS";
      else if (oneObs == RI) return "RI";
      else if (oneObs == HI) return "HI";
      else
      {
         FFStreamError e("Bad obs type:" + asString(oneObs));
         GPSTK_THROW(e);
      }
   }


   bool RinexMetHeader::compare(const RinexMetHeader& right,
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
      std::set<RinexMetType>
         lobs(obsTypeList.begin(), obsTypeList.end()),
         robs(right.obsTypeList.begin(), right.obsTypeList.end());
      std::set<sensorType>
         ltype(sensorTypeList.begin(), sensorTypeList.end()),
         rtype(right.sensorTypeList.begin(), right.sensorTypeList.end());
      std::set<sensorPosType>
         lpos(sensorPosList.begin(), sensorPosList.end()),
         rpos(right.sensorPosList.begin(), right.sensorPosList.end());
         // Compare everything first...
         // deliberately ignoring valid flags

         // only comparing first character of file type because that's
         // all that matters according to RINEX
      lineMap[stringVersion] =
         ((version == right.version) &&
          (fileType[0] == right.fileType[0]));
      lineMap[stringRunBy] =
         ((fileProgram == right.fileProgram) &&
          (fileAgency == right.fileAgency) &&
          (date == right.date));
      lineMap[stringComment] = (lcomments == rcomments);
      lineMap[stringMarkerName] = (markerName == right.markerName);
      lineMap[stringMarkerNumber] = (markerNumber == right.markerNumber);
      lineMap[stringObsType] = (lobs == robs);
      lineMap[stringSensorType] = (ltype == rtype);
      lineMap[stringSensorPos] = (lpos == rpos);
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
   } // bool RinexMetHeader::compare

   bool RinexMetHeader::sensorType::
   operator==(const gpstk::RinexMetHeader::sensorType& r)
      const
   {
      return ((obsType == r.obsType) &&
              (model == r.model) &&
              (type == r.type) &&
              (fabs(accuracy - r.accuracy) < 0.001));
   }

   bool RinexMetHeader::sensorType::
   operator<(const gpstk::RinexMetHeader::sensorType& r)
      const
   {
      if (obsType < r.obsType) return true;
      if (obsType > r.obsType) return false;
      if (model < r.model) return true;
      if (model > r.model) return false;
      if (type < r.type) return true;
      if (type > r.type) return false;
      if (accuracy < r.accuracy) return true;
      return false;
   }


   bool RinexMetHeader::sensorPosType::
   operator==(const gpstk::RinexMetHeader::sensorPosType& r)
      const
   {
      return ((obsType == r.obsType) &&
              (position == r.position) &&
              (fabs(height - r.height) < 0.001));
   }

      // pretty arbitrary sorting order, just need it to be consistent.
   bool RinexMetHeader::sensorPosType::
   operator<(const gpstk::RinexMetHeader::sensorPosType& r)
      const
   {
      if (obsType < r.obsType) return true;
      if (obsType > r.obsType) return false;
      if (position[0] < r.position[0]) return true;
      if (position[0] > r.position[0]) return false;
      if (position[1] < r.position[1]) return true;
      if (position[1] > r.position[1]) return false;
      if (position[2] < r.position[2]) return true;
      if (position[2] > r.position[2]) return false;
      if (height < r.height) return true;
      return false;
   }
} // namespace
