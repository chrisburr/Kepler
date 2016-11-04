#include <TH1.h>

#include "Tracer.h"

Tracer::Tracer(const char *name, const char *title, int npts, int avrg)
    : size(npts), average(avrg), cntr(0.), val(0.)
{
    hst = new TH1D(name, title, size, 0, size);
   
}

Tracer::~Tracer()
{
    delete hst;
}

void Tracer::Draw(const char *opt)
{
    hst->Draw(opt);
}

void Tracer::fill(double x)
{
    if (average>1.)
    {
        val = (x + cntr*val)/(++cntr);
        if ( (int(cntr)%average)==0 )
        {
            add_point(val);
        }
    }
    else
        add_point(x);
}

void Tracer::add_point(double x)
{
    queue.push_back(x);
    if (queue.size()>size)
        queue.pop_front();
    
    std::deque<double>::iterator ip;
    int ibin = 1;
    for (ip=queue.begin();ip!=queue.end();++ip, ibin++)
    {
        hst->SetBinContent(ibin, *ip);
    }
}
