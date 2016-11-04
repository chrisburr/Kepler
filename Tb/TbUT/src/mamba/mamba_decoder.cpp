#pragma GCC diagnostic ignored "-Wvla"
#pragma GCC diagnostic ignored "-Wparentheses"

#include <iostream>
#include <fstream>

#include <assert.h>
#include <stdint.h>
#include <cmath>
#include <vector>

#include "mamba_decoder.h"

using namespace std;


mamba_decoder::mamba_decoder( bool& isAType):
		m_isAType(isAType)
{
  nBeetles=4;                               //Don't change
  nSubsets=4;                               //Don't change
  nCH=32;                                   //Don't change
  startADC = 6;                             //This value must be changed according to the settings of the MAMBA GUI  -> startADC <=8 in order not to cut the beetle header

  stopADC  = 30;                            //This value must be changed according to the settings of the MAMBA GUI  -> minium value for stopADC is defined according to the definition of nADCs (below)  
  nADCs = (stopADC-startADC - 1 )*2;
  assert( nADCs >= nCH + 6 );
    
}

mamba_decoder::~mamba_decoder(){

  if(ifile)
    ifile->close();
}

bool mamba_decoder::open( const char* filename ){
  cout<<"new decoder"<<endl;
  cout<<"with A and D sensor"<<endl;

  ifile = new std::ifstream( filename, std::ios::in | std::ios::binary );
 
  if (!(*ifile))
  {
    std::cout<<"Can not open data file:\t" << filename << std::endl;
    delete ifile;
    ifile = 0;
    return false;
  }
  return true;
}


void mamba_decoder::close()
{
  if (ifile)
  {
    ifile->close();
    delete ifile;
    ifile = 0;
  }
}

bool mamba_decoder::find_mamba_header(){
  bool found = 0;
  uint32_t temp_header[4];
  
  while(!found){
    ifile->read((char*)temp_header, 4*sizeof(uint32_t));
    if(ifile->eof()) return 0;
    
    if( temp_header[0] == 0xF1CA600D && temp_header[1] == 0x00000000 && temp_header[2] == 0xFFFFFFFF && temp_header[3] == 0xABBAFFFF ){
      found = 1 ;
    }
    else{
      ifile->seekg(-( 4 * sizeof(uint32_t) ) + 1, std::ios::cur);  //torno indietro e avanzo di un byte
    }
  }  
  return found;
}

bool mamba_decoder::check_mamba_footer(){
  bool checked = 0;
  uint32_t temp_footer[4];

  ifile->seekg( (12 + nBeetles*nSubsets*nADCs/2) * sizeof(uint32_t) , std::ios::cur );  //skip the coincidence,pack_id, trig_id ...infos and ADC values
  ifile->read( (char*)temp_footer, 4 * sizeof(uint32_t) );
  if(ifile->eof()) return 0;
  ifile->seekg( - (12 + (nBeetles*nSubsets*nADCs)/2 + 4) * sizeof(uint32_t) , std::ios::cur);

  if( temp_footer[0] == 0xFFFFFFFF && temp_footer[1] == 0xFFFFFFFF && temp_footer[2] == 0xFFFFFFFF && temp_footer[3] == 0xFFFFFFFF){
    checked = 1;
  }

  return checked;  
}

int mamba_decoder::find_beetle_header( int * subsADCs, int thr ){

  int pos=-1; 
  for( int iADC = 0; iADC < nADCs-4 ; iADC++ ){  
    if(        
        abs(subsADCs[iADC+1]-subsADCs[0]) > thr  &&
        abs(subsADCs[iADC+2]-subsADCs[0]) > thr  && 
        abs(subsADCs[iADC+3]-subsADCs[0]) > thr  && 
        abs(subsADCs[iADC+4]-subsADCs[0]) > thr  ){          
      pos = iADC+1;                             //returns the position of the last baseline ADC;
      break;     
    }
  }
  return pos;
}

