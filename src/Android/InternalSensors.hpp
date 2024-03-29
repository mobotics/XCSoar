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

#ifndef XCSOAR_ANDROID_INTERNAL_SENSORS_HPP
#define XCSOAR_ANDROID_INTERNAL_SENSORS_HPP

#include "Java/Object.hpp"
#include "Compiler.h"

#include <jni.h>
#include <vector>

class Context;

// Consolidated class for handling Java objects that work with Android GPS
// and sensor facilities. Public methods handle activation and deactivation of
// specific sensors.
class InternalSensors {
 private:
  // Java objects working with the GPS and the other sensors respectively.
  Java::Object obj_InternalGPS_;
  Java::Object obj_NonGPSSensors_;

  // IDs for methods in InternalGPS.java.
  jmethodID close_method;

  // IDs for methods in NonGPSSensors.java.
  jmethodID mid_sensors_subscribeToSensor_;
  jmethodID mid_sensors_cancelSensorSubscription_;
  jmethodID mid_sensors_subscribedToSensor_;
  jmethodID mid_sensors_cancelAllSensorSubscriptions_;
  std::vector<int> subscribable_sensors_;

  InternalSensors(JNIEnv* env, jobject gps_obj, jobject sensors_obj);
  void getSubscribableSensors(JNIEnv* env, jobject sensors_obj);
 public:
  ~InternalSensors();

  // Sensor type identifier constants for use with subscription methods below.
  static const int TYPE_ACCELEROMETER;
  static const int TYPE_GYROSCOPE;
  static const int TYPE_MAGNETIC_FIELD;
  static const int TYPE_PRESSURE;

  // For information on these methods, see comments around analogous methods
  // in NonGPSSensors.java.
  const std::vector<int>& getSubscribableSensors() const {
    return subscribable_sensors_;
  }
  bool subscribeToSensor(int id);
  bool cancelSensorSubscription(int id);
  bool subscribedToSensor(int id) const;
  void cancelAllSensorSubscriptions();

  gcc_malloc
  static InternalSensors *create(JNIEnv* env, Context* native_view,
                               unsigned int index);
};

#endif
