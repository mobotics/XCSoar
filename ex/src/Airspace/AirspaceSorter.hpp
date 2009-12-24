#ifndef AIRSPACE_SORTER_HPP
#define AIRSPACE_SORTER_HPP

#include "Airspace/Airspaces.hpp"
#include "Airspace/AirspaceClass.hpp"
#include <vector>

/**
 * Structure to hold Airspace sorting information
 */
struct AirspaceSelectInfo {
  const AbstractAirspace* airspace; /**< Pointer to actual airspace (unprotected!) */
  fixed Distance; /**< Distance in user units from observer to waypoint */
  fixed Direction; /**< Bearing (deg true north) from observer to waypoint */
  unsigned int FourChars; /**< Fast access of first four characters of name */
};

typedef std::vector<AirspaceSelectInfo> AirspaceSelectInfoVector;

/**
 * Utility class to manage sorting of waypoints (e.g. for dlgWayPointSelect)
 * Note that distance queries do not yet make use of the kdtree, but this system
 * keeps a local master list so won't need to lock the waypoints for long.
 */

class AirspaceSorter
{
public:
/** 
 * Constructor.  Sorts master list of waypoints by name 
 * 
 * @param _airspaces Airspaces store
 * @param Location Location of aircraft at time of query
 * @param distance_factor Units factor to apply to distance calculations
 */
  AirspaceSorter(const Airspaces &_airspaces, 
                 const GEOPOINT &Location,
                 const fixed distance_factor);

/** 
 * Return master list
 * 
 * @return Master list
 */
  const AirspaceSelectInfoVector& get_list();

/** 
 * Remove airspaces not of specified class
 * 
 * @param vec List of airspaces to filter (read-write)
 * @param t Class of airspace to match
 */
  void filter_class(AirspaceSelectInfoVector& vec, const AirspaceClass_t t) const;

/** 
 * Remove airspaces not matching supplied initial character 
 * 
 * @param vec List of airspaces to filter (read-write)
 * @param c Character to match
 */
  void filter_name(AirspaceSelectInfoVector& vec, const unsigned char c) const;

/** 
 * Remove airspaces bearing greater than 18 degrees from supplied direction 
 * 
 * @param vec List of airspaces to filter (read-write)
 * @param direction Bearing (degrees) of desired direction
 */
  void filter_direction(AirspaceSelectInfoVector& vec, const fixed direction) const;

/** 
 * Remove airspaces further than desired direction
 * 
 * @param vec List of airspaces to filter (read-write)
 * @param distance Distance (user units) of limit
 */
  void filter_distance(AirspaceSelectInfoVector& vec, const fixed distance) const;

/** 
 * Sort airspaces by distance
 * 
 * @param vec List of airspaces to sort (read-write)
 */
  void sort_distance(AirspaceSelectInfoVector& vec) const;

/** 
 * Sort airspaces alphabetically
 * 
 * @param vec List of airspaces to sort (read-write)
 */
  void sort_name(AirspaceSelectInfoVector& vec) const;

private:
  AirspaceSelectInfoVector m_airspaces_all;
};

#endif
