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

#ifndef OLC_DIJKSTRA_HPP
#define OLC_DIJKSTRA_HPP

#include "Util/Serial.hpp"
#include "AbstractContest.hpp"
#include "PathSolvers/NavDijkstra.hpp"
#include "Trace/Vector.hpp"

#include <assert.h>

/**
 * Abstract class for contest searches using dijkstra algorithm
 *
 * These algorithms are designed for online/realtime use, and as such
 * expect solve() to be called during the simulation as time advances.
 *
 *
 */
class ContestDijkstra:
  public AbstractContest,
  protected NavDijkstra
{
  /**
   * This attribute tracks Trace::GetAppendSerial().  It is updated
   * when appnew copy of the master Trace is obtained, and is used to
   * check if that copy should be replaced with a new one.
   */
  Serial append_serial;

  /**
   * This attribute tracks Trace::GetModifySerial().  It is updated
   * when a new copy of the master Trace is obtained, and is used to
   * check if that copy should be replaced with a new one.
   */
  Serial modify_serial;

  /**
   * Is this a contest that allows continuous analysis?
   */
  const bool continuous;

  /**
   * Do an incremental analysis, attempting to improve the result in
   * each iteration?  If set, then only the last point is considered
   * as finish point, and start points are selected according to this.
   */
  bool incremental;

  bool trace_dirty;

  /**
   * Did the last Dijkstra search finish (even if without a valid
   * solution)?  This means the Dijkstra object still contains valid
   * data, and may be resumed with new data.  This flag gets cleared
   * when Trace or Dijkstra get cleared.
   */
  bool finished;

  /**
   * Working trace for solver.  This contains pointers to trace_master
   * records, which get Invalidated when the trace gets thinned.  Be
   * careful!
   */
  TracePointerVector trace;

protected:
  /** Number of points in current trace set */
  unsigned n_points;

  /**
   * The index of the first finish candidate.  During incremental
   * scan, only the new trace points are considered.
   */
  unsigned first_finish_candidate;

  /** Weightings applied to each leg distance */
  unsigned m_weightings[MAX_STAGES];

  ContestTraceVector best_solution;

public:
  /**
   * Constructor
   *
   * @param _trace Trace object reference to use for solving
   * @param n_legs Maximum number of legs in Contest task
   * @param finish_alt_diff Maximum height loss from start to finish (m)
   */
  ContestDijkstra(const Trace &_trace, 
                  bool continuous,
                  const unsigned n_legs,
                  const unsigned finish_alt_diff = 1000);

  void SetIncremental(bool _incremental) {
    incremental = _incremental;
  }

protected:
  gcc_pure
  const TracePoint &GetPoint(unsigned i) const {
    assert(i < n_points);

    return *trace[i];
  }

  gcc_pure
  const TracePoint &GetPoint(const ScanTaskPoint sp) const {
    return GetPoint(sp.GetPointIndex());
  }

  void ClearTrace();

  /**
   * Copy points that were added to the end of the master Trace.
   *
   * @return true if new points were added
   */
  bool UpdateTraceTail();

  void AddEdges(ScanTaskPoint origin, unsigned first_point);

  /**
   * Restart the solver with the new points added by
   * UpdateTraceTail().
   *
   * @param first_point the first point that was added
   */
  void AddIncrementalEdges(unsigned first_point);

  /**
   * Retrieve weighting of specified leg
   * @param index Index of leg
   * @return Weighting of leg
   */
  gcc_pure
  unsigned GetStageWeight(const unsigned index) const {
    assert(num_stages <= MAX_STAGES);
    assert(index + 1 < num_stages);

    return m_weightings[index];
  }

  /** 
   * Distance function for edges
   * 
   * @param s1 Origin node
   * @param s2 Destination node
   * 
   * @return Distance (flat) from origin to destination
   */
  gcc_pure
  unsigned CalcEdgeDistance(const ScanTaskPoint s1, const ScanTaskPoint s2) const {
    return GetPoint(s1).flat_distance(GetPoint(s2));
  }

  void Link(const ScanTaskPoint node, const ScanTaskPoint parent,
            unsigned value) {
    NavDijkstra::Link(node, parent, DIJKSTRA_MINMAX_OFFSET - value);
  }

private:
  gcc_pure
  bool IsMasterUpdated() const;

protected:
  /** Update working trace from master --- never to be done during a solution! */
  virtual void UpdateTrace();

  /**
   * Perform actions required at start of new search
   */
  virtual void StartSearch();
  virtual void AddStartEdges();

public:
  /* public virtual methods from AbstractContest */
  virtual bool Solve(bool exhaustive);
  virtual bool Score(ContestResult &result);
  virtual void CopySolution(ContestTraceVector &vec) const;
  virtual void Reset();

protected:
  /* protected virtual methods from AbstractContest */
  virtual fixed CalcDistance() const;
  virtual fixed CalcScore() const;
  virtual fixed CalcTime() const;
  virtual bool SaveSolution();

protected:
  /* methods from NavDijkstra */
  virtual void AddEdges(ScanTaskPoint curNode);
};

#endif
