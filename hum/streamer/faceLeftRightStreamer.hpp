/*! \brief faceLeftRightStreamer.h
 **
 */
#ifndef FACE_LEFT_RIGHT_STREAMER_H

#define FACE_LEFT_RIGHT_STREAMER_H

#include "ihstream.hpp"
#include <vector>

template<typename uintT>
class faceLeftRightStreamer {
public:
  faceLeftRightStreamer(ihstream &in);

  faceLeftRightStreamer(ihstream &in, size_t num_faces);

  const bool &isEof() const;

  const uintT &GetLeftCell() const;

  const uintT &GetRightCell() const;

  uintT &FaceLeftData();

  uintT &FaceRightData();

  leftRight<uintT> &FaceLRData();

  const uintT &GetElapsed() const;

  void Increment();

  void Rewind();

  /// Boundary streamer

  const bool &isEofPatch() const;

  const bool &isEofPatchFace() const;

  const uintT &GetElapsedPatch() const;

  const uintT &GetElapsedPatchFace() const;

  const std::string &GetPatchName() const;

  const uintT &GetPatchType() const;

  uintT GetPatchOffset() const;

  const uintT &GetPatchCell() const;

  const uintT &GetPatchElapsed() const;

  leftRight<uintT> &PatchFaceLRData();

  uintT &PatchFaceData();

  void IncrementPatchFace();

  void IncrementPatch();

  void SetWriteBufOn();

  void SetWriteBufOff();

protected:

private:
  ihstream &_hum_in;
  uintT _count, _count_patch, _count_patch_face;
  bool _eof, _eof_patch, _eof_patch_face, _write_buf;
  std::vector< leftRight<uintT> > _lr_buf;
  std::vector< leftRight<uintT> > _patch_lr_buf;
  uintT _buf_size, _elapsed, _elapsed_patch_face;
  uintT _patch_buf_size;

  void FillUpBuffer();
  void FillUpPatchBuffer();
  void DumpBuffer();
  void DumpPatchBuffer();

};

template<typename uintT>
faceLeftRightStreamer<uintT>
::faceLeftRightStreamer(ihstream &in)
: _hum_in(in), _buf_size(in.nInternalFace()),
_lr_buf(in.nInternalFace()), _count(0),
_elapsed(0), _eof(false), _write_buf(false),
_patch_lr_buf(in._max_patch_face),
_count_patch(0), _count_patch_face(0),
_eof_patch(false), _eof_patch_face(false),
_elapsed_patch_face(0), _patch_buf_size(in._max_patch_face) {
  FillUpBuffer();
  FillUpPatchBuffer();
}

template<typename uintT>
faceLeftRightStreamer<uintT>
::faceLeftRightStreamer(ihstream &in, size_t num_faces)
: _hum_in(in), _count(0),
_count_patch(0), _count_patch_face(0),
_eof(false), _eof_patch(false),
_eof_patch_face(false), _write_buf(false),
_buf_size(num_faces), _elapsed(0),
_elapsed_patch_face(0), _patch_buf_size(num_faces) {
  /// Check internal face sizes
  if (num_faces > _hum_in.nInternalFace())
    _buf_size = _hum_in.nInternalFace();
  _lr_buf.resize(_buf_size);
  /// Check patch face sizes
  if (num_faces > _hum_in._max_patch_face)
    _patch_buf_size = _hum_in._max_patch_face;
  _patch_lr_buf.resize(_patch_buf_size);
  /// Read data into buffer
  FillUpBuffer();
  FillUpPatchBuffer();
}

