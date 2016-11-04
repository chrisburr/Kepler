/* -*- mode: c++ -*- */
#ifndef __ChanList_h__
#define __ChanList_h__

#include <iostream>
#include <vector>
#include "TbAlibavaHit.h"
//////////////////////////////////////////////////////////////////////////
// ChanList
//
//   Created: Mon Sep  7 18:46:56 1998        Author: Carlos Lacasta
//   Purpose: Class to handle channel lists
//
//////////////////////////////////////////////////////////////////////////

class ChanList
{
    protected:
        int nch;                 // number of channels in the list
        std::vector<int> ch;     // list of channels
        TbAlibavaHitList hits;
        double cm;               // Common mode
        double noise;            // noise
        void copy(const ChanList &);
    public:
        ChanList(const char *ch = 0);
        ChanList(int, int);
        ChanList(const ChanList &);
        ChanList &operator=(const ChanList &);
        virtual ~ChanList()
        {
            hits.clear();
            ch.clear();
        }
        int Set(const char *);
        int Nch() const
        {
            return nch;
        }
        int Chan(int x) const
        {
            return ch[x];
        }

        int operator[](int x) const { return ch[x]; }
        void add_hit(const TbAlibavaHit &h) { hits.push_back(h); }
        bool empty() const { return hits.empty(); }
        int nhits() const { return hits.size(); }
        void clear_hits() { hits.clear(); }
        const TbAlibavaHit &get_hit(int i) const { return hits[i]; }
        const TbAlibavaHitList hit_list() const { return hits; }

        double CommonMode() const
        {
            return cm;
        }
        double Noise() const
        {
            return noise;
        }
        ChanList *CommonMode(double x)
        {
            cm = x;
            return this;
        }
        ChanList *Noise(double x)
        {
            noise = x;
            return this;
        }
        static int ParseChanList(const char *, ChanList **);
};
std::ostream &operator<<(std::ostream &, ChanList&);
#endif

