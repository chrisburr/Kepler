#ifndef __TBALIBAVAHIT_H__
#define __TBALIBAVAHIT_H__

/**
 * A class representing a hit
 */

#include <vector>

class TbAlibavaHit
{
    private:
        int _center;
        int _left;
        int _right;
        double _sig;
        
        void cpy(const TbAlibavaHit &h);
    public:
        TbAlibavaHit(int c=0, int l=0, int r=0, double s=0);
        TbAlibavaHit(const TbAlibavaHit &h);
        ~TbAlibavaHit();
        
        TbAlibavaHit &operator=(const TbAlibavaHit &h);
        
        int center() const { return _center; }
        int left() const { return _left; }
        int right() const { return _right; }
        double signal() const { return _sig; }
        int width() const { return _right - _left + 1; }
};
typedef std::vector< TbAlibavaHit> TbAlibavaHitList;


#endif /*__TBALIBAVAHIT_H__*/
