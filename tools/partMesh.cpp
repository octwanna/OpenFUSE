#include "cdimm.hpp"
#include "rr_dist.hpp"

/// Main program
int main( int nargs, char *args[] ) {
  int rank;
  MPI_Init( &nargs, &args );
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm comm = MPI_COMM_WORLD;
  {
    ofuse::dimm<double, uint32_t, roundRobin> dimm( "WingStore.hub", comm );  
  }
  {
    ofuse::cdimm<double, uint32_t, roundRobin> cdimm( "WingStore.hub", comm );
  }
  MPI_Finalize();
  return 0; 
}

