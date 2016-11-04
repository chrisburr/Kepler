#ifndef __Alibava_TbAsciiRoot_h__
#define __Alibava_TbAsciiRoot_h__

#include <vector>
#include "TbAlibavaData.h"
#include "TbAlibavaHit.h"
#include "ChanList.h"
#include <ctime>
#include<string>
#include <TH1.h>
#include <TH2.h>


/**
 * This is the class that reads the data files
 */

class TbAsciiRoot
{
    public:
        typedef std::vector<std::string> XtraValues;
        enum BlockType { NewFile=0, StartOfRun, DataBlock, CheckPoint, EndOfRun };
    private:
        static const int max_nchan=256;
        std::ifstream *ifile;
        unsigned int data_start;
        int _type;
        time_t _t0;
        int _npoints;
        int _from;
        int _to;
        int _step;
        int _nevts;
        int _nchan; // current number of channels
        XtraValues _xtra; // extra values from header
        double _seedcut;
        double _neighcut;
        unsigned short _header[2][16];
        double _ped[max_nchan];
        double _noise[max_nchan];
        double _signal[max_nchan];
        double _sn[max_nchan];
        double _cmmd[2];
        double _cnoise[2];
        double _gain[max_nchan];
        double _average_gain;
        bool   _mask[max_nchan];
        int     _version;
        int     _polarity;
        TbAlibavaHitList _hits;

        std::vector<ChanList> chan_list;
        EventDataBlock _data;

    protected:
        void reset_data();

    public:
        void set_data(int i, unsigned short x) { _data.data[i] = x; }

    public:
        TbAsciiRoot(const char *nam=0, const char *pedfile=0, const char *gainfile=0);
        virtual ~TbAsciiRoot();

        bool valid() const
        {
            return (ifile!=0);
        }

        void open(const char *name);
        void close();
        void rewind();
        int read_event(std::string & error_code);
        virtual void check_point(int, const char *) {};
        virtual void new_file(int, const char *) {}
        virtual void start_of_run(int, const char *) {}
        virtual void end_of_run(int, const char *) {}
        virtual void new_data_block(int, const char *) {};

        // The data format version
        int version() const { return _version; }


        int polarity() const { return _polarity; }
        void polarity(int x) { _polarity = ( x<0 ? -1 : 1); }
        /*
         * Sets the number of channels and the data in the case
         * of non "standard" values. If data==0, then only the number
         * of channels is changed
         */
        void set_data(int nchan, const unsigned short *data=0);
        int nchan() const { return _nchan; }
        int type() const
        {
            return _type;
        }
        char *date() const
        {
            return ctime(&_t0);
        }
        double ped(int i) const
        {
            return _ped[i]/_gain[i];
        }
        double noise(int i) const
        {
            return _noise[i]/_gain[i];
        }
        double signal(int i) const
        {
            return _signal[i]/_gain[i];
        }

        double sn(int i) const
        {
            return _sn[i];
        }

        double get_cmmd(int i) const
        {
            return _cmmd[i];
        }

        double get_cnoise(int i) const
        {
            return _cnoise[i];
        }

        unsigned short data(int i) const
        {
            return _data.data[i];
        }
        double value() const
        {
            return _data.value;
        }
        double time() const;
        double temp() const;
        int npts() const
        {
            return _npoints;
        }
        int from() const
        {
            return _from;
        }
        int to() const
        {
            return _to;
        }
        int step() const
        {
            return _step;
        }
        int nevts() const
        {
            return _step;
        }

        void add_hit(const TbAlibavaHit &h)
        {
            _hits.push_back(h);
        }
        TbAlibavaHitList::iterator begin()
        {
            return _hits.begin();
        }
        TbAlibavaHitList::iterator end()
        {
            return _hits.end();
        }
        int nhits() const
        {
            return _hits.size();
        }
        bool empty() const
        {
            return _hits.empty();
        }
        const TbAlibavaHit &hit(int i) const
        {
            return _hits[i];
        }
        void set_hit_list(const TbAlibavaHitList &L) { _hits = L; }
        void clear()
        {
            _hits.clear();
        }

        double get_gain(int i) const
        {
            return _gain[i];
        }
        double gain() const
        {
            return _average_gain;
        }

        double seed_cut() const
        {
            return _seedcut;
        }
        double neigh_cut() const
        {
            return _neighcut;
        }
        void set_cuts(double s, double n)
        {
            _seedcut = s;
            _neighcut = n;
        }
        unsigned short get_header(int ichip, int ibit) { return _header[ichip][ibit]; }

        TH1 *show_pedestals();
        TH1 *show_noise();
        TH2 *compute_pedestals(int mxevts=-1, bool do_cmmd=true);
        void compute_pedestals_fast(int mxevts = -1, double ped_weight=0.01, double noise_weight=0.001);


        void process_event(bool do_cmmd=true);
        void find_clusters(int ichip=-1);
        void find_clusters(ChanList &C);
        void save_pedestals(const char *fnam);
        void load_pedestals(const char *fnam);
        void load_gain(const char *fnam);
		void load_masking(const char *fnam);
        void spy_data(bool with_signal=false, int nevt=1);
        void common_mode();
        void common_mode(ChanList &C, bool correct=false);

        int  n_channel_list() const { return chan_list.size(); }
        void add_channel_list(const ChanList &C);
        void clear_channel_lists() { chan_list.clear(); }
        ChanList get_channel_list(int i) const { return chan_list[i]; }

        int nxtra() const { return _xtra.size(); }
        const std::string xtra(int i) const { return _xtra[i]; }
        void add_xtra(const std::string &x) { _xtra.push_back(x); }
        void add_xtra(const char *x) { _xtra.push_back(x); }
};
// Return true if file is an ASCII text file
bool is_text(const char *);

#endif
