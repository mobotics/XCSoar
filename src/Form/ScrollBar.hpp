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

#ifndef XCSOAR_FORM_SCROLL_BAR_HPP
#define XCSOAR_FORM_SCROLL_BAR_HPP

#include "Screen/Point.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include <winuser.h>
#endif

class Window;
class Canvas;

class ScrollBar {
protected:
  /** Whether the slider is currently being dragged */
  bool dragging;
  int drag_offset;
  /** Coordinates of the ScrollBar */
  PixelRect rc;
  /** Coordinates of the Slider */
  PixelRect rc_slider;

public:
  /** Constructor of the ScrollBar class */
  ScrollBar();

  /** Returns the width of the ScrollBar */
  PixelScalar GetWidth() const {
    return rc.right - rc.left;
  }

  /** Returns the height of the ScrollBar */
  PixelScalar GetHeight() const {
    return rc.bottom - rc.top;
  }

  /** Returns the height of the slider */
  PixelScalar GetSliderHeight() const {
    return rc_slider.bottom - rc_slider.top;
  }

  /** Returns the height of the scrollable area of the ScrollBar */
  PixelScalar GetNettoHeight() const {
    return GetHeight() - 2 * GetWidth() - 1;
  }

  /**
   * Returns the height of the visible scroll area of the ScrollBar
   * (the area thats not covered with the slider)
   */
  PixelScalar GetScrollHeight() const {
    return GetNettoHeight() - GetSliderHeight();
  }

  /**
   * Returns whether the ScrollBar is defined or has to be set up first
   * @return True if the ScrollBar is defined,
   * False if it has to be set up first
   */
  bool IsDefined() const {
    return GetWidth() > 0;
  }

  /**
   * Returns the x-Coordinate of the ScrollBar
   * (remaining client area aside the ScrollBar)
   * @param size Size of the client area including the ScrollBar
   * @return The x-Coordinate of the ScrollBar
   */
  UPixelScalar GetLeft(const PixelSize size) const {
    return IsDefined() ? rc.left : size.cx;
  }

  /**
   * Returns whether the given RasterPoint is in the ScrollBar area
   * @param pt RasterPoint to check
   * @return True if the given RasterPoint is in the ScrollBar area,
   * False otherwise
   */
  bool IsInside(const RasterPoint &pt) const {
    return ::PtInRect(&rc, pt);
  }

  /**
   * Returns whether the given RasterPoint is in the slider area
   * @param pt RasterPoint to check
   * @return True if the given RasterPoint is in the slider area,
   * False otherwise
   */
  bool IsInsideSlider(const RasterPoint &pt) const {
    return ::PtInRect(&rc_slider, pt);
  }

  /**
   * Returns whether the given y-Coordinate is on the up arrow
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is on the up arrow,
   * False otherwise
   */
  bool IsInsideUpArrow(PixelScalar y) const {
    return y < rc.top + GetWidth();
  }

  /**
   * Returns whether the given y-Coordinate is on the down arrow
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is on the down arrow,
   * False otherwise
   */
  bool IsInsideDownArrow(PixelScalar y) const {
    return y >= rc.bottom - GetWidth();
  }

  /**
   * Returns whether the given y-Coordinate is above the slider area
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is above the slider area,
   * False otherwise
   */
  bool IsAboveSlider(PixelScalar y) const {
    return y < rc_slider.top;
  }

  /**
   * Returns whether the given y-Coordinate is below the slider area
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is below the slider area,
   * False otherwise
   */
  bool IsBelowSlider(PixelScalar y) const {
    return y >= rc_slider.bottom;
  }

  /**
   * Sets the size of the ScrollBar
   * (actually just the height, width is automatically set)
   * @param size Size of the Control the ScrollBar is used with
   */
  void SetSize(const PixelSize size);

  /** Resets the ScrollBar (undefines it) */
  void Reset();

  /** Calculates the size and position of the slider */
  void SetSlider(unsigned size, unsigned view_size, unsigned origin);

  /** Calculates the new origin out of the given y-Coordinate of the drag */
  unsigned ToOrigin(unsigned size, unsigned view_size, PixelScalar y) const;

  /** Paints the ScollBar */
  void Paint(Canvas &canvas) const;

  /**
   * Returns whether the slider is currently being dragged
   * @return True if the slider is currently being dragged, False otherwise
   */
  bool IsDragging() const {
    return dragging;
  }

  /**
   * Should be called when beginning to drag
   * (Called by ListControl::OnMouseDown)
   * @param w The Window object the ScrollBar is belonging to
   * @param y y-Coordinate
   */
  void DragBegin(Window *w, UPixelScalar y);

  /**
   * Should be called when stopping to drag
   * (Called by ListControl::OnMouseUp)
   * @param w The Window object the ScrollBar is belonging to
   */
  void DragEnd(Window *w);

  /**
   * Should be called while dragging
   * @param size Size of the Scrollbar (not pixelwise)
   * @param view_size Visible size of the Scrollbar (not pixelwise)
   * @param y y-Coordinate
   * @return "Value" of the ScrollBar
   */
  unsigned DragMove(unsigned  size, unsigned view_size, PixelScalar y) const;
};

#endif
