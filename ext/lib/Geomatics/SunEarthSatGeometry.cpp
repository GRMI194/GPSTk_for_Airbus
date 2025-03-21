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

/// @file SunEarthSatGeometry.cpp
/// Module containing various routines related to Sun-Earth-Satellite geometry,
/// including satellite attitude, XYZ->UEN rotation, and (elevation,azimuth) as
/// seen at the satellite. Used by PhaseWindup and PreciseRange.
/// NB Use class SolarSystem or SolarPosition() to get Sun's position as needed.

// system
#include <map>
// GPSTk includes
#include "GPSEllipsoid.hpp"         // angVelocity
// geomatics
#include "SunEarthSatGeometry.hpp"

using namespace std;

namespace gpstk
{

   // --------------------------------------------------------------------------------
   // Same as UpEastNorth(), but with rows re-ordered.
   Matrix<double> NorthEastUp(Position& P, bool geocentric)
   {
      try {
         Matrix<double> R(3,3);
         P.transformTo(geocentric ? Position::Geocentric : Position::Geodetic);
   
         double lat = (geocentric ?  P.getGeocentricLatitude()
                                  : P.getGeodeticLatitude()) * DEG_TO_RAD;  // rad N
         double lon = P.getLongitude() * DEG_TO_RAD;                          // rad E
         double ca = ::cos(lat);
         double sa = ::sin(lat);
         double co = ::cos(lon);
         double so = ::sin(lon);
   
         // This is the rotation matrix which will
         // transform  X=(x,y,z) into (R*X)(north,east,up)
         R(0,0) = -sa*co;  R(0,1) = -sa*so;  R(0,2) = ca;
         R(1,0) =    -so;  R(1,1) =     co;  R(1,2) = 0.;
         R(2,0) =  ca*co;  R(2,1) =  ca*so;  R(2,2) = sa;
   
         // The rows of R are also the unit vectors, in ECEF, of north,east,up;
         //  R = (N && E && U) = transpose(N || E || U).
         //Vector<double> N = R.rowCopy(0);
         //Vector<double> E = R.rowCopy(1);
         //Vector<double> U = R.rowCopy(2);
   
         return R;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }
   
   // Same as NorthEastUp(P,true).
   Matrix<double> NorthEastUpGeocentric(Position& P)
      { return NorthEastUp(P, true); }
   
   // Same as NorthEastUp(P,false).
   Matrix<double> NorthEastUpGeodetic(Position& P)
      { return NorthEastUp(P, false); }

   // --------------------------------------------------------------------------------
   // Given a Position, compute unit (ECEF) vectors in the topocentric Up, East, North
   // directions at that position. Use geodetic coordinates, i.e. 'up' is
   // perpendicular to the geoid. Return the vectors in the form of a
   // 3x3 Matrix<double>, this is in fact the rotation matrix that will take an
   // ECEF vector into a local topocentric or 'up-east-north' vector.
   Matrix<double> UpEastNorth(Position& P, bool geocentric)
   {
      try {
         Matrix<double> R = NorthEastUp(P, geocentric);
         for(int i=0; i<3; i++) { double r=R(0,i); R(0,i)=R(2,i); R(2,i)=r; }
         return R;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }
   
   // Same as UpEastNorth, but using geocentric coordinates, so that the -Up
   // direction will meet the center of Earth.
   Matrix<double> UpEastNorthGeocentric(Position& P)
      { return UpEastNorth(P, true); }
   
   // Same as UpEastNorth, but using geodetic coordinates
   Matrix<double> UpEastNorthGeodetic(Position& P)
      { return UpEastNorth(P, false); }

   //---------------------------------------------------------------------------------
   // Consider the sun and the earth as seen from the satellite. Let the sun be a
   // circle of angular radius r, center in direction s, and the earth be a (larger)
   // circle of angular radius R, center in direction e. The circles overlap if
   // |e-s| < R+r; complete overlap if |e-s| < R-r.
   // The satellite is in penumbra if R-r < |e-s| < R+r, umbra if |e-s| < R-r.
   //    Let L == |e-s|. What is the area of overlap in penumbra : R-r < L < R+r ?
   // Call the two points where the circles intersect p1 and p2. Draw a line from
   // e to s; call the points where this line intersects the two circles r1 and R1,
   // respectively.
   // Draw lines from e to s, e to p1, e to p2, s to p1 and s to p2. Call the angle
   // between e-s and e-p1 alpha, and that between s-e and s-p1, beta.
   // Draw a rectangle with top and bottom parallel to e-s passing through p1 and p2,
   // and with sides passing through s and r1. Similarly for e and R1. Note that the
   // area of intersection lies within the intersection of these two rectangles.
   // Call the area of the rectangle outside the circles A and B. The height H of the
   // rectangles is
   // H = 2Rsin(alpha) = 2rsin(beta)
   // also L = rcos(beta)+Rcos(alpha)
   // The area A will be the area of the rectangle
   //              minus the area of the wedge formed by the angle 2*alpha
   //              minus the area of the two triangles which meet at s :
   // A = RH - (2alpha/2pi)*pi*R*R - 2*(1/2)*(H/2)Rcos(alpha)
   // Similarly
   // B = rH - (2beta/2pi)*pi*r*r  - 2*(1/2)*(H/2)rcos(beta)
   // The area of intersection will be the area of the rectangular intersection
   //                            minus the area A
   //                            minus the area B
   // Intersection = H(R+r-L) - A - B
   //          = HR+Hr-HL -HR+alpha*R*R+(H/2)Rcos(alpha) -Hr+beta*r*r+(H/2)rcos(beta)
   // Cancel terms, and substitute for L using above equation L = ..
   //          = -(H/2)rcos(beta)-(H/2)Rcos(alpha)+alpha*R*R+beta*r*r
   // substitute for H/2
   //          = -R*R*sin(alpha)cos(alpha)-r*r*sin(beta)cos(beta)+alpha*R*R+beta*r*r
   // Intersection = R*R*[alpha-sin(alpha)cos(alpha)]+r*r*[beta-sin(beta)cos(beta)]
   // Solve for alpha and beta in terms of R, r and L using H and L relations above
   // (r/R)cos(beta)=(L/R)-cos(alpha)
   // (r/R)sin(beta)=sin(alpha)
   // so
   // (r/R)^2 = (L/R)^2 - (2L/R)cos(alpha) + 1
   // cos(alpha) = (R/2L)(1+(L/R)^2-(r/R)^2)
   // cos(beta) = (L/r) - (R/r)cos(alpha)
   // and 0 <= alpha or beta <= pi
   //
   // AngRadEarth    angular radius of the earth as seen at the satellite (any unit)
   // AngRadSun      angular radius of the sun as seen at the satellite (same unit)
   // AngSeparation  angular distance of the sun from the earth (same unit)
   // return         fraction (0 <= f <= 1) of area of sun covered by earth
   // units only need be consistent
   double ShadowFactor(double AngRadEarth, double AngRadSun, double AngSeparation)
   {
      try {
         if(AngSeparation >= AngRadEarth+AngRadSun) return 0.0;
         if(AngSeparation <= fabs(AngRadEarth-AngRadSun)) return 1.0;
         double r=AngRadSun, R=AngRadEarth, L=AngSeparation;
         if(AngRadSun > AngRadEarth) { r=AngRadEarth; R=AngRadSun; }
         double cosalpha = (R/L)*(1.0+(L/R)*(L/R)-(r/R)*(r/R))/2.0;
         double cosbeta = (L/r) - (R/r)*cosalpha;
         double sinalpha = ::sqrt(1-cosalpha*cosalpha);
         double sinbeta = ::sqrt(1-cosbeta*cosbeta);
         double alpha = ::asin(sinalpha);
         double beta = ::asin(sinbeta);
         double shadow = r*r*(beta-sinbeta*cosbeta)+R*R*(alpha-sinalpha*cosalpha);
         shadow /= ::acos(-1.0)*AngRadSun*AngRadSun;
         return shadow;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }
   
   // --------------------------------------------------------------------------------
   double ShadowFactor(const Position& SV, const Position& Sun)
   {
      try {
         Position z(SV);
         // z points from satellite to Earth center - along the antenna boresight
         z.transformTo(Position::Cartesian);
         double svrange = z.mag();
         double d = -1.0/svrange;
         z = d * z;                                // reverse and normalize z

         // find distance to Sun, and unit vector Earth to Sun
         Position ssun(Sun);
         ssun.transformTo(Position::Cartesian);
         double dSun = ssun.radius();              // distance to Sun
         d = 1.0/dSun;
         ssun = d * ssun;                          // unit vector Earth-to-Sun

         // Sun-Earth-satellite angle = angular separation sat and Sun seen at earth
         // same as SatelliteEarthSunAngle(SV,Sun)
         double sesa = ::acos(-z.dot(ssun));

         // apparent angular radius (in deg) of Sun = 0.2666/(distance in AU)
         double AngRadSun = DEG_TO_RAD * 0.2666 / (dSun/149598.0e6);
         // angular radius of earth at sat
         double AngRadEarth = ::asin(6378137.0/svrange);

         return ShadowFactor(AngRadEarth, AngRadSun, sesa);
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
   }

   // --------------------------------------------------------------------------------
   // Compute the satellite attitude, given the time and the satellite position SV.
   // NB. Use either class SolarSystem (high accuracy) or module SolarPosition (low
   // accuracy) to get the Sun position.
   // Return a 3x3 Matrix that contains, as rows, the unit (ECEF) vectors X,Y,Z in the
   // body frame of the satellite, namely
   //    Z = along the boresight (i.e. towards Earth center),
   //    Y = perpendicular to both Z and the satellite-sun direction, and
   //    X = completing the orthonormal triad. X will generally point toward the sun.
   // Thus this rotation matrix R * (ECEF XYZ vector) = components in body frame, and
   // transpose(R) * (sat. body. frame vector) = ECEF XYZ components.
   // Also return the shadow factor = fraction of sun's area not visible to satellite,
   // and the angle in radians between sun and satellite as seen from Earth.
   Matrix<double> SatelliteAttitude(const Position& Sat, const Position& Sun)
   {
      try {
         Position PSun(Sun), PSat(Sat);
         Triple ssun(PSun.asECEF()), sat(PSat.asECEF()); // must be cartesian

         // make orthonormal triad parallel to satellite body axes
         Triple X,Y,Z,T;

         // Z points from satellite to Earth center - along the antenna boresight
         Z = sat;
         double d = -1.0/sat.mag();    // reverse Z and normalize
         Z = d * Z;

         // let T point from satellite to sun
         T = ssun - sat;               // sat-to-sun = (Earth-to-Sun) - (Earth-to-sat)
         d = 1.0/T.mag();
         T = d * T;                    // normalize

         // Y is perpendicular to Z and T ...
         Y = Z.cross(T);
         d = 1.0/Y.mag();
         Y = d * Y;                    // normalize Y
   
         // ... such that X points generally in the direction of the sun
         X = Y.cross(Z);               // X will be unit vector since Y and Z are

         if(X.dot(T) < 0) {            // need to reverse X, hence Y also
            X = -1.0 * X;
            Y = -1.0 * Y;
         }
   
         // fill the rotation matrix: rows are X, Y and Z
         // so R*V(ECEF XYZ) = V(body frame components) = (V dot X,Y,Z)
         Matrix<double> R(3,3);
         for(int i=0; i<3; i++) {
            R(0,i) = X[i];
            R(1,i) = Y[i];
            R(2,i) = Z[i];
         }
   
         return R;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }

  // -----------------------------------------------------------------------------------
  // Compute the satellite attitude, given the satellite position P and velocity V,
  // assuming an orbit-normal attitude.
  // Return a 3x3 Matrix which contains, as rows, the unit (ECEF) vectors X,Y,Z
  // in the body frame of the satellite, namely
  //    Z = along the boresight (i.e. towards Earth center),
  //    Y = perpendicular to orbital plane, pointing opposite the angular momentum vector
  //    X = along-track (same direction as velocity vector for circular orbit)
  // Thus this rotation matrix R transforms an ECEF XYZ vector into the body frame
  // of the satellite, so R * (ECEF XYZ vector) = components in body frame.
  // Also, R.transpose() * (sat. body. frame vector) = ECEF XYZ components.
  Matrix<double> OrbitNormalAttitude(const Position& P, const Position& V)
  {
    try {
      int i;
      double svrange, angmom;
      Position X,Y,Z;
      Matrix<double> R(3,3);

      // Z points from satellite to Earth center - along the antenna boresight
      svrange = P.mag();
      Z = P*(-1.0/svrange); // reverse and normalize Z
      Z.transformTo(Position::Cartesian);

      // Y points opposite the angular momentum vector
      Y = P.cross(V);
      angmom = Y.mag();
      Y = Y*(-1.0/angmom); // reverse and normalize Y
      Y.transformTo(Position::Cartesian);

      // X completes the right-handed system
      X = Y.cross(Z);

      // fill the matrix and return it
      for(i=0; i<3; i++) {
	R(0,i) = X[i];
	R(1,i) = Y[i];
	R(2,i) = Z[i];
      }

      return R;
    }
    catch(Exception& e) { GPSTK_RETHROW(e); }
    catch(std::exception& e) {Exception E("std except: "+string(e.what())); GPSTK_THROW(E);}
    catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
  }
   
   // --------------------------------------------------------------------------------
   // Compute the azimuth and nadir angle, in the satellite body frame,
   // of receiver Position RX as seen at the satellite Position SV. The nadir angle
   // is measured from the Z axis, which points to Earth center, and azimuth is
   // measured from the X axis.
   // @param SV Position Satellite position
   // @param RX Position Receiver position
   // @param Rot Matrix<double> Rotation matrix (3,3), output of SatelliteAttitude
   // @param nadir double Output nadir angle in degrees
   // @param azimuth double Output azimuth angle in degrees
   void SatelliteNadirAzimuthAngles(const Position& SV,
                                    const Position& RX,
                                    const Matrix<double>& Rot,
                                    double& nadir,
                                    double& azimuth)
   {
      try {
         if(Rot.rows() != 3 || Rot.cols() != 3) {
            Exception e("Rotation matrix invalid");
            GPSTK_THROW(e);
         }
   
         double d;
         Position RmS;
         // RmS points from satellite to receiver
         RmS = RX - SV;
         RmS.transformTo(Position::Cartesian);
         d = RmS.mag();
         if(d == 0.0) {
            Exception e("Satellite and Receiver Positions identical");
            GPSTK_THROW(e);
         }
         RmS = (1.0/d) * RmS;
   
         Vector<double> XYZ(3),Body(3);
         XYZ(0) = RmS.X();
         XYZ(1) = RmS.Y();
         XYZ(2) = RmS.Z();
         Body = Rot * XYZ;
   
         nadir = ::acos(Body(2)) * RAD_TO_DEG;
         azimuth = ::atan2(Body(1),Body(0)) * RAD_TO_DEG;
         if(azimuth < 0.0) azimuth += 360.;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }
   
   // --------------------------------------------------------------------------------
   // Compute the angle from satellite to Earth to Sun; that is the angular
   // separation of the satellite and the Sun, as seen from the center of Earth.
   // This angle lies between zero and pi, and it reaches zero (pi)
   // only when the Sun lies exactly in the orbit plane at noon (midnight).
   // NB. Use either class SolarSystem (high accuracy) or module SolarPosition
   // (low accuracy) to get the Sun position.
   // Return the angle in radians.
   // @param Sat Position Satellite position
   // @param Sun Position Sun position
   // @return double angle     Angle in radians satellite-Earth-Sun
   double SatelliteEarthSunAngle(const Position& Sat, const Position& Sun)
   {
      try {
         Position PSun(Sun), PSat(Sat);
         Triple ssun(PSun.asECEF()), sat(PSat.asECEF());
         double d;         // normalize
         d = 1.0/ssun.mag(); ssun = ssun * d;
         d = 1.0/sat.mag(); sat = sat * d;
   
         // compute the angle
         return (::acos(sat.dot(ssun)));
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }

   // --------------------------------------------------------------------------------
   // Compute the angle between the Sun and the plane of the orbit of the satellite.
   // Return the angle in radians; it lies between +-pi/2 and has the sign of RxV.
   // That is, the angle is positive if the Sun is out of the orbit plane in the
   // direction of R cross V; then Sun "sees" the orbit motion as counter-clockwise.
   // Also return, in phi, the angle, in the plane of the orbit, from midnight to the
   // satellite; this lies between 0 and 2pi and increases in the direction of Vel.
   // NB. Use either class SolarSystem (high accuracy) or module SolarPosition
   // (lower accuracy) to get the Sun position.
   // @param Position Pos   Satellite position at time of interest
   // @param Position Vel   Satellite velocity at time of interest
   // @param Position Sun   Sun position at time of interest
   // @param double beta    Return angle sun to plane of satellite orbit (radians)
   // @param double phi     Return angle in orbit plane, midn to satellite (radians)
   // NB. phi, beta and sesa - the satellite-earth-sun angle - form a right spherical
   // triangle with sesa opposite the right angle. Thus cos(sesa)=cos(beta)*cos(phi).
   void SunOrbitAngles(const Position& Pos, const Position& Vel, const Position& Sun,
                       double& beta, double& phi)
   {
      try {
         Position SatR(Pos),SatV(Vel),PSun(Sun);
         GPSEllipsoid ellips;
         double omega(ellips.angVelocity()); // 7.292115e-5 rad/sec
   
         // compute inertial velocity
         Position inertialV;
         inertialV.setECEF(SatV.X()-omega*SatR.Y(),SatV.Y()+omega*SatR.X(),SatV.Z());

         // use Triple
         Triple ssun(PSun.asECEF()),sat(SatR.asECEF());
         Triple vel(inertialV.asECEF()), u, w;

         // normalize
         double d;
         d = 1.0/ssun.mag(); ssun = ssun * d;
         d = 1.0/sat.mag(); sat = d * sat;
         d = 1.0/vel.mag(); vel = d * vel;

         // u is R cross V - normal to orbit plane
         u = sat.cross(vel);
   
         // compute the angle beta : u dot sun = sin(beta) = cos(PI/2-beta)
         double udotsun(u.dot(ssun));
         beta = PI/2.0 - ::acos(udotsun);

         // compute phi, angle in orbit plane between sun and sat
         // zero at midnight, increasing with satellite motion
         // w = ssun - u*sin(beta) lies in the orbit plane but ~points to sun
         // but if beta == pi/2, u==ssun and w==zero, phi indeterminate
         w = ssun - udotsun*u;
         d = w.mag();
         if(d > 1.e-14) {
            d = 1.0/d;
            w = d * w;                    // normalize w
            phi = ::acos(sat.dot(w));     // zero at noon where sat||w and dot=1
            if(sat.dot(u.cross(w)) < 0.0) // make phi zero at midnight
               phi = PI - phi;
            else
               phi += PI;
         }
         else
            phi = 0.0;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
      catch(exception& e) {
         Exception E("std except: " + string(e.what()));
         GPSTK_THROW(E);
      }
      catch(...) { Exception e("Unknown exception"); GPSTK_THROW(e); }
   }
   
   // --------------------------------------------------------------------------------
   // Compute the nominal yaw angle of the satellite given the satellite position and
   // velocity and the Sun position at the given time, plus a flag for GPS Block IIR
   // and IIF satellites. Return the nominal yaw angle in radians, and the yaw rate
   // in radians/second.
   // @param Position P     Satellite position at time of interest
   // @param Position V     Satellite velocity at time of interest (Cartesian, m/s)
   // @param Position Sun   Sun position at time of interest
   // @param bool blkIIRF   True if the satellite is GPS block IIR or IIF
   // @param double yawrate Return yaw rate in radians/second
   // @return double yaw    Satellite yaw angle in radians
   double SatelliteYawAngle(const Position& pos, const Position& vel,
                            const Position& Sun, const bool& blkIIRF, double& yawrate)
   {
      try {
         // get orbit tilt beta, and in-plane orbit angle from midnight, mu
         double beta, mu;
         SunOrbitAngles(pos, vel, Sun, beta, mu);
         double sinmu = ::sin(mu);  // mu is the in-orbit-plane "azimuth" from midnite
         double cosmu = ::cos(mu);  // increasing in direction of satellite motion

         // nominal yaw angle - cf Kouba(2009)
         double tanb(::tan(beta)), yaw;
         if(blkIIRF)
            yaw = ::atan2(tanb, -sinmu);
         else
            yaw = ::atan2(-tanb, sinmu);

         // orbit velocity (rad/sec)
         double orbv = TWO_PI * vel.mag() / pos.mag();

         // nominal yaw rate
         yawrate = orbv * tanb * cosmu / (sinmu*sinmu+tanb*tanb);

         return yaw;
      }
      catch(Exception& e) { GPSTK_RETHROW(e); }
   }

}  // end namespace