bool mamba_decoder::read_event(){

  if(! find_mamba_header()  ) return 0;  //eof()
  if(! check_mamba_footer() ) return 0;
  
  uint64_t temp1, temp2;

  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _pack_id = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);

  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _trig_id = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);

  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _timestamp = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);

  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _ts_timestamp = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);
    
  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _coincidence_id = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);
  
  ifile->read((char*) &temp1 ,       sizeof(uint32_t));
  ifile->read((char*) &temp2 ,       sizeof(uint32_t));
  _padding = (temp2 & 0xFFFFFFFF) + ((temp1 & 0xFFFFFFFF) << 32);
  
  const int nChannels=nBeetles*nSubsets*nADCs;
  
  uint16_t temp_adc[nChannels];
  ifile->read((char*) temp_adc,        sizeof(uint16_t)* nBeetles*nSubsets*nADCs );

  int temp_adc2[nBeetles][nSubsets][nADCs];
  _ADC.clear();
  _bHeader0.clear(); 
  _bHeader1.clear(); 
  _bHeader2.clear(); 
  _bHeader3.clear(); 
  _bHeader3P1.clear(); 
  _bHeader3P2.clear(); 


  for(int iBeetle = 0; iBeetle < nBeetles; iBeetle++){
    for(int iSubset = 0; iSubset < nSubsets; iSubset++){  
      int tempsubset = (iSubset/2)*2 + ((iSubset%2)+1)%2 ;
      for (int iADC=0; iADC < nADCs; iADC++){  // "A   only" mode  
        temp_adc2[iBeetle][iSubset][iADC] = ( ( (temp_adc[iADC*nBeetles*nSubsets+nSubsets*iBeetle+tempsubset] ) & 0xFFFF )-2048 ) ;
      }
      
      if(m_isAType && (iBeetle==0 || iBeetle==2) ){
        _bHeader0.push_back(0); 
        _bHeader1.push_back(0); 
        _bHeader2.push_back(0); 
        _bHeader3.push_back(0); 
        _bHeader3P1.push_back(0); 
        _bHeader3P2.push_back(0); 
        for(int iADC=0; iADC<nCH; iADC++){
          _ADC.push_back( 0 );
        }
      }     
      else {
        int thr = 100;
        int pos_beetle_header = find_beetle_header( temp_adc2[iBeetle][iSubset], thr );
        //cout<<pos_beetle_header<<endl;
        //if( pos_beetle_header == -1 || pos_beetle_header >= (nADCs - nCH) ) return 0;  //can't find the header in the expected place
        _baseline = 0;
        for( int iADC=0; iADC < pos_beetle_header; iADC++){
          _baseline += temp_adc2[iBeetle][iSubset][iADC];
        }      
        _baseline = _baseline/pos_beetle_header;
        
        for(int iCH  = pos_beetle_header+4  ; iCH  < nCH + pos_beetle_header+4  ; iCH++ ){
          _ADC.push_back( temp_adc2[iBeetle][iSubset][iCH] );
          //if(iCH==31) cout << "channel: " << iCH << ", ADC: " << temp_adc2[iBeetle][iSubset][iCH] << endl;  
          //if(iCH==pos_beetle_header+10) cout << "beetle: " << iBeetle << ", subset: " << iSubset << ", channel: " << iCH << ", ADC: " << temp_adc2[iBeetle][iSubset][iCH] << endl;  
        }
        _bHeader0.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header] ); 
        _bHeader1.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header+1] ); 
        _bHeader2.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header+2] ); 
        _bHeader3.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header+3] ); 
        _bHeader3P1.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header+4] ); 
        _bHeader3P2.push_back( temp_adc2[iBeetle][iSubset][pos_beetle_header+5] ); 
        //cout << "beetle: "<< iBeetle<<", subset: " <<iSubset<<"beetlepos: "<<pos_beetle_header+3<<", ADC: " << temp_adc2[iBeetle][iSubset][pos_beetle_header+3] << endl;    
        
      }
    }
  }
  
  //cout << endl; 
    
  /*_ADC.clear();
  for(int iBeetle = 0; iBeetle < nBeetles; iBeetle++){
    for(int iSubset = 0; iSubset < nSubsets; iSubset++){
      for(int iADC    = 0  ; iADC  < nADCs    ; iADC++   ){
        _ADC.push_back( temp_adc2[iBeetle][iSubset][iADC] );
      }
    }
  }*/

  
  uint32_t temp_footer[4];
  ifile->read((char*) temp_footer,     sizeof(uint32_t)* 4   );
  
  uint32_t temp_TDC[4];
  ifile->read((char*) temp_TDC,        sizeof(uint32_t)* 4   );
  
  _TDC = (  ((temp_TDC[3]) & 0xFF) - (temp_TDC[3]>>8) & 0xFF ) % 0xFF;
  
  if(ifile->eof()){
    return 0;
  }
  
  
  return 1;  
}
