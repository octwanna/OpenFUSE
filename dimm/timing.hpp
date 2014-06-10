/** \file timing.hpp
**
**/

#ifndef MPI_TIMING_H

#define MPI_TIMING_H

#include <sys/time.h>

namespace OF {

  class MPITiming {

    public:
    /// Default constructor
    MPITiming( MPI_Comm &comm );
    /// Start the timer
    void Start();
    /// Stop the timer
    double Stop();

    private:
    MPI_Comm &_comm;
    struct timeval _begin, _end;

  };

  /// Class implementation
  MPITiming
  ::MPITiming( MPI_Comm &comm )
  : _comm( comm )
  {
    gettimeofday(&_begin, NULL);
  }

  void MPITiming
  ::Start()
  {
    gettimeofday(&_begin, NULL);
  }

  double MPITiming
  ::Stop()
  {
    gettimeofday(&_end, NULL);
    double max_elapsed,
           elapsed = double( _end.tv_sec - _begin.tv_sec ) * 1.0e3
                     + double( _end.tv_usec - _begin.tv_usec ) / 1.0e3;
    MPI_Reduce( &elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, _comm );
    return max_elapsed;
  }

}

#endif

