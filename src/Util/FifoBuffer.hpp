/*
 * Copyright (C) 2003-2010 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XCSOAR_FIFO_BUFFER_HPP
#define XCSOAR_FIFO_BUFFER_HPP

#include "Util/NonCopyable.hpp"
#include "Compiler.h"

#include <utility>
#include <algorithm>

#include <assert.h>

/**
 * A first-in-first-out buffer: you can append data at the end, and
 * read data from the beginning.  This class automatically shifts the
 * buffer as needed.  It is not thread safe.
 */
template<class T, unsigned size>
class FifoBuffer : private NonCopyable {
public:
  struct Range {
    T *data;

    unsigned length;

    Range() = default;

    gcc_constexpr_ctor
    Range(T *_data, unsigned _length):data(_data), length(_length) {}

    gcc_constexpr_method
    bool IsEmpty() const {
      return length == 0;
    }
  };

protected:
  unsigned head, tail;
  T data[size];

public:
  gcc_constexpr_ctor
  FifoBuffer():head(0), tail(0) {}

protected:
  void Shift() {
    if (head == 0)
      return;

    assert(head <= size);
    assert(tail <= size);
    assert(tail >= head);

    std::move(data + head, data + tail, data);

    tail -= head;
    head = 0;
  }

public:
  void Clear() {
    head = tail = 0;
  }

  bool IsEmpty() const {
    return head == tail;
  }

  bool IsFull() const {
    return head == 0 && tail == size;
  }

  /**
   * Prepares writing.  Returns a buffer range which may be written.
   * When you are finished, call append().
   */
  Range Write() {
    Shift();
    return Range(data + tail, size - tail);
  }

  /**
   * Expands the tail of the buffer, after data has been written to
   * the buffer returned by write().
   */
  void Append(unsigned n) {
    assert(tail <= size);
    assert(n <= size);
    assert(tail + n <= size);

    tail += n;
  }

  /**
   * Return a buffer range which may be read.  The buffer pointer is
   * writable, to allow modifications while parsing.
   */
  Range Read() {
    return Range(data + head, tail - head);
  }

  /**
   * Marks a chunk as consumed.
   */
  void Consume(unsigned n) {
    assert(tail <= size);
    assert(head <= tail);
    assert(n <= tail);
    assert(head + n <= tail);

    head += n;
  }
};

#endif
