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

#ifndef XCSOAR_DEVICE_PORT_HPP
#define XCSOAR_DEVICE_PORT_HPP

#include <stddef.h>

class OperationEnvironment;

/**
 * Generic Port thread handler class
 */
class Port {
public:
  /**
   * Warning: these enum integer values are hard-coded in the
   * Android/Java class InputThread.
   */
  enum class WaitResult {
    /**
     * The port is ready; the desired operation will not block.
     */
    READY,

    /**
     * Timeout has expired.
     */
    TIMEOUT,

    /**
     * An I/O error has occurred, and the port shall not be used.
     */
    FAILED,

    /**
     * The operation was cancelled, probably by
     * OperationEnvironment::IsCancelled().
     */
    CANCELLED,
  };

  /**
   * Interface with callbacks for the #Port class.
   */
  class Handler {
  public:
    virtual void DataReceived(const void *data, size_t length) = 0;
  };

protected:
  Handler &handler;

public:
  Port(Handler &_handler);
  virtual ~Port();

  /**
   * Writes a string to the serial port
   * @param data Pointer to the first character
   * @param length Length of the string
   * @return the number of bytes written, or 0 on error
   */
  virtual size_t Write(const void *data, size_t length) = 0;

  /**
   * Writes a null-terminated string to the serial port
   * @param s The string to write
   * @return the number of bytes written, or 0 on error
   */
  size_t Write(const char *s);

  /**
   * Writes a single byte to the serial port
   * @param ch Byte to write
   */
  bool Write(char ch) {
    return Write(&ch, sizeof(ch)) == sizeof(ch);
  }

  /**
   * Write data to the serial port, take care for partial writes.
   *
   * Note that this port's write timeout is still in effect for each
   * individual write operation.
   *
   * @param timeout_ms give up after this number of milliseconds
   * @return true on success
   */
  bool FullWrite(const void *buffer, size_t length, unsigned timeout_ms);

  /**
   * Just like FullWrite(), but write a null-terminated string
   */
  bool FullWriteString(const char *s, unsigned timeout_ms);

  /**
   * Wait until all data in the output buffer has been sent.
   *
   * @return false on error
   */
  virtual bool Drain() = 0;

  /**
   * Flushes the serial port buffers
   */
  virtual void Flush() = 0;

  /**
   * Sets the RX timeout in ms
   * @param Timeout The receive timeout in ms
   * @return true on success, false on error
   */
  virtual bool SetRxTimeout(unsigned Timeout) = 0;

  /**
   * Sets the baud rate of the serial port to the given value
   * @param BaudRate The desired baudrate
   * @return The previous baud rate or 0 on error
   */
  virtual bool SetBaudrate(unsigned BaudRate) = 0;

  /**
   * Gets the current baud rate of the serial port
   * @return The current baud rate or 0 on error
   */
  virtual unsigned GetBaudrate() const = 0;

  /**
   * Stops the receive thread
   * @return True on success, False on failure
   */
  virtual bool StopRxThread() = 0;

  /**
   * (Re)Starts the receive thread
   * @return True on success, False on failure
   */
  virtual bool StartRxThread() = 0;

  /**
   * Read a single byte from the serial port
   * @return the unsigned byte that was read or -1 on failure
   */
  int GetChar();

  /**
   * Read data from the serial port
   * @param Buffer Pointer to the buffer
   * @param Size Size of the buffer
   * @return Number of bytes read from the serial port or -1 on failure
   */
  virtual int Read(void *Buffer, size_t Size) = 0;

  /**
   * Wait until data becomes available or the timeout expires.
   */
  virtual WaitResult WaitRead(unsigned timeout_ms) = 0;

  /**
   * Force flushing the receive buffers, by trying to read from the
   * port until it times out.
   *
   * The configured read timeout not relevant for this method.
   *
   * @param total_timeout_ms the timeout for each read call [ms]
   * @param total_timeout_ms the maximum total duration of this method [ms]
   * @return true on timeout, false if an error has occurred or the
   * operation was cancelled
   */
  bool FullFlush(OperationEnvironment &env, unsigned timeout_ms,
                 unsigned total_timeout_ms);

  /**
   * Read data from the serial port, take care for partial reads.
   *
   * Note that this port's receive timeout is still in effect for each
   * individual read operation.
   *
   * @param env an OperationEnvironment that allows canceling the
   * operation
   * @param timeout_ms give up after this number of milliseconds
   * @return true on success
   */
  bool FullRead(void *buffer, size_t length, OperationEnvironment &env,
                unsigned timeout_ms);

  /**
   * Wait until data becomes available, the timeout expires or the
   * operation gets cancelled.
   *
   * @param timeout_ms give up after this number of milliseconds
   * @param env an OperationEnvironment that allows cancelling the
   * operation
   */
  WaitResult WaitRead(OperationEnvironment &env, unsigned timeout_ms);

  bool ExpectString(const char *token, OperationEnvironment &env,
                    unsigned timeout_ms = 2000);

  /**
   * Wait until the expected character is received, the timeout expires
   * or the operation gets canceled.
   *
   * @param token The expected character
   * @param env An OperationEnvironment that allows canceling the
   * operation
   * @param timeout_ms give up after this number of milliseconds
   */
  WaitResult WaitForChar(const char token, OperationEnvironment &env,
                         unsigned timeout_ms);
};

#endif
