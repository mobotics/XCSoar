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

#ifndef XCSOAR_SCREEN_WINDOW_HPP
#define XCSOAR_SCREEN_WINDOW_HPP

#include "Util/NonCopyable.hpp"
#include "Screen/Font.hpp"
#include "Screen/Point.hpp"
#include "Screen/Features.hpp"
#include "Thread/Debug.hpp"
#include "Compiler.h"

#include <assert.h>

#ifdef USE_GDI
#include <windows.h>
#endif /* GDI */

#ifdef ANDROID
struct Event;
#endif

class Canvas;
class ContainerWindow;
class WindowTimer;

/**
 * A portable wrapper for describing a window's style settings on
 * creation.
 */
class WindowStyle {
#ifndef USE_GDI
protected:
  bool visible;
  bool enabled;
  bool tab_stop, control_parent;
  bool double_clicks;
  bool has_border;
  int text_style;

public:
  gcc_constexpr_ctor
  WindowStyle()
    :visible(true), enabled(true),
     tab_stop(false), control_parent(false),
     double_clicks(false), has_border(false),
     text_style(0) {}

#else /* USE_GDI */
protected:
  DWORD style, ex_style;
  bool double_clicks;
  bool custom_painting;

#ifdef _WIN32_WCE
  /* workaround for gcc optimization bug on ARM/XScale */
  bool dummy0, dummy1;
#endif

public:
  gcc_constexpr_ctor
  WindowStyle()
    :style(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS),
     ex_style(0), double_clicks(false), custom_painting(false)
#ifdef _WIN32_WCE
    , dummy0(0), dummy1(0)
#endif
  {}
#endif /* USE_GDI */

  /** The window is initially not visible. */
  void Hide() {
#ifndef USE_GDI
    visible = false;
#else
    style &= ~WS_VISIBLE;
#endif
  }

  /**
   * The window is initially disabled.
   * A disabled window cannot receive input from the user.
   */
  void Disable() {
#ifndef USE_GDI
    enabled = false;
#else
    style |= WS_DISABLED;
#endif
  }

  /**
   * The window is a control that can receive the keyboard focus when the
   * user presses the TAB key. Pressing the TAB key changes the keyboard
   * focus to the next control with the WS_TABSTOP style.
   */
  void TabStop() {
#ifndef USE_GDI
    tab_stop = true;
#else
    style |= WS_TABSTOP;
#endif
  }

  /**
   * If the search for the next control with the WS_TABSTOP style encounters
   * a window with the WS_EX_CONTROLPARENT style, the system recursively
   * searches the window's children.
   */
  void ControlParent() {
#ifndef USE_GDI
    control_parent = true;
#else
    ex_style |= WS_EX_CONTROLPARENT;
#endif
  }

  /** The window has a thin-line border. */
  void Border() {
#ifdef USE_GDI
    style |= WS_BORDER;
#else
    has_border = true;
#endif
  }

  /** The window has a sunken 3D border. */
  void SunkenEdge() {
    Border();
#ifdef USE_GDI
    ex_style |= WS_EX_CLIENTEDGE;
#endif
  }

  /** The window has a vertical scroll bar. */
  void VerticalScroll() {
#ifdef USE_GDI
    style |= WS_VSCROLL;
#endif
  }

  void Popup() {
#ifdef USE_GDI
    style &= ~WS_CHILD;
    style |= WS_SYSMENU;
#endif
  }

  void EnableCustomPainting() {
#ifdef USE_GDI
    custom_painting = true;
#endif
  }

  void EnableDoubleClicks() {
    double_clicks = true;
  }

  friend class Window;
};

/**
 * A Window is a portion on the screen which displays something, and
 * which optionally interacts with the user.  To draw custom graphics
 * into a Window, derive your class from #PaintWindow.
 */
class Window : private NonCopyable {
  friend class ContainerWindow;

protected:
#ifndef USE_GDI
  ContainerWindow *parent;

private:
  PixelScalar left, top;
  UPixelScalar width, height;

private:
  const Font *font;
  int text_style;

  bool tab_stop, control_parent;

