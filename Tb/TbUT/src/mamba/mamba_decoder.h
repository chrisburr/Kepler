#ifndef MAMBA_DECODER
#define MAMBA_DECODER

#include <stdint.h>
#include <vector>
#include <fstream>

class mamba_decoder
{

  private:
	bool& m_isAType;

    int nBeetles;
    int nSubsets;
    int nCH;
    int startADC;
    int stopADC;
    int nADCs;
      
    uint64_t _pack_id;
    uint64_t _trig_id;
    uint64_t _timestamp;
    uint64_t _ts_timestamp;
    uint64_t _coincidence_id;
    uint64_t _padding;
    std::vector<int>  _ADC;
    std::vector<int>  _bHeader0;
    std::vector<int>  _bHeader1;
    std::vector<int>  _bHeader2;
    std::vector<int>  _bHeader3;
    std::vector<int>  _bHeader3P1;
    std::vector<int>  _bHeader3P2;
    unsigned int _TDC;
    double _baseline;
    
  public:

    mamba_decoder(	bool& isAType);

    virtual ~mamba_decoder();
    
    bool eof(){ return ifile->eof(); }

    bool open(const char * filename);
    void close();
    void rewind(){ ifile->clear(); ifile->seekg(0, std::ios::beg); }

    std::ifstream * ifile;  
    bool find_mamba_header();
    bool check_mamba_footer();  
    
    int find_beetle_header( int * subsADCs, int thr) ;

    bool read_event();
 
    uint64_t PackID(){ return _pack_id; }
    uint64_t TrigID(){ return _trig_id; }
    uint64_t Timestamp(){ return _timestamp; }
    uint64_t TsTimestamp(){ return _ts_timestamp; }
    uint64_t CoincidenceID(){ return _coincidence_id; }
    uint64_t Padding(){ return _padding; }
    std::vector<int> ADC(){
    	return _ADC;
    }
    std::vector<int> BHeader0() {
      return _bHeader0; 
    }
    std::vector<int> BHeader1() {
      return _bHeader1; 
    }
    std::vector<int> BHeader2() {
      return _bHeader2; 
    }
    std::vector<int> BHeader3() {
      return _bHeader3; 
    }
    std::vector<int> BHeader3P1() {
      return _bHeader3P1; 
    }
    std::vector<int> BHeader3P2() {
      return _bHeader3P2; 
    }
    unsigned int TDC(){ return _TDC; }

};


#endif
