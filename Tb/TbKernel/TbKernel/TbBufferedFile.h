#include <iostream>

// ROOT
#include "TFile.h"

/* @class TbBufferedFile TbBufferedFile.h
 *
 * Generic buffered interface to ROOT IO for use of IO plugins
 *
 */

// BUFFERSIZE is the number of objects that can be buffered
template <unsigned int BUFFERSIZE, class TYPE>
class TbBufferedFile {

 public:
  /// Constructor
  TbBufferedFile(const std::string& filename, bool init = true)
      : m_file(TFile::Open((filename + "?filetype=raw").c_str())),
        m_bytesRead(0),
        m_filename(filename),
        m_buffer_size(BUFFERSIZE * sizeof(TYPE)),
        m_buffer_pos(BUFFERSIZE),
        m_init(false) {
    if (is_open()) {
      m_bytes = m_file->GetSize();
      if (init) initialise();
    } else {
      std::cout << "File opening failed - waiting 5s then reconnecting"
                << std::endl;
      sleep(5);
      m_file = TFile::Open((filename + "?filetype=raw").c_str());
      if (is_open()) {
        m_bytes = m_file->GetSize();
        if (init) initialise();
      } else {
        std::cerr << "Cannot open file: " << filename << " critical error!"
                  << std::endl;
      }
    }
  }
  /// Destructor
  ~TbBufferedFile() { close(); }

  // sets the offset in the file and loads the corresponding buffer (IN BYTES )
  void setOffset(const uint64_t pos) {
    if (pos < m_bytesRead && pos > m_bytesRead - m_buffer_size) {
      m_buffer_pos = (pos - (m_bytesRead - m_buffer_size)) / sizeof(TYPE);
      return;
    }
    if (pos > m_bytes) {
      std::cout << "Attempted to read outside of file, offsetting cancelled"
                << std::endl;
      return;
    }
    m_file->SetOffset(pos, TFile::ERelativeTo::kBeg);
    m_buffer_size = (m_bytesRead + BUFFERSIZE * sizeof(TYPE) > m_bytes)
                        ? m_bytes - m_bytesRead
                        : m_buffer_size;
    m_file->ReadBuffer((char*)m_buffer, m_buffer_size);
    m_buffer_pos = 0;
    m_bytesRead = pos + m_buffer_size;
  }

  TYPE getNext() {

    if (__builtin_expect(m_buffer_pos * sizeof(TYPE) == m_buffer_size, 0)) {
      m_buffer_size = (m_bytesRead + BUFFERSIZE * sizeof(TYPE) > m_bytes)
                          ? m_bytes - m_bytesRead
                          : m_buffer_size;
      if (m_file->ReadBuffer((char*)m_buffer, m_buffer_size)) {
        /// this is to give additional protection in case of network timeouts
        std::cout << "Reading the buffer of " << m_filename
                  << " failed. Waiting for 5s" << std::endl;
        sleep(5);
        m_file->SetOffset(-m_buffer_size, TFile::ERelativeTo::kCur);
        if (m_file->ReadBuffer((char*)m_buffer, m_buffer_size)) {
          std::cout << "Reading the buffer failed again, closing file"
                    << std::endl;
          m_bytesRead = m_bytes;
          m_buffer_pos = m_buffer_size / sizeof(TYPE);
          return TYPE();
        }
      }
      m_bytesRead = m_bytesRead + m_buffer_size;
      m_buffer_pos = 0;
    }
    return m_buffer[m_buffer_pos++];
  }
  TYPE getPrevious() {
    if (m_buffer_pos > 0) {
      m_buffer_pos -= 1;
      return m_buffer[m_buffer_pos];
    } 
    return TYPE();
  }
  bool eof() const {
    return (m_bytesRead == m_bytes) &&
           (m_buffer_pos * sizeof(TYPE) == m_buffer_size);
  }
  void close() {
    if (m_init) {
      delete[] m_buffer;
      m_init = false;
    }
    if (is_open()) m_file->Close();
  }

  void reset() {
    if (m_init) {
      delete[] m_buffer;
      m_init = false;
    }
    m_bytesRead = 0;
    m_buffer_size = BUFFERSIZE * sizeof(TYPE);
    m_buffer_pos = BUFFERSIZE;
  }
  bool is_open() const { return m_file != 0 && m_file->IsOpen(); }

  void initialise() {
    if (!m_init) {
      m_buffer = new TYPE[BUFFERSIZE];
      m_init = true;
    }
  }

 protected:
  TFile* m_file;
  uint64_t m_bytesRead;
  std::string m_filename;

 private:
  uint64_t m_buffer_size;
  uint64_t m_buffer_pos;
  uint64_t m_bytes;
  TYPE* m_buffer;
  bool m_failure;
  bool m_init;
};