  bool visible;
  bool enabled;
  bool focused;
  bool capture;
  bool has_border;
#else
  HWND hWnd;

private:
  WNDPROC prev_wndproc;
#endif

private:
  bool double_clicks;
  bool custom_painting;

public:
#ifndef USE_GDI
  Window()
    :parent(NULL), width(0), height(0),
     font(NULL),
     visible(true), focused(false), capture(false), has_border(false),
     double_clicks(false) {}
#else
  Window():hWnd(NULL), prev_wndproc(NULL),
           double_clicks(false), custom_painting(false) {}
#endif
  virtual ~Window();

  /**
   * Activates the OnPaint() method.  It is disabled by default
   * because its preparation would needlessly allocate resources.
   */
  void EnableCustomPainting() {
#ifdef USE_GDI
    custom_painting = true;
#endif
  }

#ifndef USE_GDI
  const ContainerWindow *GetParent() const {
    return parent;
  }
#else
  operator HWND() const {
    return hWnd;
  };

  /**
   * Is it this window?
   */
  gcc_pure
  bool identify(HWND h) const {
    return h == hWnd;
  }

  /**
   * Is it this window or one of its descendants?
   */
  gcc_pure
  bool identify_descendant(HWND h) const {
    return h == hWnd || ::IsChild(hWnd, h);
  }
#endif

protected:
  /**
   * Assert that the current thread is the one which created this
   * window.
   */
#ifdef NDEBUG
  void AssertThread() const {}
  void AssertThreadOrUndefined() const {}
#else
  void AssertThread() const;
  void AssertThreadOrUndefined() const;
#endif

#ifdef USE_GDI
  bool GetCustomPainting() const {
    return custom_painting;
  }
#endif

#ifndef USE_GDI
  bool HasBorder() const {
    return has_border;
  }
#endif

public:
  bool IsDefined() const {
#ifndef USE_GDI
    return width > 0;
#else
    return hWnd != NULL;
#endif
  }

#ifndef USE_GDI
  PixelScalar get_top() const {
    return top;
  }

  PixelScalar get_left() const {
    return left;
  }

  UPixelScalar get_width() const {
    return width;
  }

  UPixelScalar get_height() const {
    return height;
  }

  PixelScalar get_right() const {
    return get_left() + get_width();
  }

  PixelScalar get_bottom() const {
    return get_top() + get_height();
  }

  int get_text_style() const {
    return text_style;
  }
#else /* USE_GDI */
  UPixelScalar get_width() const {
    return get_size().cx;
  }

  UPixelScalar get_height() const {
    return get_size().cy;
  }
#endif

#ifndef USE_GDI
  void set(ContainerWindow *parent,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle window_style=WindowStyle());

  void set(ContainerWindow *parent, const PixelRect rc,
           const WindowStyle window_style=WindowStyle()) {
    set(parent, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        window_style);
  }
#else
  void set(ContainerWindow *parent, const TCHAR *cls, const TCHAR *text,
           PixelScalar left, PixelScalar top,
           UPixelScalar width, UPixelScalar height,
           const WindowStyle window_style=WindowStyle());

  void set(ContainerWindow *parent, const TCHAR *cls, const TCHAR *text,
           const PixelRect rc, const WindowStyle window_style=WindowStyle()) {
    set(parent, cls, text, rc.left, rc.top,
        rc.right - rc.left, rc.bottom - rc.top,
        window_style);
  }

  /**
   * Create a message-only window.
   */
  void CreateMessageWindow();
#endif

#ifdef USE_GDI
  void created(HWND _hWnd);
#endif

  void reset();

  /**
   * Determines the root owner window of this Window.  This is
   * probably a pointer to the #MainWindow instance.
   */
  gcc_pure
  ContainerWindow *GetRootOwner();

  void move(PixelScalar left, PixelScalar top) {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    this->left = left;
    this->top = top;
    Invalidate();
#else
    ::SetWindowPos(hWnd, NULL, left, top, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER |
                   SWP_NOACTIVATE | SWP_NOOWNERZORDER);
#endif
  }

