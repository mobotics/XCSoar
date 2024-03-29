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

#include "UnitsEditor.hpp"
#include "DataField/Enum.hpp"
#include "Language/Language.hpp"
#include "Util/Macros.hpp"
#include "Units/Units.hpp"

void
CAI302UnitsEditor::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  static const StaticEnumChoice vario_list[] = {
    { 0, _T("m/s"), },
    { 1, _T("kt"), },
    { 0 }
  };
  AddEnum(_("Vario"), NULL, vario_list,
          data.GetVarioUnit());

  static const StaticEnumChoice altitude_list[] = {
    { 0, _T("m"), },
    { 1, _T("ft"), },
    { 0 }
  };
  AddEnum(_("Altitude"), NULL, altitude_list,
          data.GetAltitudeUnit());

  static const StaticEnumChoice temperature_list[] = {
    { 0, _T(DEG "C"), },
    { 1, _T(DEG "F"), },
    { 0 }
  };
  AddEnum(_("Temperature"), NULL, temperature_list,
          data.GetTemperatureUnit());

  static const StaticEnumChoice pressure_list[] = {
    { 0, _T("hPa"), },
    { 1, _T("inHg"), },
    { 0 }
  };
  AddEnum(_("Pressure"), NULL, pressure_list,
          data.GetPressureUnit());

  static const StaticEnumChoice distance_list[] = {
    { 0, _T("km"), },
    { 1, _T("NM"), },
    { 2, _T("mi"), },
    { 0 }
  };
  AddEnum(_("Distance"), NULL, distance_list,
          data.GetDistanceUnit());

  static const StaticEnumChoice speed_list[] = {
    { 0, _T("m/s"), },
    { 1, _T("kt"), },
    { 2, _T("mph"), },
    { 0 }
  };
  AddEnum(_("Speed"), NULL, speed_list,
          data.GetSpeedUnit());
}

bool
CAI302UnitsEditor::Save(bool &_changed, bool &require_restart)
{
  bool changed = false;

  unsigned vario = data.GetVarioUnit();
  if (SaveValue(VarioUnit, vario)) {
    data.SetVarioUnit(vario);
    changed = true;
  }

  unsigned altitude = data.GetAltitudeUnit();
  if (SaveValue(AltitudeUnit, altitude)) {
    data.SetAltitudeUnit(altitude);
    changed = true;
  }

  unsigned temperature = data.GetTemperatureUnit();
  if (SaveValue(TemperatureUnit, temperature)) {
    data.SetTemperatureUnit(temperature);
    changed = true;
  }

  unsigned pressure = data.GetPressureUnit();
  if (SaveValue(PressureUnit, pressure)) {
    data.SetPressureUnit(pressure);
    changed = true;
  }

  unsigned distance = data.GetDistanceUnit();
  if (SaveValue(DistanceUnit, distance)) {
    data.SetDistanceUnit(distance);
    changed = true;
  }

  unsigned speed = data.GetSpeedUnit();
  if (SaveValue(SpeedUnit, speed)) {
    data.SetSpeedUnit(speed);
    changed = true;
  }

  _changed |= changed;
  return true;
}
