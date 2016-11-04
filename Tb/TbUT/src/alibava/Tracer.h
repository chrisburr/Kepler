#ifndef TRACER_H_
#define TRACER_H_
#include <deque>

class TH1;

class Tracer
{
    private:
        unsigned int size;
        int average;
        double cntr;
        double val;
        std::deque<double> queue;
        TH1 *hst;

    public:
        /**
         * Defines a tracer. At input npts defines the size of
         * the buffer (a FIFO actually) with the values that the
         * Tracer will remember. Also, if average is given, the 
         * Tracer will add as new points the average over 'average'
         * inputs
         */
        Tracer(const char*nam, const char *tit, int npts, int average=0);
        virtual ~Tracer();

        void Draw(const char *opt="");

        TH1 *get_hst()
        {
            return hst;
        }

        void fill(double val);
        void add_point(double x);
};

#endif /*TRACER_H_*/