  void move(PixelScalar left, PixelScalar top,
            UPixelScalar width, UPixelScalar height) {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    move(left, top);
    resize(width, height);
#else /* USE_GDI */
    ::SetWindowPos(hWnd, NULL, left, top, width, height,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    // XXX store new size?
#endif
  }

  void move(const PixelRect rc) {
    move(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
  }

  /**
   * Like move(), but does not trigger a synchronous redraw.  The
   * caller is responsible for redrawing.
   */
  void fast_move(PixelScalar left, PixelScalar top,
                 UPixelScalar width, UPixelScalar height) {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    move(left, top, width, height);
#else /* USE_GDI */
    ::SetWindowPos(hWnd, NULL, left, top, width, height,
                   SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_DEFERERASE |
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
#endif
  }

  /**
   * Move the Window to the specified position within the parent
   * ContainerWindow and make it visible.
   */
  void MoveAndShow(const PixelRect rc) {
    assert_none_locked();
    AssertThread();

#ifdef USE_GDI
    ::SetWindowPos(hWnd, NULL,
                   rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                   SWP_SHOWWINDOW | SWP_NOACTIVATE |
                   SWP_NOZORDER | SWP_NOOWNERZORDER);
#else
    move(rc);
    Show();
#endif
  }

  void resize(UPixelScalar width, UPixelScalar height) {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    if (width == this->width && height == this->height)
      return;

    this->width = width;
    this->height = height;

    Invalidate();
    OnResize(width, height);
#else /* USE_GDI */
    ::SetWindowPos(hWnd, NULL, 0, 0, width, height,
                   SWP_NOMOVE | SWP_NOZORDER |
                   SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    // XXX store new size?
#endif
  }

#ifndef USE_GDI
  void BringToTop();
  void BringToBottom();
#else
  void BringToTop() {
    assert_none_locked();
    AssertThread();

    /* not using BringWindowToTop() because it activates the
       winddow */
    ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOMOVE|SWP_NOSIZE|
                   SWP_NOACTIVATE|SWP_NOOWNERZORDER);
  }

  void BringToBottom() {
    assert_none_locked();
    AssertThread();

    ::SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0,
                   SWP_NOMOVE|SWP_NOSIZE|
                   SWP_NOACTIVATE|SWP_NOOWNERZORDER);
  }
#endif

  void show_on_top() {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    BringToTop();
    Show();
#else
    ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|
                   SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif
  }

  void set_font(const Font &_font) {
    assert_none_locked();
    AssertThread();

#ifndef USE_GDI
    font = &_font;
    Invalidate();
#else
    ::SendMessage(hWnd, WM_SETFONT,
                  (WPARAM)_font.Native(), MAKELPARAM(TRUE,0));
#endif
  }

  /**
   * Determine whether this Window is visible.  This method disregards
   * the visibility of parent windows, it just checks if the "visible"
   * flag is set for this Window.
   */
  gcc_pure
  bool is_visible() const {
#ifndef USE_GDI
    return visible;
#else
    return (get_window_style() & WS_VISIBLE) != 0;
#endif
  }

#ifndef USE_GDI
  void Show();
  void Hide();
#else
  void Show() {
    AssertThread();

    ::ShowWindow(hWnd, SW_SHOW);
  }

  void Hide() {
    AssertThread();

    ::ShowWindow(hWnd, SW_HIDE);
  }
#endif

  /**
   * Like Hide(), but does not trigger a synchronous redraw of the
   * parent window's background.
   */
  void FastHide() {
    AssertThread();

#ifndef USE_GDI
    Hide();
#else
    ::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                   SWP_HIDEWINDOW |
                   SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_DEFERERASE |
                   SWP_NOMOVE | SWP_NOSIZE |
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
#endif
  }

  void set_visible(bool visible) {
    if (visible)
      Show();
    else
      Hide();
  }

#ifndef USE_GDI
  bool is_tab_stop() const {
    return tab_stop;
  }

  bool is_control_parent() const {
    return control_parent;
  }
#endif

  /**
   * Can this window get user input?
   */
  gcc_pure
  bool is_enabled() const {
#ifndef USE_GDI
    return enabled;
#else
    return ::IsWindowEnabled(hWnd);
#endif
  }

  /**
   * Specifies whether this window can get user input.
   */
#ifdef USE_GDI
  void SetEnabled(bool enabled) {
    AssertThread();

    ::EnableWindow(hWnd, enabled);
  }
#else
  void SetEnabled(bool enabled);
#endif

#ifndef USE_GDI

  virtual Window *GetFocusedWindow();
  virtual void SetFocus();

  /**
   * Called by the parent window when this window loses focus, or when
   * one of its (indirect) child windows loses focus.  This method is
   * responsible for invoking OnKillFocus().
   */
  virtual void ClearFocus();

#else /* USE_GDI */

  void SetFocus() {
    assert_none_locked();
    AssertThread();

    ::SetFocus(hWnd);
  }

#endif /* USE_GDI */

  gcc_pure
  bool has_focus() const {
#ifndef USE_GDI
    return focused;
#else
    return hWnd == ::GetFocus();
#endif
  }

#ifndef USE_GDI
  void SetCapture();
  void ReleaseCapture();
  virtual void ClearCapture();
#else /* USE_GDI */

  void SetCapture() {
    assert_none_locked();
    AssertThread();

    ::SetCapture(hWnd);
  }

  void ReleaseCapture() {
    assert_none_locked();
    AssertThread();

    ::ReleaseCapture();
  }

  WNDPROC set_wndproc(WNDPROC wndproc)
  {
    AssertThread();

    return (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)wndproc);
  }

  void set_userdata(void *value)
  {
    AssertThread();

    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)value);
  }
