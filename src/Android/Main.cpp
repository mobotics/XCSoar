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

#include "Android/Main.hpp"
#include "Android/Context.hpp"
#include "Android/NativeView.hpp"
#include "Android/SoundUtil.hpp"
#include "Android/Vibrator.hpp"
#include "Android/PortBridge.hpp"
#include "Android/NativeInputListener.hpp"
#include "Language/Language.hpp"
#include "LocalPath.hpp"
#include "Screen/Debug.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Android/Event.hpp"
#include "Screen/OpenGL/Init.hpp"
#include "Simulator.hpp"
#include "Profile/Profile.hpp"
#include "MainWindow.hpp"
#include "Interface.hpp"
#include "Java/Global.hpp"
#include "Compiler.h"
#include "org_xcsoar_NativeView.h"

#ifdef IOIOLIB
#include "Android/IOIOHelper.hpp"
#endif

#ifndef NDEBUG
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Buffer.hpp"
#endif

#include <assert.h>

Context *context;

NativeView *native_view;

EventQueue *event_queue;

SoundUtil *sound_util;

Vibrator *vibrator;
bool os_haptic_feedback_enabled;

#ifdef IOIOLIB
IOIOHelper *ioio_helper;
#endif

gcc_visibility_default
JNIEXPORT jboolean JNICALL
Java_org_xcsoar_NativeView_initializeNative(JNIEnv *env, jobject obj,
                                            jobject _context,
                                            jint width, jint height,
                                            jint xdpi, jint ydpi,
                                            jint sdk_version, jstring product)
{
  Java::Init(env);

  NativeInputListener::Initialise(env);
  PortBridge::Initialise(env);

  context = new Context(env, _context);

  InitialiseDataPath();

  OpenGL::Initialise();

  assert(native_view == NULL);
  native_view = new NativeView(env, obj, width, height, xdpi, ydpi,
                               sdk_version, product);

  event_queue = new EventQueue();

  sound_util = new SoundUtil(env);
  vibrator = Vibrator::Create(env, *context);

#ifdef IOIOLIB
  ioio_helper = new IOIOHelper(env);
#endif

  ScreenInitialized();
  AllowLanguage();
  return XCSoarInterface::Startup();
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_runNative(JNIEnv *env, jobject obj)
{
  OpenGL::Initialise();

  CommonInterface::main_window.event_loop();
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_deinitializeNative(JNIEnv *env, jobject obj)
{
  CommonInterface::main_window.reset();
  DisallowLanguage();
  Fonts::Deinitialize();

#ifdef IOIOLIB
  delete ioio_helper;
  ioio_helper = NULL;
#endif

  delete sound_util;
  delete event_queue;
  event_queue = NULL;
  delete native_view;

  OpenGL::Deinitialise();
  ScreenDeinitialized();
  DeinitialiseDataPath();

  delete context;
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_resizedNative(JNIEnv *env, jobject obj,
                                         jint width, jint height)
{
  if (event_queue == NULL)
    return;

  CommonInterface::main_window.AnnounceResize(width, height);

  event_queue->Purge(Event::RESIZE);

  Event event(Event::RESIZE, width, height);
  event_queue->Push(event);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_pauseNative(JNIEnv *env, jobject obj)
{
  if (event_queue == NULL)
    /* pause before we have initialized the event subsystem does not
       work - let's bail out, nothing is lost anyway */
    exit(0);

  CommonInterface::main_window.pause();

  assert(num_textures == 0);
  assert(num_buffers == 0);
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_resumeNative(JNIEnv *env, jobject obj)
{
  if (event_queue == NULL)
    /* there is nothing here yet which can be resumed */
    exit(0);

  CommonInterface::main_window.resume();
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NativeView_setHapticFeedback(JNIEnv *env, jobject obj,
                                             jboolean on)
{
  os_haptic_feedback_enabled = on;
}
