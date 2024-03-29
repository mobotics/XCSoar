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

#ifndef XCSOAR_ROW_FORM_WIDGET_HPP
#define XCSOAR_ROW_FORM_WIDGET_HPP

#include "WindowWidget.hpp"
#include "Edit.hpp"
#include "DataField/Base.hpp"
#include "Util/StaticArray.hpp"
#include "Units/Settings.hpp"

#include <assert.h>
#include <stdint.h>

struct DialogLook;
struct StaticEnumChoice;
class ActionListener;

/**
 * A #Widget that contains #WndProperty controls, one in a row.
 * Derive from this class, and construct the controls in Initialise()
 * or Prepare().  Don't forget to call the base class in these two
 * methods!
 */
class RowFormWidget : public WindowWidget {
  struct Row {
    enum class Type : uint8_t {
      /**
       * A dummy entry without a #Window.  Its height is zero.
       */
      DUMMY,

      /**
       * A generic #Window.
       */
      GENERIC,

      /**
       * A #WndProperty.
       */
      EDIT,

      /**
       * A multi-line #WndProperty.
       */
      MULTI_LINE,

      /**
       * A #WndButton.
       */
      BUTTON,

      /**
       * A generic #Window that fills the remaining vertical space at
       * the bottom.  It must be the last row, and there can only be
       * one.
       */
      REMAINING,
    };

    Type type;

    /**
     * Shall this row be available?  If not, it is hidden and no
     * screen space is reserved for it.
     */
    bool available;

    /**
     * Shall this row be visible?  The "expert" flag overrides it in
     * global "non-expert" mode.
     */
    bool visible;

    /**
     * If true, then the row is only visible in "expert" mode.
     */
    bool expert;

    Window *window;

    Row() = default;

    Row(Type _type)
      :type(_type), available(true), visible(false), expert(false),
       window(NULL) {
      assert(_type == Type::DUMMY);
    }

    Row(Type _type, Window *_window)
      :type(_type), available(true), visible(true), expert(false),
       window(_window) {
      assert(_type != Type::DUMMY);
      assert(_window != NULL);
    }

    /**
     * Delete the #Window object.
     */
    void Delete() {
      delete window;
    }

    gcc_pure
    Window &GetWindow() {
      assert(window != NULL);

      return *window;
    }

    gcc_pure
    const Window &GetWindow() const {
      assert(window != NULL);

      return *window;
    }

    gcc_pure
    WndProperty &GetControl() {
      assert(type == Type::EDIT);
      assert(window != NULL);

      return *(WndProperty *)window;
    }

    gcc_pure
    const WndProperty &GetControl() const {
      assert(type == Type::EDIT);
      assert(window != NULL);

      return *(WndProperty *)window;
    }

    /**
     * Will this row grow when there is excess screen space?
     */
    bool IsElastic() const {
      return GetMaximumHeight() > GetMinimumHeight();
    }

    gcc_pure
    UPixelScalar GetMinimumHeight() const;

    gcc_pure
    UPixelScalar GetMaximumHeight() const;
  };

  const DialogLook &look;

  StaticArray<Row, 32u> rows;

public:
  RowFormWidget(const DialogLook &look);
  virtual ~RowFormWidget();

protected:
  const DialogLook &GetLook() const {
    return look;
  }

  void Add(Row::Type type, Window *window);

  WndProperty *CreateEdit(const TCHAR *label, const TCHAR *help=NULL,
                          bool read_only=false);

public:
  /**
   * Add a "dummy" row.  It does not have a #Window and its height is
   * zero.  This may be used to "reserve" a row index when a row is
   * not present in a certain instance of the form.
   */
  void AddDummy() {
    rows.push_back(Row::Type::DUMMY);
  }

  void Add(Window *window) {
    Add(Row::Type::GENERIC, window);
  }

  /**
   * Add a #Window that fills the remaining vertical space at the
   * bottom.  It must be the last row, and there can only be one.
   */
  void AddRemaining(Window *window) {
    Add(Row::Type::REMAINING, window);
  }

  WndProperty *Add(const TCHAR *label, const TCHAR *help=NULL,
                   bool read_only=false);

  /**
   * Add a read-only control.  You can use SetText() to update its
   * text.
   */
  void AddReadOnly(const TCHAR *label, const TCHAR *help=NULL,
                   const TCHAR *text=NULL);

  WndProperty *Add(const TCHAR *label, const TCHAR *help,
                   DataField *df);

  WndProperty *AddBoolean(const TCHAR *label, const TCHAR *help,
                          bool value=false,
                          DataField::DataAccessCallback callback=NULL);

  WndProperty *AddBoolean(const TCHAR *label, const TCHAR *help,
                          bool value, DataFieldListener *listener) {
    WndProperty *control = AddBoolean(label, help, value);
    control->GetDataField()->SetListener(listener);
    return control;
  }

