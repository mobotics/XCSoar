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

#include "FinalGlideBarRenderer.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/TextInBox.hpp"
#include "NMEA/Derived.hpp"
#include "Look/FinalGlideBarLook.hpp"
#include "Look/TaskLook.hpp"
#include "Formatter/UserUnits.hpp"
#include "Util/Macros.hpp"
#include "Screen/Fonts.hpp"

void
FinalGlideBarRenderer::Draw(Canvas &canvas, const PixelRect &rc,
                            const DerivedInfo &calculated,
                            const bool &final_glide_bar_mc0_enabled) const
{
  RasterPoint GlideBar[6] = {
      { 0, 0 }, { 9, -9 }, { 18, 0 }, { 18, 0 }, { 9, 0 }, { 0, 0 }
  };
  RasterPoint GlideBar0[4] = {
      { 0, 0 }, { 9, -9 }, { 9, 0 }, { 0, 0 }
  };
  RasterPoint clipping_arrow[6] = {
      { 0, 0 }, { 9, 9 }, { 18, 0 }, { 18, 6 }, { 9, 15 }, { 0, 6 }
  };
  RasterPoint clipping_arrow0[4] = {
      { 0, 0 }, { 9, 9 }, { 9, 15 }, { 0, 6 }
  };

  TCHAR Value[10];

  if (!calculated.task_stats.task_valid ||
      !calculated.task_stats.total.solution_remaining.IsOk() ||
      !calculated.task_stats.total.solution_mc0.IsDefined())
    return;

  const int y0 = (rc.bottom + rc.top) / 2;

  PixelScalar dy_glidebar = 0;
  PixelScalar dy_glidebar0 = 0;

  FormatUserAltitude(calculated.task_stats.total.solution_remaining.altitude_difference,
                            Value, false);
  canvas.Select(Fonts::map_bold);
  const PixelSize text_size = canvas.CalcTextSize(Value);

  PixelScalar clipping_arrow_offset = Layout::Scale(4);
  PixelScalar clipping_arrow0_offset = Layout::Scale(4);

  // 468 meters is it's size. Will be divided by 9 to fit screen resolution.
  int altitude_difference = ((int)calculated.task_stats.total.solution_remaining.altitude_difference);
  int altitude_difference0 = ((int)calculated.task_stats.total.solution_mc0.altitude_difference);
  // TODO feature: should be an angle if in final glide mode

  // cut altitude_difference at +- 468 meters (55 units)
  if (altitude_difference > 468)
    altitude_difference = 468;
  if (altitude_difference < -468)
    altitude_difference = -468;

  // 55 units is size, 468 meters div by 9 means 55.
  int Offset = altitude_difference / 9;
  
  Offset = Layout::Scale(Offset);
  if (altitude_difference <= 0) {
    GlideBar[1].y = Layout::Scale(9);
    dy_glidebar = text_size.cy + 2;
  } else {
    GlideBar[1].y = -Layout::Scale(9);
    clipping_arrow[1].y = -clipping_arrow[1].y;
    clipping_arrow[3].y = -clipping_arrow[3].y;
    clipping_arrow[4].y = -clipping_arrow[4].y;
    clipping_arrow[5].y = -clipping_arrow[5].y;
    clipping_arrow_offset = -clipping_arrow_offset;
    dy_glidebar = -1;
  }

  // cut altitude_difference0 at +- 468 meters (55 units)
  if (altitude_difference0 > 468)
    altitude_difference0 = 468;
  if (altitude_difference0 < -468)
    altitude_difference0 = -468;

  // 55 units is size, therefore div by 9.
  int Offset0 = altitude_difference0 / 9;
  
  Offset0 = Layout::Scale(Offset0);
  if (altitude_difference0 <= 0) {
    GlideBar0[1].y = Layout::Scale(9);
    dy_glidebar0 = text_size.cy + 2;
  } else {
    GlideBar0[1].y = -Layout::Scale(9);
    clipping_arrow0[1].y = -clipping_arrow0[1].y;
    clipping_arrow0[2].y = -clipping_arrow0[2].y;
    clipping_arrow0[3].y = -clipping_arrow0[3].y;
    clipping_arrow0_offset = -clipping_arrow0_offset;
    dy_glidebar0 = -1;
  }

  for (unsigned i = 0; i < 6; i++) {
    GlideBar[i].y += y0 + dy_glidebar;
    GlideBar[i].x = Layout::Scale(GlideBar[i].x) + rc.left;
  }

  GlideBar[0].y -= Offset;
  GlideBar[1].y -= Offset;
  GlideBar[2].y -= Offset;

  for (unsigned i = 0; i < 4; i++) {
    GlideBar0[i].y += y0 + dy_glidebar0;
    GlideBar0[i].x = Layout::Scale(GlideBar0[i].x) + rc.left;
  }

  GlideBar0[0].y -= Offset0;
  GlideBar0[1].y -= Offset0;

  if ((altitude_difference0 >= altitude_difference) && (altitude_difference > 0)) {
    // both above and mc0 arrow larger than mc arrow
    GlideBar0[2].y = GlideBar[1].y;    // both below
    GlideBar0[3].y = GlideBar[0].y;
  }

  // prepare clipping arrow
  for (unsigned i = 0; i < 6; i++) {
    clipping_arrow[i].y = Layout::Scale(clipping_arrow[i].y) + y0 - Offset
      + clipping_arrow_offset + dy_glidebar;
    clipping_arrow[i].x = Layout::Scale(clipping_arrow[i].x) + rc.left;
  }

  // prepare clipping arrow mc0
  for (unsigned i = 0; i < 4; i++) {
    clipping_arrow0[i].y = Layout::Scale(clipping_arrow0[i].y) + y0 - Offset0
      + clipping_arrow0_offset + dy_glidebar0;
    clipping_arrow0[i].x = Layout::Scale(clipping_arrow0[i].x) + rc.left;
  }

  // draw actual glide bar
  if (altitude_difference <= 0) {
    if (calculated.common_stats.landable_reachable) {
      canvas.Select(look.pen_below_landable);
      canvas.Select(look.brush_below_landable);
    } else {
      canvas.Select(look.pen_below);
      canvas.Select(look.brush_below);
    }
  } else {
    canvas.Select(look.pen_above);
    canvas.Select(look.brush_above);
  }
  canvas.polygon(GlideBar, 6);

  // draw clipping arrow
  if ((altitude_difference <= -468 ) || (altitude_difference >= 468))
    canvas.polygon(clipping_arrow, 6);

  // draw glide bar at mc 0
  if (altitude_difference0 <= 0 && final_glide_bar_mc0_enabled) {
    if (calculated.common_stats.landable_reachable) {
      canvas.Select(look.pen_below_landable);
      canvas.Select(look.brush_below_landable_mc0);
    } else {
      canvas.Select(look.pen_below);
      canvas.Select(look.brush_below_mc0);
    }
  } else {
    canvas.Select(look.pen_above);
    canvas.Select(look.brush_above_mc0);
  }

  if ( ( (altitude_difference != altitude_difference0) || (altitude_difference0 < 0) )
      && final_glide_bar_mc0_enabled) {
    canvas.polygon(GlideBar0, 4);

    if ((altitude_difference0 <= -468 ) || (altitude_difference0 >= 468))
      canvas.polygon(clipping_arrow0, 4);
  }

  // draw cross (x) on final glide bar if unreachable at current Mc
  // or above final glide but impeded by obstacle
  int cross_sign = 0;

  if (!calculated.task_stats.total.IsAchievable())
    cross_sign = 1;
  if (calculated.terrain_warning && (altitude_difference>0))
    cross_sign = -1;

  if (cross_sign != 0) {
    canvas.Select(task_look.bearing_pen);
    canvas.line(Layout::Scale(9 - 5), y0 + cross_sign * Layout::Scale(9 - 5),
                Layout::Scale(9 + 5), y0 + cross_sign * Layout::Scale(9 + 5));
    canvas.line(Layout::Scale(9 - 5), y0 + cross_sign * Layout::Scale(9 + 5),
                Layout::Scale(9 + 5), y0 + cross_sign * Layout::Scale(9 - 5));
  }

  canvas.SetTextColor(COLOR_BLACK);
  canvas.SetBackgroundColor(COLOR_WHITE);

  TextInBoxMode style;
  style.mode = RM_ROUNDED_BLACK;
  style.bold = true;
  style.move_in_view = true;

  if (text_size.cx < Layout::Scale(18)) {
    style.align = A_RIGHT;
    TextInBox(canvas, Value, Layout::Scale(18), y0, style, rc);
  } else
    TextInBox(canvas, Value, 0, y0, style, rc);

}
