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

#ifndef XCSOAR_ANDROID_IOIOHELPER_HPP
#define XCSOAR_ANDROID_IOIOHELPER_HPP

#include "Java/Object.hpp"
#include "Java/Ref.hpp"

class PortBridge;

class IOIOHelper : protected Java::Object {
private:
  jmethodID open_mid;
  jmethodID openUart_mid;

public:
  IOIOHelper(JNIEnv *env);
  ~IOIOHelper() {
  }

  /**
   * Open a connection to the IOIO board.
   * Soft resets all ports on board.
   * Waits for up to 3000ms, then fails if IOIO board does not respond
   * @param env
   * @return true if connection to IOIO is established, else false
   */
  bool open(JNIEnv *env) {
    return env->CallBooleanMethod(Get(), open_mid);
  }

  /**
   * Close the connection to the IOIO board.  It can be reopened
   * by calling open().
   * Should not be called until all open Uarts have been closed
   * by calling closeUart();
   * @param env
   */
  void close() {
    CallVoid("shutdown");
  }

  /**
   * Opens a single Uart.  Can only be called after connect()
   * has been called.
   * @param env
   * @param ID Id of Uart (0, 1, 2, 3)
   * @param baud
   * @return True if Uart port on IOIO board is successfully
   * opened.  Else false.
   */
  PortBridge *openUart(JNIEnv *env, unsigned ID, unsigned baud);
};

#endif