  WndProperty *AddInteger(const TCHAR *label, const TCHAR *help,
                          const TCHAR *display_format,
                          const TCHAR *edit_format,
                          int min_value, int max_value, int step, int value,
                          DataField::DataAccessCallback callback=NULL);

  WndProperty *AddInteger(const TCHAR *label, const TCHAR *help,
                          const TCHAR *display_format,
                          const TCHAR *edit_format,
                          int min_value, int max_value, int step, int value,
                          DataFieldListener *listener) {
    WndProperty *control = AddInteger(label, help, display_format, edit_format,
                                      min_value, max_value, step, value);
    control->GetDataField()->SetListener(listener);
    return control;
  }

  WndProperty *AddFloat(const TCHAR *label, const TCHAR *help,
                        const TCHAR *display_format,
                        const TCHAR *edit_format,
                        fixed min_value, fixed max_value,
                        fixed step, bool fine,
                        fixed value,
                        DataField::DataAccessCallback callback=NULL);

  WndProperty *AddFloat(const TCHAR *label, const TCHAR *help,
                        const TCHAR *display_format,
                        const TCHAR *edit_format,
                        fixed min_value, fixed max_value,
                        fixed step, bool fine,
                        UnitGroup unit_group, fixed value,
                        DataField::DataAccessCallback callback=NULL);

  WndProperty *AddEnum(const TCHAR *label, const TCHAR *help,
                       const StaticEnumChoice *list, unsigned value=0,
                       DataField::DataAccessCallback callback=NULL);

  WndProperty *AddEnum(const TCHAR *label, const TCHAR *help,
                       const StaticEnumChoice *list, unsigned value,
                       DataFieldListener *listener) {
    WndProperty *control = AddEnum(label, help, list, value);
    control->GetDataField()->SetListener(listener);
    return control;
  }

  WndProperty *AddEnum(const TCHAR *label, const TCHAR *help,
                       DataField::DataAccessCallback callback=NULL);

  WndProperty *AddEnum(const TCHAR *label, const TCHAR *help,
                       DataFieldListener *listener) {
    WndProperty *control = AddEnum(label, help);
    control->GetDataField()->SetListener(listener);
    return control;
  }

  WndProperty *AddText(const TCHAR *label, const TCHAR *help,
                       const TCHAR *content);

  /**
   * Add a password edit control.  The password is obfuscated while
   * not editing.
   */
  WndProperty *AddPassword(const TCHAR *label, const TCHAR *help,
                           const TCHAR *content);

  WndProperty *AddTime(const TCHAR *label, const TCHAR *help,
                       int min_value, int max_value, unsigned step,
                       int value, unsigned max_tokens = 2,
                       DataField::DataAccessCallback callback=NULL);

  WndProperty *AddTime(const TCHAR *label, const TCHAR *help,
                       int min_value, int max_value, unsigned step,
                       int value, unsigned max_tokens,
                       DataFieldListener *listener) {
    WndProperty *control = AddTime(label, help, min_value, max_value, step,
                                   value, max_tokens);
    control->GetDataField()->SetListener(listener);
    return control;
  }

  WndProperty *AddSpacer();

  WndProperty *AddFileReader(const TCHAR *label, const TCHAR *help,
                             const TCHAR *registry_key, const TCHAR *filters,
                             bool nullable = true);

  /**
   * Add a read-only multi-line control.  You can use
   * SetMultiLineText() to update its text.
   */
  void AddMultiLine(const TCHAR *label, const TCHAR *help=NULL,
                    const TCHAR *text=NULL);

  void AddButton(const TCHAR *label, ActionListener *listener=NULL, int id=0);

  gcc_pure
  Window &GetRow(unsigned i) {
    return rows[i].GetWindow();
  }

  gcc_pure
  const Window &GetRow(unsigned i) const {
    return rows[i].GetWindow();
  }

  /**
   * Modify the "available" flag on this row.
   */
  void SetRowAvailable(unsigned i, bool available);

  void SetRowVisible(unsigned i, bool visible);

  void ShowRow(unsigned i) {
    SetRowVisible(i, true);
  }

  void HideRow(unsigned i) {
    SetRowVisible(i, false);
  }

  /**
   * Enable the "expert" flag on this row: hide unless the form is in
   * "expert" mode.  This must be called before the first Show()
   * invocation.
   */
  void SetExpertRow(unsigned i);

  gcc_pure
  Window &GetGeneric(unsigned i) {
    return rows[i].GetWindow();
  }

  gcc_pure
  WndProperty &GetControl(unsigned i) {
    return rows[i].GetControl();
  }

  gcc_pure
  const WndProperty &GetControl(unsigned i) const {
    return rows[i].GetControl();
  }

  /**
   * Update the text of a multi line control.
   */
  void SetText(unsigned i, const TCHAR *text) {
    assert(text != NULL);

    WndProperty &control = GetControl(i);
    assert(control.GetDataField() == NULL);
    control.SetText(text);
  }

