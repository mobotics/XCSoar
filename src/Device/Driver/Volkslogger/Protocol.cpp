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

#include "Protocol.hpp"
#include "CRC16.hpp"
#include "Device/Port/Port.hpp"
#include "Operation/Operation.hpp"
#include "TimeoutClock.hpp"

#include <string.h>

bool
Volkslogger::Reset(Port &port, OperationEnvironment &env, unsigned n)
{
  static const unsigned delay = 2;

  while (n-- > 0) {
    if (!port.Write(CAN))
      return false;

    env.Sleep(delay);
  }

  return true;
}

bool
Volkslogger::Handshake(Port &port, OperationEnvironment &env,
                       unsigned timeout_ms)
{
  TimeoutClock timeout(timeout_ms);

  while (true) { // Solange R's aussenden, bis ein L zurückkommt
    if (!port.Write('R'))
      return false;

    int remaining = timeout.GetRemainingSigned();
    if (remaining < 0)
      return false;

    if (remaining > 500)
      remaining = 500;

    Port::WaitResult result =
      port.WaitForChar('L', env, remaining);
    if (result == Port::WaitResult::READY)
      break;

    if (result != Port::WaitResult::TIMEOUT)
      return false;

    /* timeout, try again */
  }

  unsigned count = 1;
  while (true) { // Auf 4 hintereinanderfolgende L's warten
    int remaining = timeout.GetRemainingSigned();
    if (remaining < 0)
      return false;

    if (port.WaitForChar('L', env, remaining) != Port::WaitResult::READY)
      return false;

    count++;
    if (count >= 4)
      return true;
  }
}

bool
Volkslogger::Connect(Port &port, OperationEnvironment &env,
                     unsigned timeout_ms)
{
  return Reset(port, env, 10) && Handshake(port, env, timeout_ms);
}

bool
Volkslogger::ConnectAndFlush(Port &port, OperationEnvironment &env,
                             unsigned timeout_ms)
{
  port.Flush();

  return Connect(port, env, timeout_ms) && port.FullFlush(env, 50, 300);
}

static bool
SendWithCRC(Port &port, const void *data, size_t length)
{
  if (!port.FullWrite(data, length, 2000))
    return false;

  uint16_t crc16 = UpdateCRC(data, length, 0);
  return port.Write(crc16 >> 8) && port.Write(crc16 & 0xff);
}

bool
Volkslogger::SendCommand(Port &port, OperationEnvironment &env,
                         Command cmd, uint8_t param1, uint8_t param2)
{
  static const unsigned delay = 2;

  /* flush buffers */
  if (!port.FullFlush(env, 20, 100))
    return false;

  /* reset command interpreter */
  if (!Reset(port, env, 6))
    return false;

  /* send command packet */

  const uint8_t cmdarray[8] = {
    (uint8_t)cmd, param1, param2,
    0, 0, 0, 0, 0,
  };

  if (!port.Write(ENQ))
    return false;

  env.Sleep(delay);

  if (!SendWithCRC(port, cmdarray, sizeof(cmdarray)))
    return false;

  /* wait for confirmation */

  return port.WaitRead(env, 4000) == Port::WaitResult::READY &&
    port.GetChar() == 0;
}

gcc_const
static int
GetBaudRateIndex(unsigned baud_rate)
{
  switch(baud_rate) {
  case 9600:
    return 1;

  case 19200:
    return 2;

  case 38400:
    return 3;

  case 57600:
    return 4;

  case 115200:
    return 5;

  default:
    return -1;
  }
}

bool
Volkslogger::SendCommandSwitchBaudRate(Port &port, OperationEnvironment &env,
                                       Command cmd, uint8_t param1,
                                       unsigned baud_rate)
{
  int baud_rate_index = GetBaudRateIndex(baud_rate);
  if (baud_rate_index < 0)
    return false;

  if (!SendCommand(port, env, cmd, param1, baud_rate_index))
    return false;

  return port.SetBaudrate(baud_rate);
}

bool
Volkslogger::WaitForACK(Port &port, OperationEnvironment &env)
{
  return port.WaitForChar(ACK, env, 30000) == Port::WaitResult::READY;
}

