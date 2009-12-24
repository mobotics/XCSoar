/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
 */
#ifndef FLATPOINT_HPP
#define FLATPOINT_HPP

#include "Math/fixed.hpp"

/**
 * 2-d Cartesian projected real-valued point
 */
struct FlatPoint 
{
/** 
 * Constructor given known location
 * 
 * @param _x X position
 * @param _y Y position
 * 
 * @return Initialised object
 */
  FlatPoint(const fixed _x, const fixed _y): x(_x),y(_y) {};

/** 
 * Constructor at origin
 * 
 * @return Initialised object
 */
  FlatPoint(): x(fixed_zero),y(fixed_zero) {};

  fixed x; /**< X location */
  fixed y; /**< Y location */

/** 
 * Calculate cross product of two points
 * 
 * @param p2 Other point
 * 
 * @return Cross product
 */
  fixed cross(const FlatPoint& p2) const;

/** 
 * Multiply Y value of point
 * 
 * @param a Value to multiply
 */
  void mul_y(const fixed a);

/** 
 * Subtract delta from this point
 * 
 * @param p2 Point to subtract
 */
  void sub(const FlatPoint&p2);

/** 
 * Add delta to this point
 * 
 * @param p2 Point to add
 */
  void add(const FlatPoint&p2);

/** 
 * Rotate point clockwise around origin
 * 
 * @param angle Angle (deg) to rotate point clockwise
 */
  void rotate(const fixed angle);

/** 
 * Calculate distance between two points
 * 
 * @param p Other point
 * 
 * @return Distance
 */
  fixed d(const FlatPoint &p) const;

/** 
 * Find dx*dx+dy*dy
 * @return Magnitude squared
 */
  fixed mag_sq() const;

/** 
 * Find sqrt(dx*dx+dy*dy)
 * @return Magnitude 
 */
  fixed mag() const;

/** 
 * Test whether two points are co-located
 * 
 * @param other Point to compare
 * 
 * @return True if coincident
 */
  bool operator== (const FlatPoint &other) const {
    return (x == other.x) && (y == other.y);
  };

/** 
 * Calculate dot product of one point with another
 * 
 * @param other That point
 * 
 * @return Dot product
 */
  fixed dot(const FlatPoint &other) const {
    return x*other.x+y*other.y;
  }

/** 
 * Scale a point
 * 
 * @param p Scale
 * 
 * @return Scaled point
 */
  FlatPoint operator* (const fixed &p) const {
    FlatPoint res= *this;
    res.x *= p;
    res.y *= p;
    return res;
  };

/** 
 * Add one point to another
 * 
 * @param p2 Point to add
 * 
 * @return Added value
 */
  FlatPoint operator+ (const FlatPoint &p2) const {
    FlatPoint res= *this;
    res.x += p2.x;
    res.y += p2.y;
    return res;
  };

/** 
 * Subtract one point from another
 * 
 * @param p2 Point to subtract
 * 
 * @return Subtracted value
 */
  FlatPoint operator- (const FlatPoint &p2) const {
    FlatPoint res= *this;
    res.x -= p2.x;
    res.y -= p2.y;
    return res;
  };
};

#endif
