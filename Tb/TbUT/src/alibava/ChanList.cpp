#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include "ChanList.h"

static char svStr[8192];

//////////////////////////////////////////////////////////////////////////
// ChanList class
//
//   Created: Mon Sep  7 18:50:38 1998        Author: Carlos Lacasta
//   Purpose:
//
//////////////////////////////////////////////////////////////////////////
int cmp(const void *x1, const void *x2)
{
    double d1 = *(double *) x1 - *(double *) x2;
    return (d1 == 0. ? 0 : (d1 < 0. ? -1 : 1));
}
int icmp(const void *x1, const void *x2)
{
    int d1 = *(int *) x1 - *(int *) x2;
    return (d1 == 0 ? 0 : (d1 < 0 ? -1 : 1));
}

ChanList::ChanList(int i1, int /*i2*/) :
        nch(0), cm(0.), noise(0.)
{
    for (int i = 0; i < nch; i++)
        ch.push_back(i1 + i);

    nch = ch.size();
}
ChanList::ChanList(const char *str) :
        cm(0.), noise(0.)
{
    if (str == 0)
        return;
    Set(str);
}

ChanList::ChanList(const ChanList &cl)
{
    copy(cl);
}

ChanList &ChanList::operator=(const ChanList &cl)
{
    if (&cl == this)
        return *this;
    copy(cl);
    return *this;
}

void ChanList::copy(const ChanList &cl)
{
    if ( &cl != this )
    {
        hits = cl.hits;
        ch = cl.ch;
        nch = ch.size();
        cm = cl.cm;
        noise = cl.noise;
    }
}
int ChanList::Set(const char *str)
{
    char *p, *q;
    int i, is;
    int ival1, ival2, wRange;

    ch.clear();
    strcpy(svStr, str);
    p = q = svStr;
    wRange = 0;
    while (*p)
    {
        switch (*p)
        {
            case ',':
                *p = 0;
                if (wRange)
                {
                    is = sscanf(q, "%d", &ival2);
                    for (i = ival1 + 1; i <= ival2; i++)
                        ch.push_back(i);
                }
                else
                {
                    is = sscanf(q, "%d", &ival1);
                    ch.push_back(ival1);
                }
                wRange = 0;
                q = p + 1;
                p += is;
                break;
            case '-':
                *p = 0;
                wRange = 1;
                is = sscanf(q, "%d", &ival1);
                ch.push_back(ival1);
                q = p + 1;
                p += is;
                break;
        }
        p++;
    }
    if (wRange)
    {
        is = sscanf(q, "%d", &ival2);
        for (i = ival1 + 1; i <= ival2; i++)
            ch.push_back(i);
    }
    else
    {
        is = sscanf(q, "%d", &ival2);
        ch.push_back(ival2);
    }
    // sort
    std::sort(ch.begin(), ch.end());
    nch = ch.size();
    return nch;
}

std::ostream &operator<<(std::ostream &os, ChanList &cl)
{
    for (int i = 0; i < cl.Nch(); i++)
    {
        if (i && !(i % 15))
            os << std::endl;
        os << std::setw(5) << cl[i];
    }
    return os;
}

int ChanList::ParseChanList(const char *str, ChanList **cl)
{
    int ireg, nreg = 0;
    std::vector<char *> clst;
    char *q, *p, *ptr;

    if (str == 0)
        return 0;
    if (cl == 0)
        return 0;
    p = ptr = strdup(str);
    do
    {
        clst.push_back(p);
        q = strchr(p, ';');
        if (q == 0)
            break;
        *q++ = 0;
        p = q;
    } while (*p);

    nreg = clst.size();
    *cl = new ChanList[nreg];
    for (ireg = 0; ireg < nreg; ireg++)
        (*cl)[ireg].Set(clst[ireg]);

    free(ptr);
    return nreg;
}
