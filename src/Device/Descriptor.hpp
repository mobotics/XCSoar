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

#ifndef XCSOAR_DEVICE_DESCRIPTOR_HPP
#define XCSOAR_DEVICE_DESCRIPTOR_HPP

#include "Port/Port.hpp"
#include "Port/LineHandler.hpp"
#include "Device/Parser.hpp"
#include "Profile/DeviceConfig.hpp"
#include "RadioFrequency.hpp"
#include "NMEA/ExternalSettings.hpp"
#include "PeriodClock.hpp"
#include "Job/Async.hpp"
#include "Thread/Notify.hpp"

#include <assert.h>
#include <tchar.h>
#include <stdio.h>

struct NMEAInfo;
struct DerivedInfo;
struct Declaration;
struct Waypoint;
class Port;
class Device;
class AtmosphericPressure;
struct DeviceRegister;
class InternalSensors;
class RecordedFlightList;
struct RecordedFlightInfo;
class OperationEnvironment;
class OpenDeviceJob;

class DeviceDescriptor : private Notify, private PortLineHandler {
  /** the index of this device in the global list */
  const unsigned index;

  DeviceConfig config;

  /**
   * This object runs the Open() method in background to make it
   * non-blocking.
   */
  AsyncJobRunner async;

  OpenDeviceJob *open_job;

  Port *port;
  Port::Handler *monitor;

  DeviceDescriptor *pipe_to_device;
  const DeviceRegister *driver;

  Device *device;

#ifdef ANDROID
  InternalSensors *internal_sensors;
#endif

  /**
   * This clock keeps track when we need to reopen the device next
   * time after a failure or after a timeout.  It gets updated each
   * time the failure/timeout occurs, and again after each retry.
   */
  PeriodClock reopen_clock;

  NMEAParser parser;

  /**
   * The settings that were sent to the device.  This is used to check
   * if the device is sending back the new configuration; then the
   * device isn't actually sending a new setting, it is merely
   * repeating the settings we sent it.  This should not make XCSoar
   * reconfigure itself.
   */
  ExternalSettings settings_sent;

  /**
   * The settings that were received from the device.  This temporary
   * buffer mirrors NMEA_INFO::settings; NMEA_INFO::settings may get
   * cleared with ExternalSettings::EliminateRedundant(), so this one
   * always preserves the original values from the device, without
   * having to do a full NMEA_INFO copy.
   */
  ExternalSettings settings_received;

  bool was_alive;

  bool ticker;

  /**
   * True when somebody has "borrowed" the device.  Link timeouts are
   * disabled meanwhile.
   *
   * This attribute is only accessed from the main thread.
   *
   * @see CanBorrow(), Borrow()
   */
  bool borrowed;

public:
  DeviceDescriptor(unsigned index);
  ~DeviceDescriptor() {
    assert(!IsOccupied());
  }

  unsigned GetIndex() const {
    return index;
  }

  const DeviceConfig &GetConfig() const {
    return config;
  }

  DeviceConfig &SetConfig() {
    return config;
  }

  void SetPipeTo(DeviceDescriptor *_pipe_to_device) {
    pipe_to_device = _pipe_to_device;
  }

  bool IsConfigured() const {
    return config.port_type != DeviceConfig::PortType::DISABLED;
  }

  bool IsOpen() const {
    return port != NULL
#ifdef ANDROID
      || internal_sensors != NULL;
#endif
    ;
  }

  /**
   * Returns the Device object; may be NULL if the device is not open
   * or if the Device class is not applicable for this object.
   *
   * Should only be used by driver-specific code (such as the CAI 302
   * manager).
   */
  Device *GetDevice() {
    return device;
  }

private:
  /**
   * Cancel the #AsyncJobRunner object if it is running.
   */
  void CancelAsync();

  /**
   * When this method fails, the caller is responsible for freeing the
   * Port object.
   */
  bool Open(Port &port, const DeviceRegister &driver,
            OperationEnvironment &env);

  bool OpenInternalSensors();

public:
  /**
   * To be used by OpenDeviceJob, don't call directly.
   */
  bool DoOpen(OperationEnvironment &env);

