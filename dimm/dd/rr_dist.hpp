/*! \file rr_dist.hpp
**  \brief A round robin distribution class
*/
#ifndef ROUND_ROBIN_H

#define ROUND_ROBIN_H

#include<mpi.h>

class roundRobin {
  public:
  /// Default constructor
  roundRobin();
  ///
  roundRobin( size_t size, MPI_Comm &comm );
  /// Query functions
  /// Checks if the id belongs to this distribution
  template<typename uintT>
  inline bool in_dist( const uintT &id );
  /// Get the proc id given the gid
  template<typename uintT>
  inline int pid( const uintT &id );

  /// Data access functions
  inline const size_t &start() const;
  inline const size_t &end() const;
  inline size_t start( size_t proc_id );
  inline size_t end( size_t proc_id );
  inline size_t start_gid( size_t id );
  inline size_t end_gid( size_t id );
  inline const size_t &size() const;
  inline size_t size( int proc_id ) const;
  inline const int &rank() const;
  inline const int &comm_size() const;

  private:
  size_t _threshold, _residue,
         _entity_per_proc,
         _global_size, _local_size;
  int _rank, _comm_size;
  size_t _start, _end;

};

roundRobin
::roundRobin()
: _global_size(0)
{
  /* empty */
}

roundRobin
::roundRobin( size_t size , MPI_Comm &comm )
: _global_size(size)
{
  /// Get communicator size and my rank in communicator
  MPI_Comm_size( comm, &_comm_size );
  MPI_Comm_rank( comm, &_rank );
  _entity_per_proc = _global_size / _comm_size;
  _residue = _global_size % _comm_size;
  _start = 0;
  _threshold = _residue * ( _entity_per_proc + 1 );
  for( int i = 0 ; i < _rank ; ++i ) {
    _start += _entity_per_proc;
    if( unsigned(i) < _residue ) _start++;
  }
  _end = _start + _entity_per_proc;
  if( unsigned(_rank) < _residue ) _end++;
  _local_size = _end - _start;
#if 0
  std::cerr << "start : " << start() << "\n";
  std::cerr << "end   : " << end() << "\n";
#endif
}

const size_t &roundRobin
::start() const
{
  return _start;
}

size_t roundRobin
::start( size_t proc_id )
{
  return ( proc_id < _residue ) ?
         proc_id * ( _entity_per_proc + 1 ) :
         _threshold + ( proc_id - _residue ) * _entity_per_proc ;
}

size_t roundRobin
::start_gid( size_t id )
{
  return ( id < _threshold ) ?
         ( id / ( _entity_per_proc + 1 ) ) * ( _entity_per_proc + 1 ) :
         ( ( ( id - _threshold ) / _entity_per_proc ) *  _entity_per_proc + _threshold );
}

const size_t &roundRobin
::end() const
{
  return _end;
}

size_t roundRobin
::end( size_t proc_id )
{
  return ( proc_id < _residue ) ?
         ( proc_id + 1 ) * ( _entity_per_proc + 1 ) :
         _threshold + ( proc_id - _residue + 1 ) * _entity_per_proc ;
}

size_t roundRobin
::end_gid( size_t id )
{
  return ( id < _threshold ) ?
         ( id / ( _entity_per_proc + 1 ) + 1 ) * ( _entity_per_proc + 1 ) :
         ( ( ( id - _threshold ) / _entity_per_proc + 1 ) *  _entity_per_proc + _threshold );
}

const size_t &roundRobin
::size() const
{
  return _local_size;
}

size_t roundRobin
::size( int proc_id ) const
{
  return ( proc_id < _residue ) ? ( _entity_per_proc + 1 ): _entity_per_proc;
}

const int &roundRobin
::rank() const
{
  return _rank;
}

const int &roundRobin
::comm_size() const
{
  return _comm_size;
}

template <typename uintT>
bool roundRobin
::in_dist( const uintT &id )
{
  return ( id >= _start &&
           id < _end );
}

template <typename uintT>
int roundRobin
::pid( const uintT &id )
{
  return ( id < _threshold ) ?
         ( id / ( _entity_per_proc + 1 ) ) :
         ( ( id - _threshold ) / _entity_per_proc + _residue );
}

#endif
