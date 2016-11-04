#include "TbAlibavaHit.h"

TbAlibavaHit::TbAlibavaHit(int c, int l, int r, double s) :
    _center(c), _left(l), _right(r), _sig(s)
{
}

TbAlibavaHit::TbAlibavaHit(const TbAlibavaHit &h)
{
    cpy(h);
}
TbAlibavaHit::~TbAlibavaHit()
{
}

void TbAlibavaHit::cpy(const TbAlibavaHit &h)
{
    _center = h._center;
    _left = h._left;
    _right = h._right;
    _sig = h._sig;
}
TbAlibavaHit &TbAlibavaHit::operator=(const TbAlibavaHit &h)
{
    if (&h!=this)
        cpy(h);

    return *this;
}
