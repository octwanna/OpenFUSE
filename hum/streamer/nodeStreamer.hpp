/*! \brief nodeStreamer.hpp
 **
 */
#ifndef NODE_STREAMER_HPP

#define NODE_STREAMER_HPP

#include "ihstream.h"
#include<vector>

template<typename floatT>
class nodeStreamer {
public:
  nodeStreamer(ihstream &in);

  nodeStreamer(ihstream &in, size_t num_nodes);

  const bool &isEof() const;

  const floatT &GetX() const;

  const floatT &GetY() const;

  const floatT &GetZ() const;

  const floatT *GetXYZ() const;

  floatT *XYZData();

  const floatT &GetElapsed() const;

  void Increment();

  void Rewind();

  void SetWriteBufOn();

  void SetWriteBufOff();

protected:

private:
  ihstream &_hum_in;
  floatT _count;
  bool _eof, _write_buf;
  std::vector< node<floatT> > _node_buf;
  floatT _buf_size, _elapsed;

  void FillUpBuffer();
  void DumpBuffer();

};

template<typename floatT>
nodeStreamer<floatT>
::nodeStreamer(ihstream &in)
: _hum_in(in), _buf_size(in.nNode()),
_node_buf(in.nNode()), _count(0),
_elapsed(0), _eof(false), _write_buf(false) {
  FillUpBuffer();
}

template<typename floatT>
nodeStreamer<floatT>
::nodeStreamer(ihstream &in, size_t num_nodes)
: _hum_in(in), _buf_size(num_nodes),
_count(0), _elapsed(0), _eof(false),
_write_buf(false) {
  /// If the user specified a buffer
  /// size more than we need shrink it
  if (num_nodes > _hum_in.nNode())
    _buf_size = _hum_in.nNode();
  _node_buf.resize(_buf_size),
    /// Read info into buffer
    FillUpBuffer();
}

template<typename floatT>
const bool &nodeStreamer<floatT>
::isEof() const {
  return _eof;
}

template<typename floatT>
const floatT &nodeStreamer<floatT>
::GetX() const {
  return _node_buf[_count].xyz[0];
}

template<typename floatT>
const floatT &nodeStreamer<floatT>
::GetY() const {
  return _node_buf[_count].xyz[1];
}

template<typename floatT>
const floatT &nodeStreamer<floatT>
::GetZ() const {
  return _node_buf[_count].xyz[2];
}

template<typename floatT>
const floatT *nodeStreamer<floatT>
::GetXYZ() const {
  return _node_buf[_count].xyz;
}

template<typename floatT>
floatT *nodeStreamer<floatT>
::XYZData() {
  return _node_buf[_count].xyz;
}

template<typename floatT>
const floatT &nodeStreamer<floatT>
::GetElapsed() const {
  return _elapsed;
}

template<typename floatT>
void nodeStreamer<floatT>
::SetWriteBufOn() {
  unsigned intent;
  herr_t err = H5Fget_intent(_hum_in._file, &intent);
  assert(intent == H5F_ACC_RDWR);
  _write_buf = true;
}

template<typename floatT>
void nodeStreamer<floatT>
::SetWriteBufOff() {
  _write_buf = false;
}

template<typename floatT>
void nodeStreamer<floatT>
::Increment() {
  _count++;
  _elapsed++;
  if (_elapsed == _hum_in.nNode()) /// Reached end-of-face
  {
    _eof = true;
    if (_write_buf == true) DumpBuffer();
    return;
  }
  if (_count == _buf_size) /// Reached end-of-buffer
  {
    if (_write_buf == true) DumpBuffer();
    FillUpBuffer();
    _count = 0;
  }
}

template<typename floatT>
void nodeStreamer<floatT>
::FillUpBuffer() {
  hsize_t size = _hum_in.nNode() - _elapsed;
  if (size > _buf_size) size = _buf_size;
  _hum_in.Read< node<floatT>, H5TNode<floatT> >
    (
    &_node_buf[0], _elapsed, 1, size
    );
}

template<typename floatT>
void nodeStreamer<floatT>
::DumpBuffer() {
  _hum_in.Write
    (
    &_node_buf[0], _elapsed - _count, 1, _count
    );
}

#endif

