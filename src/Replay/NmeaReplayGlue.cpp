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

#include "Replay/NmeaReplayGlue.hpp"
#include "Device/Parser.hpp"
#include "Device/Driver.hpp"
#include "Device/Register.hpp"
#include "Language/Language.hpp"
#include "Dialogs/Message.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Interface.hpp"

NmeaReplayGlue::NmeaReplayGlue()
  :parser(NULL), device(NULL)
{
}

NmeaReplayGlue::~NmeaReplayGlue()
{
  delete device;
  delete parser;
}

void
NmeaReplayGlue::Start()
{
  assert(parser == NULL);
  assert(device == NULL);

  parser = new NMEAParser();
  parser->SetReal(false);

  /* get the device driver name from the profile */
  const DeviceConfig &config =
    CommonInterface::GetSystemSettings().devices[0];

  parser->SetIgnoreChecksum(config.ignore_checksum);

  /* instantiate it */
  const struct DeviceRegister *driver = FindDriverByName(config.driver_name);
  assert(driver != NULL);
  if (driver->CreateOnPort != NULL) {
    DeviceConfig config;
    config.Clear();
    device = driver->CreateOnPort(config, port);
  }

  NmeaReplay::Start();
}

void
NmeaReplayGlue::Stop()
{
  NmeaReplay::Stop();

  delete device;
  device = NULL;
  delete parser;
  parser = NULL;

  device_blackboard->StopReplay();
}

void
NmeaReplayGlue::OnSentence(const char *line)
{
  assert(device != NULL);

  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &data = device_blackboard->SetReplayState();

  if ((device != NULL && device->ParseNMEA(line, data)) ||
      (parser != NULL && parser->ParseLine(line, data))) {
    data.gps.replay = true;
    data.alive.Update(fixed(MonotonicClockMS()) / 1000);
    device_blackboard->ScheduleMerge();
  }
}

void
NmeaReplayGlue::OnBadFile()
{
  MessageBoxX(_("Could not open NMEA file!"),
              _("Flight replay"), MB_OK | MB_ICONINFORMATION);
}

bool
NmeaReplayGlue::UpdateTime()
{
  return clock.CheckUpdate(1000);
}

void
NmeaReplayGlue::ResetTime()
{
  clock.Reset();
}
