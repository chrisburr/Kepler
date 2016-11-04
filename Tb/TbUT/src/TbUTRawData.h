#pragma once
#include "GaudiKernel/DataObject.h"
#include <string>
#include <vector>
#include "TbUTDataLocations.h"
#include "TbUTSensor.h"

namespace TbUT
{
template<typename DATA_TYPE = int>
class RawData
{
public:
	typedef std::vector<DATA_TYPE> SignalVector;
	typedef DATA_TYPE DataType;

	static void setSensor(Sensor p_sensor)
	{
		s_sensor=p_sensor;
	}
	static int getMaxChannel()
	{
		return s_sensor.maxChannel();
	}

	static int getMinChannel()
	{
		return s_sensor.minChannel();
	}
	static int getnChannelNumber()
	{
		return s_sensor.channelNumber*s_sensor.sensorNumber();
	}

	static int getSensorNumber()
	{
		return s_sensor.sensorNumber();
	}

	RawData<DATA_TYPE>():
		m_signal(),
    		m_header0(), 
    		m_header1(), 
    		m_header2(), 
    		m_header3(), 
    		m_header3P1(), 
    		m_header3P2(),
		m_time(),
		m_temperature(),
		m_tdc()
	{
	};

	RawData<DATA_TYPE>& operator=( const RawData<DATA_TYPE>& rhs )
	{
		 if( this != &rhs ) {
			 m_signal=rhs.m_signal;
			 m_header0=rhs.m_header0;
        	 m_header1=rhs.m_header1;
	         m_header2=rhs.m_header2;
        	 m_header3=rhs.m_header3;
			 m_header3P1=rhs.m_header3P1;
			 m_header3P2=rhs.m_header3P2;
			 m_time=rhs.m_time;
			 m_temperature=rhs.m_temperature;
			 m_tdc=rhs.m_tdc;
		 }
	      return *this;
	  }
	RawData<DATA_TYPE>(const RawData<DATA_TYPE> &rhs):
		 m_signal(rhs.m_signal),
		 m_header0(rhs.m_header0),
         m_header1(rhs.m_header1),
		 m_header2(rhs.m_header2),
		 m_header3(rhs.m_header3),
		 m_header3P1(rhs.m_header3P1),
		 m_header3P2(rhs.m_header3P2),
		 m_time(rhs.m_time),
		 m_temperature(rhs.m_temperature),
		 m_tdc(rhs.m_tdc)
	{
	}
	
  	void setTemp(double p_temp)
	{
		m_temperature = p_temp;
	}
	void setTime(unsigned long long p_time)
	{
		m_time = p_time;
	}
	void setSignal(DATA_TYPE p_signal)
	{
		m_signal.push_back(p_signal);
	}
  void setHeader0(DATA_TYPE p_header) 
  {
    m_header0.push_back(p_header); 
  }
  void setHeader1(DATA_TYPE p_header) 
  {
    m_header1.push_back(p_header); 
  }
  void setHeader2(DATA_TYPE p_header) 
  {
    m_header2.push_back(p_header); 
  }
  void setHeader3(DATA_TYPE p_header) 
  {
    m_header3.push_back(p_header); 
  }
  void setHeader3P1(DATA_TYPE p_header) 
  {
    m_header3P1.push_back(p_header); 
  }
  void setHeader3P2(DATA_TYPE p_header) 
  {
    m_header3P2.push_back(p_header); 
  }
	SignalVector& getSignal()
	{
		return m_signal;
	}
	SignalVector& getHeader0()
	{
		return m_header0;
	}
	SignalVector& getHeader1()
	{
		return m_header1;
	}
	SignalVector& getHeader2()
	{
		return m_header2;
	}
	SignalVector& getHeader3()
	{
		return m_header3;
	}
	SignalVector& getHeader3P1()
	{
		return m_header3P1;
	}
	SignalVector& getHeader3P2()
	{
		return m_header3P2;
	}

	unsigned long long getTime() const
	{
		return m_time;
	}
	double getTemp() const
	{
		return m_temperature;
	}

	DATA_TYPE getSignal(int channel) const
	{
		return m_signal[channel];
	}
	
  DATA_TYPE getHeader0(int subset) const
	{
		return m_header0[subset];
	}
  DATA_TYPE getHeader1(int subset) const
	{
		return m_header1[subset];
	}
  DATA_TYPE getHeader2(int subset) const
	{
		return m_header2[subset];
	}
  DATA_TYPE getHeader3(int subset) const
	{
		return m_header3[subset];
	}
  DATA_TYPE getHeader3P1(int subset) const
	{
		return m_header3P1[subset];
	}
  DATA_TYPE getHeader3P2(int subset) const
	{
		return m_header3P2[subset];
	}

	unsigned int getTDC() const
	{
		return m_tdc;
	}

	void setTDC(unsigned int p_tdc)
	{
		m_tdc=p_tdc;
	}

protected:
	SignalVector m_signal;
	SignalVector m_header0;
	SignalVector m_header1;
	SignalVector m_header2;
	SignalVector m_header3;
	SignalVector m_header3P1;
	SignalVector m_header3P2;
	unsigned long long m_time;
	double m_temperature;
	unsigned int m_tdc;

	static Sensor s_sensor;
};


template<typename DataType  = int>
class RawDataContainer:  public DataObject
{
public:
	typedef std::vector<RawData<DataType> > RawDataVec;

	RawDataContainer<DataType>():
		empty(true),
		m_dataVector()
	{};

	void addData(RawData<DataType> rawData)
	{
		m_dataVector.push_back(rawData);
		empty=false;
	}
	bool isEmpty(){return empty;}
	RawDataVec getData(){return m_dataVector;}
private:
	bool empty;
	RawDataVec m_dataVector;

};


}

