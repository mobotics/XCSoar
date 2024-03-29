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

#include "Android/InternalSensors.hpp"
#include "Android/Context.hpp"
#include "Atmosphere/Pressure.hpp"
#include "Java/Class.hpp"
#include "org_xcsoar_InternalGPS.h"
#include "org_xcsoar_NonGPSSensors.h"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "OS/Clock.hpp"
#include "Geo/Geoid.hpp"
#include "Compiler.h"

// In Android-targeted InternalSensors implementations, these type identifier
// constants must have the same numerical values as their counterparts in the
// Android API's Sensor class.
const int InternalSensors::TYPE_ACCELEROMETER = 0x1;
const int InternalSensors::TYPE_GYROSCOPE = 0x4;
const int InternalSensors::TYPE_MAGNETIC_FIELD = 0x2;
const int InternalSensors::TYPE_PRESSURE = 0x6;

InternalSensors::InternalSensors(JNIEnv* env, jobject gps_obj, jobject sensors_obj)
    : obj_InternalGPS_(env, gps_obj),
      obj_NonGPSSensors_(env, sensors_obj) {
  // Retrieve method IDs from the InternalGPS object.
  Java::Class gps_cls(env, env->GetObjectClass(gps_obj));
  close_method = env->GetMethodID(gps_cls, "close", "()V");

  // Retrieve method IDs from the NonGPSSensors object.
  Java::Class sensors_cls(env, env->GetObjectClass(sensors_obj));
  mid_sensors_subscribeToSensor_ =
      env->GetMethodID(sensors_cls, "subscribeToSensor", "(I)Z");
  mid_sensors_cancelSensorSubscription_ =
      env->GetMethodID(sensors_cls, "cancelSensorSubscription", "(I)Z");
  mid_sensors_subscribedToSensor_ =
      env->GetMethodID(sensors_cls, "subscribedToSensor", "(I)Z");
  mid_sensors_cancelAllSensorSubscriptions_ =
      env->GetMethodID(sensors_cls, "cancelAllSensorSubscriptions", "()V");
  assert(mid_sensors_subscribeToSensor_ != NULL);
  assert(mid_sensors_cancelSensorSubscription_ != NULL);
  assert(mid_sensors_subscribedToSensor_ != NULL);
  assert(mid_sensors_cancelAllSensorSubscriptions_ != NULL);

  // Import the list of subscribable sensors from the NonGPSSensors object.
  getSubscribableSensors(env, sensors_obj);
}

InternalSensors::~InternalSensors() {
  // Unsubscribe from sensors and the GPS.
  cancelAllSensorSubscriptions();
  JNIEnv *env = Java::GetEnv();
  env->CallVoidMethod(obj_InternalGPS_.Get(), close_method);
}

bool InternalSensors::subscribeToSensor(int id) {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_subscribeToSensor_, (jint) id);
}

bool InternalSensors::cancelSensorSubscription(int id) {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_cancelSensorSubscription_,
                                (jint) id);
}

bool InternalSensors::subscribedToSensor(int id) const {
  JNIEnv* env = Java::GetEnv();
  return env->CallBooleanMethod(obj_NonGPSSensors_.Get(),
                                mid_sensors_subscribedToSensor_, (jint) id);
}

void InternalSensors::cancelAllSensorSubscriptions() {
  JNIEnv* env = Java::GetEnv();
  env->CallVoidMethod(obj_NonGPSSensors_.Get(),
                      mid_sensors_cancelAllSensorSubscriptions_);
}

InternalSensors* InternalSensors::create(JNIEnv* env, Context* context,
                                         unsigned int index) {
  // Construct InternalGPS object.
  Java::Class gps_cls(env, "org/xcsoar/InternalGPS");
  jmethodID gps_ctor_id =
      env->GetMethodID(gps_cls, "<init>", "(Landroid/content/Context;I)V");
  assert(gps_ctor_id != NULL);
  jobject gps_obj =
      env->NewObject(gps_cls, gps_ctor_id, context->Get(), index);
  assert(gps_obj != NULL);

  // Construct NonGPSSensors object.
  Java::Class sensors_cls(env, "org/xcsoar/NonGPSSensors");
  jmethodID sensors_ctor_id =
      env->GetMethodID(sensors_cls, "<init>", "(Landroid/content/Context;I)V");
  assert(sensors_ctor_id != NULL);
  jobject sensors_obj =
      env->NewObject(sensors_cls, sensors_ctor_id, context->Get(), index);
  assert(sensors_obj != NULL);

  InternalSensors *internal_sensors =
      new InternalSensors(env, gps_obj, sensors_obj);
  env->DeleteLocalRef(gps_obj);
  env->DeleteLocalRef(sensors_obj);

  return internal_sensors;
}