  /**
   * Update the text of a multi line control.
   */
  void SetMultiLineText(unsigned i, const TCHAR *text) {
    assert(text != NULL);
    assert(rows[i].type == Row::Type::MULTI_LINE);

    WndProperty &control = *(WndProperty *)rows[i].window;
    assert(control.GetDataField() == NULL);
    control.SetText(text);
  }

  gcc_pure
  DataField &GetDataField(unsigned i) {
    DataField *df = GetControl(i).GetDataField();
    assert(df != NULL);
    return *df;
  }

  gcc_pure
  const DataField &GetDataField(unsigned i) const {
    const DataField *df = GetControl(i).GetDataField();
    assert(df != NULL);
    return *df;
  }

  /**
   * Compare a row's data field with the given reference (by their
   * addresses).
   */
  gcc_pure
  bool IsDataField(unsigned i, const DataField &df) const {
    return &df == &GetDataField(i);
  }

  void LoadValue(unsigned i, int value);
  void LoadValue(unsigned i, bool value);
  void LoadValueEnum(unsigned i, int value);

  template<typename T>
  void LoadValueEnum(unsigned i, T value) {
    LoadValueEnum(i, (int)value);
  }

  void LoadValue(unsigned i, fixed value);
  void LoadValue(unsigned i, fixed value, UnitGroup unit_group);

  gcc_pure
  bool GetValueBoolean(unsigned i) const;

  gcc_pure
  int GetValueInteger(unsigned i) const;

  gcc_pure
  fixed GetValueFloat(unsigned i) const;

  gcc_pure
  const TCHAR *GetValueString(unsigned i) const {
    return GetDataField(i).GetAsString();
  }

  bool SaveValue(unsigned i, bool &value, bool negated = false) const;
  bool SaveValue(unsigned i, int &value) const;
  bool SaveValue(unsigned i, uint16_t &value) const;
  bool SaveValue(unsigned i, fixed &value) const;
  bool SaveValue(unsigned i, TCHAR *string, size_t max_size) const;
  bool SaveValue(unsigned i, const TCHAR *registry_key, TCHAR *string, size_t max_size) const;

  bool SaveValue(unsigned i, unsigned &value) const {
    return SaveValue(i, (int &)value);
  }

  bool SaveValue(unsigned i, const TCHAR *registry_key, bool &value, bool negated = false) const;
  bool SaveValue(unsigned i, const TCHAR *registry_key, int &value) const;
  bool SaveValue(unsigned i, const TCHAR *registry_key, uint16_t &value) const;
  bool SaveValue(unsigned i, const TCHAR *registry_key, fixed &value) const;

  bool SaveValue(unsigned i, const TCHAR *registry_key,
                 unsigned &value) const {
    return SaveValue(i, registry_key, (int &)value);
  }

  bool SaveValue(unsigned i, UnitGroup unit_group, fixed &value) const;

  bool SaveValue(unsigned i, UnitGroup unit_group,
                 const TCHAR *registry_key, fixed &value) const;

  bool SaveValue(unsigned i, UnitGroup unit_group,
                 const TCHAR *registry_key, unsigned int &value) const;

  template<typename T>
  bool SaveValueEnum(unsigned i, T &value) const {
    if (sizeof(T) == sizeof(int))
      return SaveValue(i, (int &)value);

    int value2 = (int)value;
    if (!SaveValue(i, value2))
      return false;

    value = (T)value2;
    return true;
  }

  template<typename T>
  bool SaveValueEnum(unsigned i, const TCHAR *registry_key, T &value) const {

    int value2 = (int)value;
    if (!SaveValue(i, registry_key, value2))
      return false;

    value = (T)value2;
    return true;
  }

  bool SaveValueFileReader(unsigned i, const TCHAR *registry_key);

protected:
  gcc_pure
  UPixelScalar GetRecommendedCaptionWidth() const;

  void NextControlRect(PixelRect &rc, UPixelScalar height) {
    assert(IsDefined());

    rc.top = rc.bottom;
    rc.bottom = rc.top + height;
  }

  gcc_pure
  PixelRect InitialControlRect(UPixelScalar height) {
    assert(IsDefined());

    PixelRect rc = GetWindow()->get_client_rect();
    rc.bottom = rc.top + height;
    return rc;
  }

  /**
   * Recalculate all button positions.
   */
  void UpdateLayout();

public:
  /* virtual methods from Widget */
  virtual PixelSize GetMinimumSize() const;
  virtual PixelSize GetMaximumSize() const;
  virtual void Initialise(ContainerWindow &parent, const PixelRect &rc);
  virtual void Show(const PixelRect &rc);
  virtual void Move(const PixelRect &rc);
  virtual bool SetFocus();
};

#endif
