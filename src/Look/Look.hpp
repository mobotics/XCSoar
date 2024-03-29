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

#ifndef XCSOAR_LOOK_HPP
#define XCSOAR_LOOK_HPP

#include "DialogLook.hpp"
#include "TerminalLook.hpp"
#include "UnitsLook.hpp"
#include "VarioLook.hpp"
#include "ChartLook.hpp"
#include "ThermalBandLook.hpp"
#include "TraceHistoryLook.hpp"
#include "MapLook.hpp"
#include "CrossSectionLook.hpp"
#include "HorizonLook.hpp"
#include "TrafficLook.hpp"
#include "Gauge/FlarmTrafficLook.hpp"
#include "InfoBoxLook.hpp"
#include "FinalGlideBarLook.hpp"
#include "IconLook.hpp"

struct UISettings;

struct Look {
  DialogLook dialog;
  TerminalLook terminal;
  UnitsLook units;
  VarioLook vario;
  ChartLook chart;
  ThermalBandLook thermal_band;
  TraceHistoryLook trace_history;
  MapLook map;
  CrossSectionLook cross_section;
  HorizonLook horizon;
  TrafficLook traffic;
  FlarmTrafficLook flarm_gauge;
  FlarmTrafficLook flarm_dialog;
  InfoBoxLook info_box;
  FinalGlideBarLook final_glide_bar;
  IconLook icon;

  void Initialise();
  void InitialiseConfigured(const UISettings &settings);
};

#endif