template<typename uintT>
const bool &faceLeftRightStreamer<uintT>
::isEof() const {
  return _eof;
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetLeftCell() const {
  return _lr_buf[_count].left;
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetRightCell() const {
  return _lr_buf[_count].right;
}

template<typename uintT>
uintT &faceLeftRightStreamer<uintT>
::FaceLeftData() {
  return _lr_buf[_count].left;
}

template<typename uintT>
uintT &faceLeftRightStreamer<uintT>
::FaceRightData() {
  return _lr_buf[_count].right;
}

template<typename uintT>
leftRight<uintT> &faceLeftRightStreamer<uintT>
::FaceLRData() {
  assert(_count < _lr_buf.size());
  return _lr_buf[_count];
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetElapsed() const {
  return _elapsed;
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::SetWriteBufOn() {
  unsigned intent;
  herr_t err = H5Fget_intent(_hum_in._file, &intent);
  assert(err >= 0);
  assert(intent == H5F_ACC_RDWR);
  _write_buf = true;
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::SetWriteBufOff() {
  _write_buf = false;
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::Increment() {
  /// Increment counters
  _count++;
  _elapsed++;
  if (_elapsed == _hum_in.nInternalFace()) /// Reached end-of-face
  {
    if (_write_buf == true) DumpBuffer();
    _eof = true;
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
void faceLeftRightStreamer<uintT>
::FillUpBuffer() {
  hsize_t size = _hum_in.nInternalFace() - _elapsed;
  if (size > _buf_size) size = _buf_size;
  _hum_in.read< leftRight<uintT>, H5TLeftRight<uintT> >
    (
    &_lr_buf[0], _elapsed, 1, size
    );
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::DumpBuffer() {
  _hum_in.write
    (
    &_lr_buf[0], _elapsed - _count, 1, _count
    );
}

/// Boundary streamer

template<typename uintT>
const bool &faceLeftRightStreamer<uintT>
::isEofPatch() const {
  return _eof_patch;
}

template<typename uintT>
const bool &faceLeftRightStreamer<uintT>
::isEofPatchFace() const {
  return _eof_patch_face;
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetPatchCell() const {
  //  return _elapsed_patch_face;
  return _patch_lr_buf[ _count_patch_face ].left;
}

template<typename uintT>
uintT &faceLeftRightStreamer<uintT>
::PatchFaceData() {
  //  return _elapsed_patch_face;
  return _patch_lr_buf[ _count_patch_face ].left;
}

template<typename uintT>
leftRight<uintT> &faceLeftRightStreamer<uintT>
::PatchFaceLRData() {
  return _patch_lr_buf[ _count_patch_face ];
}

template<typename uintT>
const std::string &faceLeftRightStreamer<uintT>
::GetPatchName() const {
  return _hum_in._patch_name_by_num[ _count_patch ];
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetPatchType() const {
  return _hum_in._patch_info_by_num[ _count_patch ].bcType;
}

template<typename uintT>
uintT faceLeftRightStreamer<uintT>
::GetPatchOffset() const {
  return _hum_in._patch_info_by_num[ _count_patch ].startFace;
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetElapsedPatchFace() const {
  return _elapsed_patch_face;
}

template<typename uintT>
const uintT &faceLeftRightStreamer<uintT>
::GetElapsedPatch() const {
  return _count_patch;
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::IncrementPatchFace() {
  _count_patch_face++;
  _elapsed_patch_face++;
  if (_elapsed_patch_face == _hum_in._patch_info_by_num[_count_patch].faceCount) {
    if (_write_buf == true) DumpPatchBuffer();
    _eof_patch_face = true;
    return;
  }
  if (_count_patch_face == _patch_buf_size) {

    if (_write_buf == true) DumpPatchBuffer();
    FillUpPatchBuffer();
    _count_patch_face = 0;
  }
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::IncrementPatch() {
  _count_patch++;
  _count_patch_face = 0;
  _eof_patch_face = false;
  _elapsed_patch_face = 0;
  if (_count_patch == _hum_in.nPatch()) {
    _eof_patch = true;
    return;
  }
  FillUpPatchBuffer();
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::FillUpPatchBuffer() {
  /// Calculate data chunk size
  hsize_t size = _hum_in._patch_info_by_num[ _count_patch ].faceCount - _elapsed_patch_face;
  hsize_t offset = _hum_in._patch_info_by_num[ _count_patch ].startFace;
  if (size > _patch_buf_size) size = _patch_buf_size;
  /// Read the chunk from hum file
  _hum_in.read< leftRight<uintT>, H5TLeftRight<uintT> >
    (
    &_patch_lr_buf[0], _elapsed_patch_face + offset, 1, size
    );
  //  std::cerr << "Elapsed face = " << _elapsed_patch_face << "\n";
}

template<typename uintT>
void faceLeftRightStreamer<uintT>
::DumpPatchBuffer() {
  hsize_t offset = _hum_in._patch_info_by_num[ _count_patch ].startFace;
  /// Read the chunk from hum file
  _hum_in.write
    (
    &_patch_lr_buf[0],
    _elapsed_patch_face - _count_patch_face + offset,
    1, _count_patch_face
    );
}

#endif

