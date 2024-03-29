/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

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

#include "Device.hpp"
#include "Device/Port/Port.hpp"
#include "Device/Declaration.hpp"
#include "Operation/Operation.hpp"

bool
FlarmDevice::Declare(const Declaration &declaration,
                     gcc_unused const Waypoint *home,
                     OperationEnvironment &env)
{
  if (!TextMode(env))
    return false;

  if (!DeclareInternal(declaration, env)) {
    mode = Mode::UNKNOWN;
    return false;
  }

  // TODO bug: JMW, FLARM Declaration checks
  // Only works on IGC approved devices
  // Total data size must not surpass 183 bytes
  // probably will issue PFLAC,ERROR if a problem?

  return true;
}

bool
FlarmDevice::DeclareInternal(const Declaration &declaration,
                             OperationEnvironment &env)
{
  unsigned size = declaration.Size();

  env.SetProgressRange(6 + size);
  env.SetProgressPosition(0);

  if (!SetPilot(declaration.pilot_name.c_str(), env))
    return false;

  env.SetProgressPosition(1);

  if (!SetPlaneRegistration(declaration.aircraft_registration.c_str(), env))
    return false;

  env.SetProgressPosition(2);

  if (!SetPlaneType(declaration.aircraft_type.c_str(), env))
    return false;

  env.SetProgressPosition(3);

  if (!SetConfig("NEWTASK", _T("Task"), env))
    return false;

  env.SetProgressPosition(4);

  if (!SetConfig("ADDWP", _T("0000000N,00000000E,TAKEOFF"), env))
    return false;

  env.SetProgressPosition(5);

  for (unsigned i = 0; i < size; ++i) {
    int DegLat, DegLon;
    fixed tmp, MinLat, MinLon;
    char NoS, EoW;

    tmp = declaration.GetLocation(i).latitude.Degrees();
    if (negative(tmp)) {
      NoS = 'S';
      tmp = -tmp;
    } else {
      NoS = 'N';
    }
    DegLat = (int)tmp;
    MinLat = (tmp - fixed(DegLat)) * 60 * 1000;

    tmp = declaration.GetLocation(i).longitude.Degrees();
    if (negative(tmp)) {
      EoW = 'W';
      tmp = -tmp;
    } else {
      EoW = 'E';
    }
    DegLon = (int)tmp;
    MinLon = (tmp - fixed(DegLon)) * 60 * 1000;

    StaticString<256> buffer;
    buffer.Format(_T("%02d%05.0f%c,%03d%05.0f%c,%s"), DegLat,
                  (double)MinLat, NoS, DegLon, (double)MinLon, EoW,
                  declaration.GetName(i));

    if (!SetConfig("ADDWP", buffer, env))
      return false;

    env.SetProgressPosition(6 + i);
  }

  if (!SetConfig("ADDWP", _T("0000000N,00000000E,LANDING"), env))
    return false;

  env.SetProgressPosition(6 + size);

  // PFLAC,S,KEY,VALUE
  // Expect
  // PFLAC,A,blah
  // PFLAC,,COPIL:
  // PFLAC,,COMPID:
  // PFLAC,,COMPCLASS:

  // PFLAC,,NEWTASK:
  // PFLAC,,ADDWP:

  // Reset the FLARM to activate the declaration
  Restart();

  return true;
}