#endif /* USE_GDI */

#ifndef USE_GDI
  void ToScreen(PixelRect &rc) const;
#endif

  /**
   * Returns the position on the screen.
   */
  gcc_pure
  const PixelRect get_screen_position() const
  {
    PixelRect rc;
#ifndef USE_GDI
    rc = get_position();
    ToScreen(rc);
#else
    ::GetWindowRect(hWnd, &rc);
#endif
    return rc;
  }

  /**
   * Returns the position within the parent window.
   */
  gcc_pure
  const PixelRect get_position() const
  {
    PixelRect rc;
#ifndef USE_GDI
    rc.left = get_left();
    rc.top = get_top();
    rc.right = get_right();
    rc.bottom = get_bottom();
#else
    rc = get_screen_position();

    HWND parent = ::GetParent(hWnd);
    if (parent != NULL) {
      POINT pt;

      pt.x = rc.left;
      pt.y = rc.top;
      ::ScreenToClient(parent, &pt);
      rc.left = pt.x;
      rc.top = pt.y;

      pt.x = rc.right;
      pt.y = rc.bottom;
      ::ScreenToClient(parent, &pt);
      rc.right = pt.x;
      rc.bottom = pt.y;
    }
#endif
    return rc;
  }

  gcc_pure
  const PixelRect get_client_rect() const
  {
    PixelRect rc;
#ifndef USE_GDI
    rc.left = 0;
    rc.top = 0;
    rc.right = get_width();
    rc.bottom = get_height();
#else
    ::GetClientRect(hWnd, &rc);
#endif
    return rc;
  }

  gcc_pure
  const PixelSize get_size() const
  {
    PixelRect rc = get_client_rect();
    PixelSize s;
    s.cx = rc.right;
    s.cy = rc.bottom;
    return s;
  }

  /**
   * Returns the parent's client area rectangle.
   */
#ifdef USE_GDI
  gcc_pure
  PixelRect GetParentClientRect() const {
    HWND hParent = ::GetParent(hWnd);
    assert(hParent != NULL);

    PixelRect rc;
    ::GetClientRect(hParent, &rc);
    return rc;
  }
#else
  gcc_pure
  PixelRect GetParentClientRect() const;
#endif

#ifndef USE_GDI
  void Setup(Canvas &canvas);

  virtual void Invalidate();

  /**
   * Ensures that the window is updated on the physical screen.
   */
  virtual void Expose();