int
Volkslogger::ReadBulk(Port &port, OperationEnvironment &env,
                      void *buffer, unsigned max_length)
{
  unsigned nbytes = 0;
  bool dle_r = false;
  uint16_t crc16 = 0;
  bool start = false, ende = false;

  memset(buffer, 0xff, max_length);

  uint8_t *p = (uint8_t *)buffer;
  while (!ende) {
    // Zeichen anfordern und darauf warten
    if (!port.Write(ACK) ||
        port.WaitRead(env, 1000) != Port::WaitResult::READY)
      return -1;

    int ch = port.GetChar();
    if (ch < 0)
      return -1;

    // dabei ist Benutzerabbruch jederzeit möglich
    if (env.IsCancelled()) {
      env.Sleep(10);
      port.Write(CAN);
      port.Write(CAN);
      port.Write(CAN);
      return -1;
    }

    // oder aber das empfangene Zeichen wird ausgewertet
    switch (ch) {
    case DLE:
      if (!dle_r) {             //!DLE, DLE -> Achtung!
        dle_r = true;
      }
      else { 	                 // DLE, DLE -> DLE-Zeichen
        dle_r = false;
        if (start) {
          if(nbytes < max_length)
            *p++ = ch;
          nbytes++;
          crc16 = UpdateCRC(ch, crc16);
        }
      }
      break;
    case ETX:
      if (!dle_r) {             //!DLE, ETX -> Zeichen
        if (start) {
          if(nbytes < max_length) {
            *p++ = ch;
          }
          nbytes++;
          crc16 = UpdateCRC(ch, crc16);
        };
      }
      else {
        if (start) {
          ende = true; // DLE, ETX -> Blockende
          dle_r = false;
        }
      }
      break;
    case STX:
      if (!dle_r) { //!DLE, STX -> Zeichen
        if (start) {
          if(nbytes < max_length)
            *p++ = ch;
          nbytes++;
          crc16 = UpdateCRC(ch, crc16);
        }
      }
      else {
        start = true; // DLE, STX -> Blockstart
        dle_r = false;
        crc16 = 0;
      }
      break;
    default:
      if (start) {
        if(nbytes < max_length)
          *p++ = ch;
        nbytes++;
        crc16 = UpdateCRC(ch, crc16);
      }
      break;
    }
  }

  env.Sleep(100);

  if (crc16 != 0)
    return -1;

  if (nbytes < 2)
    return 0;

  // CRC am Ende abschneiden
  return nbytes - 2;
}

bool
Volkslogger::WriteBulk(Port &port, OperationEnvironment &env,
                       const void *buffer, unsigned length)
{
  const unsigned delay = 1;

  env.SetProgressRange(length);

  uint16_t crc16 = 0;
  const uint8_t *p = (const uint8_t *)buffer, *end = p + length;
  while (p < end) {
    unsigned n = end - p;
    if (n > 400)
      n = 400;

    n = port.Write(p, n);
    if (n == 0)
      return false;

    crc16 = UpdateCRC(p, n, crc16);
    p += n;

    env.SetProgressPosition(p - (const uint8_t *)buffer);

    /* throttle sending a bit, or the Volkslogger's receive buffer
       will overrun */
    env.Sleep(delay * 100);
  }

  return port.Write(crc16 >> 8) && port.Write(crc16 & 0xff);
}

int
Volkslogger::SendCommandReadBulk(Port &port, OperationEnvironment &env,
                                 Command cmd,
                                 void *buffer, unsigned max_length)
{
  return SendCommand(port, env, cmd)
    ? ReadBulk(port, env, buffer, max_length)
    : -1;
}

int
Volkslogger::SendCommandReadBulk(Port &port, OperationEnvironment &env,
                                 Command cmd, uint8_t param1,
                                 void *buffer, unsigned max_length,
                                 unsigned baud_rate)
{
  unsigned old_baud_rate = port.GetBaudrate();

  if (!SendCommandSwitchBaudRate(port, env, cmd, param1, baud_rate))
    return -1;

  /* after switching baud rates, this sleep time is necessary; it has
     been verified experimentally */
  env.Sleep(300);

  int nbytes = ReadBulk(port, env, buffer, max_length);

  port.SetBaudrate(old_baud_rate);

  return nbytes;
}

bool
Volkslogger::SendCommandWriteBulk(Port &port, OperationEnvironment &env,
                                  Command cmd,
                                  const void *data, unsigned size)
{
  if (!SendCommand(port, env, cmd, 0, 0) || !WaitForACK(port, env))
    return false;

  env.Sleep(100);

  return WriteBulk(port, env, data, size) && WaitForACK(port, env);
}
