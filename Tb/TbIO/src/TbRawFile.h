#pragma once

// Tb/TbKernel
#include "TbKernel/TbBufferedFile.h"

// Local
#include "TbHeaderDecoder.h"

/** @class TbRawFile TbRawFile.h
 *
 * Interface for raw files in SPIDR format via TbBufferedFile (ROOT I/O)
 *
 */

class TbRawFile : public TbBufferedFile<1000000, uint64_t> {
 public:
  /// Constructor
  TbRawFile(const std::string& filename, TbHeaderDecoder* headerDecoder);
  /// Destructor
  virtual ~TbRawFile() {}

  /// Return the file index
  unsigned int splitIndex() const { return m_split_index; }
  uint64_t size() const { return m_size; }
  /// Return the chip identifier
  std::string id() const { return m_id; }
  /// Return whether the file has been opened successfully
  bool good() const { return m_good; }
  /// Return the number of data packets in the file
  uint64_t nPackets() const { return m_nPackets; }

  /// Set the offset in size of packets
  virtual void setOffset(const uint64_t offset) {
    TbBufferedFile::setOffset(offset * 8 + m_headerSize);
  }

 protected:
  uint64_t m_size;
  /// Number of data packets in the file
  uint64_t m_nPackets;
  /// Index by which large files are divided
  unsigned int m_split_index;
  /// Header size in bytes
  unsigned int m_headerSize;
  /// Chip identifier
  std::string m_id;
  bool m_good;
};