// Helper for retrieving the set of sensors to which we can subscribe.
void InternalSensors::getSubscribableSensors(JNIEnv* env, jobject sensors_obj) {
  Java::Class cls(env, env->GetObjectClass(sensors_obj));
  jmethodID mid_sensors_getSubscribableSensors =
      env->GetMethodID(cls, "getSubscribableSensors", "()[I");
  assert(mid_sensors_getSubscribableSensors != NULL);

  jintArray ss_arr = (jintArray) env->CallObjectMethod(
      obj_NonGPSSensors_.Get(), mid_sensors_getSubscribableSensors);
  jsize ss_arr_size = env->GetArrayLength(ss_arr);
  jint* ss_arr_elems = env->GetIntArrayElements(ss_arr, NULL);
  subscribable_sensors_.assign(ss_arr_elems, ss_arr_elems + ss_arr_size);
  env->ReleaseIntArrayElements(ss_arr, ss_arr_elems, 0);
}

/*
 * From here to end: C++ functions called by Java to export GPS and sensor
 * information into XCSoar C++ code.
 */

// Helper for the C++ functions called by Java (below).
inline unsigned int getDeviceIndex(JNIEnv *env, jobject obj) {
  jfieldID fid_index = env->GetFieldID(env->GetObjectClass(obj),
                                       "index", "I");
  return env->GetIntField(obj, fid_index);
}

// Implementations of the various C++ functions called by InternalGPS.java.

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_InternalGPS_setConnected(JNIEnv *env, jobject obj,
                                         jint connected)
{
  unsigned index = getDeviceIndex(env, obj);

  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);

  switch (connected) {
  case 0: /* not connected */
    basic.alive.Clear();
    basic.location_available.Clear();
    break;

  case 1: /* waiting for fix */
    basic.alive.Update(fixed(MonotonicClockMS()) / 1000);
    basic.gps.android_internal_gps = true;
    basic.location_available.Clear();
    break;

  case 2: /* connected */
    basic.alive.Update(fixed(MonotonicClockMS()) / 1000);
    basic.gps.android_internal_gps = true;
    break;
  }

  device_blackboard->ScheduleMerge();
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_InternalGPS_setLocation(JNIEnv *env, jobject obj,
                                        jlong time, jint n_satellites,
                                        jdouble longitude, jdouble latitude,
                                        jboolean hasAltitude, jdouble altitude,
                                        jboolean hasBearing, jdouble bearing,
                                        jboolean hasSpeed, jdouble ground_speed,
                                        jboolean hasAccuracy, jdouble accuracy,
                                        jboolean hasAcceleration, jdouble acceleration)
{
  unsigned index = getDeviceIndex(env, obj);

  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  basic.UpdateClock();
  basic.alive.Update(basic.clock);

  BrokenDateTime date_time = BrokenDateTime::FromUnixTimeUTC(time / 1000);
  fixed second_of_day = fixed(date_time.GetSecondOfDay()) +
    /* add the millisecond fraction of the original timestamp for
       better accuracy */
    fixed((unsigned)(time % 1000)) / 1000u;

  if (second_of_day < basic.time &&
      basic.date_available &&
      (BrokenDate)date_time > (BrokenDate)basic.date_time_utc)
    /* don't wrap around when going past midnight in UTC */
    second_of_day += fixed(24u * 3600u);

  basic.time = second_of_day;
  basic.time_available.Update(basic.clock);
  basic.date_time_utc = date_time;
  basic.date_available = true;

  basic.gps.satellites_used = n_satellites;
  basic.gps.satellites_used_available.Update(basic.clock);
  basic.gps.real = true;
  basic.gps.android_internal_gps = true;
  basic.location = GeoPoint(Angle::Degrees(fixed(longitude)),
                            Angle::Degrees(fixed(latitude)));
  basic.location_available.Update(basic.clock);

  if (hasAltitude) {
    fixed GeoidSeparation = EGM96::LookupSeparation(basic.location);
    basic.gps_altitude = fixed(altitude) - GeoidSeparation;
    basic.gps_altitude_available.Update(basic.clock);
  } else
    basic.gps_altitude_available.Clear();

  if (hasBearing) {
    basic.track = Angle::Degrees(fixed(bearing));
    basic.track_available.Update(basic.clock);
  } else
    basic.track_available.Clear();

  if (hasSpeed) {
    basic.ground_speed = fixed(ground_speed);
    basic.ground_speed_available.Update(basic.clock);
  }

  if (hasAccuracy)
    basic.gps.hdop = fixed(accuracy);

  if (hasAcceleration)
    basic.acceleration.ProvideGLoad(fixed(acceleration), true);

  device_blackboard->ScheduleMerge();
}

// Implementations of the various C++ functions called by NonGPSSensors.java.

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NonGPSSensors_setAcceleration(
    JNIEnv* env, jobject obj, jfloat ddx, jfloat ddy, jfloat ddz) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NonGPSSensors_setRotation(
    JNIEnv* env, jobject obj,
    jfloat dtheta_x, jfloat dtheta_y, jfloat dtheta_z) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NonGPSSensors_setMagneticField(
    JNIEnv* env, jobject obj, jfloat h_x, jfloat h_y, jfloat h_z) {
  // TODO
  /*
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  */
}

gcc_visibility_default
JNIEXPORT void JNICALL
Java_org_xcsoar_NonGPSSensors_setBarometricPressure(
    JNIEnv* env, jobject obj, jfloat pressure) {
  const unsigned int index = getDeviceIndex(env, obj);
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  basic.ProvideStaticPressure(
      AtmosphericPressure::HectoPascal(fixed(pressure)));
  device_blackboard->ScheduleMerge();
}
