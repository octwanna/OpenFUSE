/*! \brief faceStreamer.hpp
 **
 */
#ifndef FACE_STREAMER_HPP

#define FACE_STREAMER_HPP

#include "ihstream.hpp"
#include <vector>

template<typename uintT>
class faceStreamer {
public:
  faceStreamer(ihstream &in);

  faceStreamer(ihstream &in, size_t num_faces);

  const bool &isEof() const;

  const uintT &GetNumFaceNodes() const;

  const uintT *GetFaceNodes() const;

  uintT *FaceNodesData();

  face<uintT> &FaceData();

  const uintT &GetElapsed() const;

  void Increment();

  void Rewind();

  void SetWriteBufOn();

  void SetWriteBufOff();

protected:

private:
  ihstream &_hum_in;
  uintT _count;
  bool _eof, _write_buf;
  std::vector< face<uintT> > _face_buf;
  uintT _buf_size, _elapsed;

  void FillUpBuffer();
  void DumpBuffer();

};

template<typename uintT>
faceStreamer<uintT>
::faceStreamer(ihstream &in)
: _hum_in(in),
_count(0),
_eof(false),
_write_buf(false),
_face_buf(in.nFace()),
_buf_size(in.nFace()),
_elapsed(0) {
  FillUpBuffer();
}

template<typename uintT>
faceStreamer<uintT>
::faceStreamer(ihstream &in, size_t num_faces)
: _hum_in(in),
_count(0),
_eof(false),
_write_buf(false),
_buf_size(num_faces),
_elapsed(0) {
  /// If the user specified a buffer
  /// size more than we need shrink it
  if (num_faces > _hum_in.nFace())
    _buf_size = _hum_in.nFace();
  _face_buf.resize(_buf_size),
    /// Read info into buffer
    FillUpBuffer();
}

template<typename uintT>
const bool &faceStreamer<uintT>
::isEof() const {
  return _eof;
}

template<typename uintT>
void faceStreamer<uintT>
::SetWriteBufOn() {
  unsigned intent;
  herr_t err = H5Fget_intent(_hum_in._file, &intent);
  assert(err >= 0);
  assert(intent == H5F_ACC_RDWR);
  _write_buf = true;
}

template<typename uintT>
void faceStreamer<uintT>
::SetWriteBufOff() {
  _write_buf = false;
}

template<typename uintT>
const uintT &faceStreamer<uintT>
::GetNumFaceNodes() const {
  return _face_buf[_count].bField;
}

template<typename uintT>
const uintT *faceStreamer<uintT>
::GetFaceNodes() const {
  return _face_buf[_count].entityID;
}

template<typename uintT>
uintT *faceStreamer<uintT>
::FaceNodesData() {
  return _face_buf[_count].entityID;
}

template<typename uintT>
face<uintT> &faceStreamer<uintT>
::FaceData() {
  return _face_buf[_count];
}

template<typename uintT>
const uintT &faceStreamer<uintT>
::GetElapsed() const {
  return _elapsed;
}

template<typename uintT>
void faceStreamer<uintT>
::Increment() {
  _count++;
  _elapsed++;
  if (_elapsed == _hum_in.nFace()) /// Reached end-of-face
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

template<typename uintT>
void faceStreamer<uintT>
::DumpBuffer() {
  _hum_in.write
    (
    &_face_buf[0], _elapsed - _count, 1, _count
    );
}

template<typename uintT>
void faceStreamer<uintT>
::FillUpBuffer() {
  hsize_t size = _hum_in.nFace() - _elapsed;
  if (size > _buf_size) size = _buf_size;
  _hum_in.read< face<uintT>, H5TFace<uintT> >
    (
    &_face_buf[0], _elapsed, 1, size
    );
}

#endif