  /**
   * @param env a persistent object
   */
  void Open(OperationEnvironment &env);

  void Close();

  /**
   * @param env a persistent object
   */
  void Reopen(OperationEnvironment &env);

  /**
   * Call this periodically to auto-reopen a failed device after a
   * certain delay.
   *
   * @param env a persistent object
   */
  void AutoReopen(OperationEnvironment &env);

  /**
   * Call this method after Declare(), ReadFlightList(),
   * DownloadFlight() when you're done, to switch back to NMEA mode.
   */
  bool EnableNMEA(OperationEnvironment &env);

  const TCHAR *GetDisplayName() const;

  /**
   * Compares the driver's name.
   */
  bool IsDriver(const TCHAR *name) const;

  gcc_pure
  bool CanDeclare() const;

  gcc_pure
  bool IsLogger() const;

  bool IsCondor() const {
    return IsDriver(_T("Condor"));
  }

  bool IsVega() const {
    return IsDriver(_T("Vega"));
  }

  bool IsNMEAOut() const;
  bool IsManageable() const;

  bool IsBorrowed() const {
    return borrowed;
  }

  /**
   * Is this device currently occupied, i.e. does somebody have
   * exclusive access?
   *
   * May only be called from the main thread.
   */
  bool IsOccupied() const {
    return IsBorrowed() || async.IsBusy();
  }

  /**
   * Can this device be borrowed?
   *
   * May only be called from the main thread.
   *
   * @see Borrow()
   */
  bool CanBorrow() const {
    return device != NULL && port != NULL && !IsOccupied();
  }

  /**
   * "Borrow" the device.  The caller gets exclusive access, e.g. to
   * submit a task declaration.  Call Return() when you are done.
   *
   * May only be called from the main thread.
   *
   * @return false if the device is already occupied and cannot be
   * borrowed
   */
  bool Borrow();

  /**
   * Return a borrowed device.  The caller is responsible for
   * switching the device back to NMEA mode, see EnableNMEA().
   *
   * May only be called from the main thread.
   */
  void Return();

  /**
   * Query the device's "alive" flag from the DeviceBlackboard.
   * This method locks the DeviceBlackboard.
   */
  gcc_pure
  bool IsAlive() const;

private:
  bool ParseNMEA(const char *line, struct NMEAInfo &info);

public:
  void SetMonitor(Port::Handler *_monitor) {
    monitor = _monitor;
  }

  bool WriteNMEA(const char *line);
#ifdef _UNICODE
  bool WriteNMEA(const TCHAR *line);
#endif

  bool PutMacCready(fixed mac_cready, OperationEnvironment &env);
  bool PutBugs(fixed bugs, OperationEnvironment &env);
  bool PutBallast(fixed fraction, fixed overload,
                  OperationEnvironment &env);
  bool PutVolume(int volume, OperationEnvironment &env);
  bool PutActiveFrequency(RadioFrequency frequency,
                          OperationEnvironment &env);
  bool PutStandbyFrequency(RadioFrequency frequency,
                           OperationEnvironment &env);
  bool PutQNH(const AtmosphericPressure &pres, OperationEnvironment &env);

  /**
   * Caller is responsible for calling Borrow() and Return().
   */
  bool Declare(const Declaration &declaration, const Waypoint *home,
               OperationEnvironment &env);

  /**
   * Caller is responsible for calling Borrow() and Return().
   */
  bool ReadFlightList(RecordedFlightList &flight_list,
                      OperationEnvironment &env);

  /**
   * Caller is responsible for calling Borrow() and Return().
   */
  bool DownloadFlight(const RecordedFlightInfo &flight, const TCHAR *path,
                      OperationEnvironment &env);

  void OnSysTicker(const DerivedInfo &calculated);

private:
  bool ParseLine(const char *line);

  /* virtual methods from class Notify */
  virtual void OnNotification();

  /* virtual methods from Port::Handler */
  virtual void DataReceived(const void *data, size_t length);

  /* virtual methods from PortLineHandler */
  virtual void LineReceived(const char *line);
};

#endif
