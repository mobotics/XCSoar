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

#include "K6BtPort.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "Message.hpp"
#include "Asset.hpp"
#include "OS/Sleep.h"

#include <assert.h>
#include <tchar.h>
#include <stdio.h>

K6BtPort::K6BtPort(Port *_port, unsigned _baud_rate, Handler &_handler)
  :Port(_handler), port(_port), baud_rate(_baud_rate)
{
  SetBaudrate(baud_rate);
}

K6BtPort::~K6BtPort()
{
  delete port;
}

void
K6BtPort::SendCommand(uint8_t cmd)
{
  const uint8_t data[2] = { ESCAPE, cmd };
  port->Write(data, sizeof(data));
}

size_t
K6BtPort::Write(const void *_data, size_t length)
{
  /* in order to forward the buffer verbatim to the real device, we
     have to escape all ESCAPE bytes (i.e. send each of them twice) */

  const uint8_t *data = (const uint8_t *)_data;

  size_t total = 0;

  const uint8_t *p;
  while ((p = (const uint8_t *)memchr(data, ESCAPE, length)) != NULL) {
    size_t chunk = p - data + 1;
    size_t nbytes = port->Write(data, chunk);
    total += nbytes;
    if (nbytes != chunk)
      return total;

    /* write the ESCAPE byte again (but don't consider it in the
       return value) */
    if (port->Write(p, 1) != 1)
      return total;

    ++p;

    length = data + length - p;
    data = p;
  }

  if (length > 0)
    total += port->Write(data, length);

  return total;
}

bool
K6BtPort::Drain()
{
  return port->Drain();
}

void
K6BtPort::Flush()
{
  port->Flush();

  SendCommand(FLUSH_BUFFERS | 0x3); /* flush RX and TX buffer */
}

bool
K6BtPort::SetRxTimeout(unsigned timeout_ms)
{
  return port->SetRxTimeout(timeout_ms);
}

gcc_const
static int
BaudRateToK6Bt(unsigned baud_rate)
{
  switch (baud_rate) {
  case 2400:
    return 0x0;

  case 4800:
    return 0x1;

  case 9600:
    return 0x2;

  case 19200:
    return 0x3;

  case 38400:
    return 0x4;

  case 57600:
    return 0x5;

  case 115200:
    return 0x6;

  default:
    return -1;
  }
}

bool
K6BtPort::SetBaudrate(unsigned _baud_rate)
{
  int code = BaudRateToK6Bt(_baud_rate);
  if (code < 0)
    /* not supported by K6Bt */
    return false;

  SendCommand(CHANGE_BAUD_RATE | code);
  return true;
}

unsigned
K6BtPort::GetBaudrate() const
{
  return baud_rate;
}

bool
K6BtPort::StopRxThread()
{
  return port->StopRxThread();
}

bool
K6BtPort::StartRxThread(void)
{
  return port->StartRxThread();
}

int
K6BtPort::Read(void *Buffer, size_t Size)
{
  return port->Read(Buffer, Size);
}

Port::WaitResult
K6BtPort::WaitRead(unsigned timeout_ms)
{
  return port->WaitRead(timeout_ms);
}
