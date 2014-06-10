#include "dimm.h"
#include "dd/rr_dist.hpp"

/// Main program
int main( int nargs, char *args[] ) {
  int rank;
  MPI_Init( &nargs, &args );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm comm = MPI_COMM_WORLD;
  OF::DIMM<double, uint32_t, roundRobin> dimm( "WingStore.hub", comm );  

  MPI_Finalize();
  return 0; 
}


