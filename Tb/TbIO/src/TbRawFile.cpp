// ROOT
#include "TFile.h"

// Local
#include "TbRawFile.h"

//=============================================================================
// Standard constructor
//=============================================================================
TbRawFile::TbRawFile(const std::string& filename, 
                     TbHeaderDecoder* headerDecoder) 
    : TbBufferedFile(filename, 0) {

  if (!is_open()) {
    m_good = false;
    return;
  }
  const size_t pos = m_filename.find(".dat");
  const size_t dash = m_filename.find_last_of("-");

  if (pos == std::string::npos) {
    m_good = false;
    return;
  }

  if (!(pos - dash > 1)) {
    m_good = false;
    return;
  }
  m_split_index = atoi(m_filename.substr(dash + 1, pos - dash - 1).c_str());
  if (m_split_index == 1) initialise();
  m_good = true;

  char header[16];
  m_file->ReadBuffer(header, 0, 16);
  unsigned int format = 0;
  headerDecoder->readCommon(header, m_headerSize, format);

  char* devHeader = new char[m_headerSize];
  m_file->ReadBuffer(devHeader, 0, m_headerSize);

  if (!headerDecoder->read(devHeader, format, m_id)) {
    m_good = false;
    return;
  }
  m_file->Seek(m_headerSize);
  m_size = m_file->GetSize();
  m_nPackets = (m_size - m_headerSize) / 8;
  m_bytesRead = m_headerSize;
  m_good = true;
}
