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

#include "Form/TabBar.hpp"
#include "Form/TabDisplay.hpp"
#include "Screen/PaintWindow.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Screen/Canvas.hpp"
#include "Asset.hpp"

#include <assert.h>
#include <winuser.h>

TabBarControl::TabBarControl(ContainerWindow &_parent, const DialogLook &look,
                             PixelScalar x, PixelScalar y,
                             UPixelScalar _width, UPixelScalar _height,
                             const WindowStyle style, bool _flipOrientation,
                             bool _clientOverlapTabs)
  :tab_display(NULL),
   tab_line_height((Layout::landscape ^ _flipOrientation)
                 ? (Layout::Scale(TabLineHeightInitUnscaled) * 0.75)
                 : Layout::Scale(TabLineHeightInitUnscaled)),
   flip_orientation(_flipOrientation),
   client_overlap_tabs(_clientOverlapTabs),
   page_flipped_callback(NULL)
{
  set(_parent, 0, 0, _parent.get_width(), _parent.get_height(), style),

  tab_display = new TabDisplay(*this, look, *this,
                                 x, y, _width, _height,
                                 flip_orientation);

  PixelRect rc = get_client_rect();
  if (!_clientOverlapTabs) {
    if (Layout::landscape ^ flip_orientation)
      rc.left += tab_display->GetTabWidth();
    else
      rc.top += tab_display->GetTabHeight();
  }

  pager.Move(rc);
}

TabBarControl::~TabBarControl()
{
  delete tab_display;

  reset();
}

void
TabBarControl::SetClientOverlapTabs(bool value)
{
  if (client_overlap_tabs == value)
    return;

  client_overlap_tabs = value;

  PixelRect rc = get_client_rect();
  if (!client_overlap_tabs) {
    if (Layout::landscape ^ flip_orientation)
      rc.left += tab_display->GetTabWidth();
    else
      rc.top += tab_display->GetTabHeight();
  }

  pager.Move(rc);
}

const TCHAR*
TabBarControl::GetButtonCaption(unsigned i) const
{
  return tab_display->GetCaption(i);
}

unsigned
TabBarControl::AddTab(Widget *widget, const TCHAR *caption,
                      bool button_only, const Bitmap *bmp)
{
  pager.Add(widget);
  tab_display->Add(caption, button_only, bmp);
  return GetTabCount() - 1;
}

void
TabBarControl::ClickPage(unsigned i)
{
  const bool is_current = i == pager.GetCurrentIndex();
  if (!pager.ClickPage(i) || is_current)
    /* failure */
    return;

  if (tab_display != NULL)
    tab_display->Invalidate();

  if (!is_current)
    /* switching to a new page by mouse click focuses the first
       control of the page, which is important for Altair hot keys */
    pager.SetFocus();

  if (page_flipped_callback != NULL)
    page_flipped_callback();
}

void
TabBarControl::SetCurrentPage(unsigned i)
{
  if (i == pager.GetCurrentIndex())
    /* no-op */
    return;

  if (!pager.SetCurrent(i))
    /* failed to switch */
    return;

  if (tab_display != NULL)
    tab_display->Invalidate();

  if (page_flipped_callback != NULL)
    page_flipped_callback();
}

void
TabBarControl::NextPage()
{
  if (!pager.Next(HasPointer()))
    /* failed to switch */
    return;

  if (tab_display != NULL)
    tab_display->Invalidate();

  if (page_flipped_callback != NULL)
    page_flipped_callback();
}

void
TabBarControl::PreviousPage()
{
  if (!pager.Previous(HasPointer()))
    /* failed to switch */
    return;

  if (tab_display != NULL)
    tab_display->Invalidate();

  if (page_flipped_callback != NULL)
    page_flipped_callback();
}

UPixelScalar
TabBarControl::GetTabHeight() const
{
  return tab_display->GetTabHeight();
}

UPixelScalar
TabBarControl::GetTabWidth() const
{
  return tab_display->GetTabWidth();
}

void
TabBarControl::OnCreate()
{
  ContainerWindow::OnCreate();

  const PixelRect rc = get_client_rect();
  pager.Initialise(*this, rc);
  pager.Prepare(*this, rc);
  pager.Show(rc);
}

void
TabBarControl::OnDestroy()
{
  pager.Hide();
  pager.Unprepare();

  ContainerWindow::OnDestroy();
}
