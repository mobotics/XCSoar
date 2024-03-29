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

#include "DataField/Enum.hpp"
#include "DataField/ComboList.hpp"
#include "Form/Edit.hpp"
#include "Form/Util.hpp"
#include "Form/List.hpp"
#include "Form/DockWindow.hpp"
#include "Widgets/DeviceEditWidget.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Dialogs/Dialogs.h"
#include "Interface.hpp"
#include "Profile/DeviceConfig.hpp"
#include "Device/Register.hpp"
#include "Device/List.hpp"
#include "Device/Parser.hpp"
#include "Device/Driver.hpp"
#include "Device/device.hpp"
#include "Asset.hpp"
#include "Protection.hpp"
#include "DevicesConfigPanel.hpp"
#include "Language/Language.hpp"
#include "Compatibility/string.h"
#include "Util/Macros.hpp"
#include "Form/XMLWidget.hpp"
#include "Screen/Layout.hpp"
#include "Dialogs/CallBackTable.hpp"

class DevicesConfigPanel
  : public XMLWidget, private ListControl::Handler,
    private DeviceEditWidget::Listener {
  unsigned current_device;
  bool current_modified;

  gcc_pure
  DeviceEditWidget &GetEditWidget() {
    DockWindow *dock = (DockWindow *)form.FindByName(_T("edit"));
    assert(dock != NULL);
    return *(DeviceEditWidget *)dock->GetWidget();
  }

  gcc_pure
  const DeviceEditWidget &GetEditWidget() const {
    const DockWindow *dock = (const DockWindow *)form.FindByName(_T("edit"));
    assert(dock != NULL);
    return *(const DeviceEditWidget *)dock->GetWidget();
  }

  bool SaveDeviceConfig();

public:
  const DeviceConfig &GetDeviceConfig(unsigned i) const {
    assert(i < NUMDEV);

    return CommonInterface::GetSystemSettings().devices[i];
  }

  const DeviceConfig &GetListItemConfig(unsigned i) const {
    assert(i < NUMDEV);

    return i == current_device
      ? GetEditWidget().GetConfig()
      : GetDeviceConfig(i);
  }

  void SetDeviceConfig(unsigned i, const DeviceConfig &config) const {
    assert(i < NUMDEV);

    CommonInterface::SetSystemSettings().devices[i] = config;
    Profile::SetDeviceConfig(i, config);
    DevicePortChanged = true;
  }

  void ShowDevice(unsigned idx);

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Move(const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* virtual methods from List::Handler */
  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned idx);
  virtual void OnCursorMoved(unsigned index);
  virtual bool CanActivateItem(unsigned index) const;
  virtual void OnActivateItem(unsigned index);

  /* virtual methods from DeviceEditWidget::Listener */
  virtual void OnModified(DeviceEditWidget &widget);
};

bool
DevicesConfigPanel::SaveDeviceConfig()
{
  bool changed = current_modified, require_restart = false;
  DeviceEditWidget &widget = GetEditWidget();
  if (!widget.Save(changed, require_restart))
    return false;

  if (changed)
    SetDeviceConfig(current_device, widget.GetConfig());

  current_modified = false;
  return true;
}

void
DevicesConfigPanel::ShowDevice(unsigned idx)
{
  assert(idx < NUMDEV);

  if (idx == current_device)
    return;

  if (!SaveDeviceConfig())
    return;

  current_device = idx;
  current_modified = false;
  GetEditWidget().SetConfig(GetDeviceConfig(current_device));
}

bool
DevicesConfigPanel::CanActivateItem(unsigned index) const
{
  return true;
}

void
DevicesConfigPanel::OnActivateItem(unsigned idx)
{
  ShowDevice(idx);
  GetEditWidget().SetFocus();
}

void
DevicesConfigPanel::OnCursorMoved(unsigned idx)
{
  ShowDevice(idx);
}

void
DevicesConfigPanel::OnPaintItem(Canvas &canvas, const PixelRect rc,
                                unsigned idx)
{
  const DeviceConfig &config = GetListItemConfig(idx);

  const UPixelScalar margin = Layout::Scale(2);

  TCHAR buffer1[256], buffer2[256];
  const TCHAR *name = config.GetPortName(buffer1, 128);

  if (config.UsesDriver()) {
    const struct DeviceRegister *driver = FindDriverByName(config.driver_name);
    const TCHAR *driver_name = (driver != NULL) ? driver->display_name :
                                                  config.driver_name.c_str();

    buffer2[0] = TCHAR('A' + idx);
    buffer2[1] = _T(':');
    buffer2[2] = _T(' ');

    _sntprintf(buffer2 + 3, 128, _("%s on %s"),
               driver_name, name);
    name = buffer2;
  } else {
    _sntprintf(buffer2, 128, _T("%c: %s"),
               TCHAR('A' + idx), name);
    name = buffer2;
  }

  canvas.text(rc.left + margin, rc.top + margin, name);
}

void
DevicesConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  LoadWindow(NULL, parent,
             Layout::landscape ? _T("IDR_XML_DEVICESCONFIGPANEL") :
                               _T("IDR_XML_DEVICESCONFIGPANEL_L"));

  current_device = 0;

  DockWindow *dock = (DockWindow *)form.FindByName(_T("edit"));
  assert(dock != NULL);
  DeviceEditWidget *edit = new DeviceEditWidget(GetDeviceConfig(0));
  edit->SetListener(this);
  dock->SetWidget(edit);

  ListControl *list = (ListControl *)form.FindByName(_T("list"));
  assert(list != NULL);
  list->SetHandler(this);
  list->SetLength(NUMDEV);
}

void
DevicesConfigPanel::Move(const PixelRect &rc)
{
  XMLWidget::Move(rc);

  /* update "expert" rows (hack) */
  DockWindow *dock = (DockWindow *)form.FindByName(_T("edit"));
  assert(dock != NULL);
  dock->MoveWidget();
}

bool
DevicesConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  if (!SaveDeviceConfig())
    return false;

  if (DevicePortChanged)
    changed = true;

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

void
DevicesConfigPanel::OnModified(DeviceEditWidget &widget)
{
  bool changed = false, require_restart = false;
  if (GetEditWidget().Save(changed, require_restart) && changed) {
    current_modified = true;
    ListControl *list = (ListControl *)form.FindByName(_T("list"));
    list->Invalidate();
  }
}

Widget *
CreateDevicesConfigPanel()
{
  return new DevicesConfigPanel();
}