#else /* USE_GDI */
  HDC BeginPaint(PAINTSTRUCT *ps) {
    AssertThread();

    return ::BeginPaint(hWnd, ps);
  }

  void EndPaint(PAINTSTRUCT *ps) {
    AssertThread();

    ::EndPaint(hWnd, ps);
  }

  void scroll(PixelScalar dx, PixelScalar dy, const PixelRect &rc) {
    ::ScrollWindowEx(hWnd, dx, dy, &rc, NULL, NULL, NULL, SW_INVALIDATE);
  }

  gcc_const
  static void *get_userdata(HWND hWnd) {
    return (void *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
  }

  /**
   * Converts a #HWND into a #Window pointer, without checking if that
   * is legal.
   */
  gcc_const
  static Window *get_unchecked(HWND hWnd) {
    return (Window *)get_userdata(hWnd);
  }

  /**
   * Converts a #HWND into a #Window pointer.  Returns NULL if the
   * HWND is not a Window peer.  This only works for windows which
   * have called InstallWndProc().
   */
  gcc_const
  static Window *get(HWND hWnd) {
    WNDPROC wndproc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    return wndproc == WndProc
#ifdef _WIN32_WCE
      /* Windows CE seems to put WNDPROC pointers into some other
         segment (0x22000000 added); this is a dirty workaround which
         will be implemented properly once we understand what this
         really means */
      || ((DWORD)wndproc & 0xffffff) == (DWORD)WndProc
#endif
      ? get_unchecked(hWnd)
      : NULL;
  }

  gcc_pure
  LONG get_window_long(int nIndex) const {
    return ::GetWindowLong(hWnd, nIndex);
  }

  void SetWindowLong(int nIndex, LONG value) {
    ::SetWindowLong(hWnd, nIndex, value);
  }

  LONG get_window_style() const {
    return get_window_long(GWL_STYLE);
  }

  void SetWindowStyle(LONG value) {
    SetWindowLong(GWL_STYLE, value);
  }

  LONG get_window_ex_style() const {
    return get_window_long(GWL_EXSTYLE);
  }

  void SetWindowExStyle(LONG value) {
    SetWindowLong(GWL_EXSTYLE, value);
  }
#endif

#ifndef USE_GDI
  void SendUser(unsigned id);
#else
  void SendUser(unsigned id) {
    ::PostMessage(hWnd, WM_USER + id, (WPARAM)0, (LPARAM)0);
  }
#endif

protected:
#ifndef USE_GDI
public:
#endif /* !USE_GDI */
  /**
   * @return true on success, false if the window should not be
   * created
   */
  virtual void OnCreate();
  virtual void OnDestroy();
  virtual bool OnClose();
  virtual void OnResize(UPixelScalar width, UPixelScalar height);
  virtual bool OnMouseMove(PixelScalar x, PixelScalar y, unsigned keys);
  virtual bool OnMouseDown(PixelScalar x, PixelScalar y);
  virtual bool OnMouseUp(PixelScalar x, PixelScalar y);
  virtual bool OnMouseDouble(PixelScalar x, PixelScalar y);
  virtual bool OnMouseWheel(PixelScalar x, PixelScalar y, int delta);

#ifdef HAVE_MULTI_TOUCH
  /**
   * A secondary pointer is being pressed.
   */
  virtual bool OnMultiTouchDown();

  /**
   * A secondary pointer is being released.
   */
  virtual bool OnMultiTouchUp();
#endif

  /**
   * Checks if the window wishes to handle a special key, like cursor
   * keys and tab.  This wraps the WIN32 message WM_GETDLGCODE.
   *
   * @return true if the window will handle they key, false if the
   * dialog manager may use it
   */
  gcc_pure
  virtual bool OnKeyCheck(unsigned key_code) const;

  virtual bool OnKeyDown(unsigned key_code);
  virtual bool OnKeyUp(unsigned key_code);
  virtual bool OnCommand(unsigned id, unsigned code);
  virtual bool OnCancelMode();
  virtual void OnSetFocus();
  virtual void OnKillFocus();
  virtual bool OnTimer(WindowTimer &timer);
  virtual bool OnUser(unsigned id);

  virtual void OnPaint(Canvas &canvas);
  virtual void OnPaint(Canvas &canvas, const PixelRect &dirty);

#ifdef USE_GDI
  /**
   * Called by OnMessage() when the message was not handled by any
   * virtual method.  Calls the default handler.  This function is
   * virtual, because the Dialog class will have to override it -
   * dialogs have slightly different semantics.
   */
  virtual LRESULT OnUnhandledMessage(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam);

  virtual LRESULT OnMessage(HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam);
#endif /* USE_GDI */

public:
#ifndef USE_GDI
  void InstallWndProc() {
    // XXX
  }
#else /* USE_GDI */
  /**
   * This static method reads the Window* object from GWL_USERDATA and
   * calls OnMessage().
   */
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam);

  /**
   * Installs Window::WndProc() has the WNDPROC.  This enables the
   * methods on_*() methods, which may be implemented by sub classes.
   */
  void InstallWndProc();
#endif /* USE_GDI */
};

#endif
