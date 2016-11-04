#ifndef TBALIBAVADATA_H_
#define TBALIBAVADATA_H_

/**
 *  Data is received from the USB port with this format
 * 
 */
struct EventBlock
{
        unsigned int time;
        unsigned short temp;
        unsigned short data[256];
};



/**
 * We add value as an estra parameter when moving the
 * data within the application. It tells the value of
 * the variables which are scanned
 */
struct EventData : public EventBlock
{
        double value;
};

struct EventDataBlock : public EventData
{
        unsigned short header[32];
};

#endif /*TBALIBAVADATA_H_*/
