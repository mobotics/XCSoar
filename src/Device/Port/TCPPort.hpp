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

#ifndef XCSOAR_DEVICE_TCP_PORT_HPP
#define XCSOAR_DEVICE_TCP_PORT_HPP

#include "Thread/StoppableThread.hpp"
#include "Port.hpp"

/**
 * A TCP listener port class.
 */
class TCPPort : public Port, protected StoppableThread
{
  unsigned port;

  int listener_fd, connection_fd;

public:
  /**
   * Creates a new TCPPort object, but does not open it yet.
   *
   * @param port the port number (1..32767)
   * @param handler the callback object for input received on the
   * port
   */
  TCPPort(unsigned port, Handler &handler);

  /**
   * Closes the serial port (Destructor)
   */
  virtual ~TCPPort();

  virtual size_t Write(const void *data, size_t length);
  virtual bool Drain();
  virtual void Flush();

  /**
   * Opens the serial port
   * @return True on success, False on failure
   */
  bool Open();
  /**
   * Closes the serial port
   * @return True on success, False on failure
   */
  bool Close();

  virtual bool SetRxTimeout(unsigned Timeout);
  virtual unsigned GetBaudrate() const;
  virtual bool SetBaudrate(unsigned baud_rate);
  virtual bool StopRxThread();
  virtual bool StartRxThread();

  virtual int Read(void *buffer, size_t length);
  virtual WaitResult WaitRead(unsigned timeout_ms);

protected:
  /**
   * Entry point for the receive thread
   */
  virtual void Run();
};

#endif
