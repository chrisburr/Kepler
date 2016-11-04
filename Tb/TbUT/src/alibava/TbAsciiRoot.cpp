#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <sstream>
#include <csignal>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <TROOT.h>
#include <TCanvas.h>
#include <TProfile2D.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include "TbAsciiRoot.h"
#include "utils.h"
#include "Tracer.h"
#include "TbAlibavaHit.h"
#include "ChanList.h"
/*

#ifdef __APPLE__
#define sighandler_t sig_t
#endif

bool _A_do_run = true;
void _A_got_intr(int)
{
    _A_do_run = false;
}
*/
// decodes the header and returns a vector with the integers found
std::vector<int> decode_header(const std::string &h, TbAsciiRoot::XtraValues &xtra)
{
    std::vector<int> vout;
    std::istringstream istr(h);
    char *endptr;
    char buf[256];
    long val;

    xtra.clear();

    while (istr)
    {
        istr.getline(buf, sizeof(buf), ';');
        if (!istr)
            break;

        errno = 0;
        val = strtol(buf, &endptr, 0);

        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
        {
            std::string sval(buf), sout;
            sout = trim_str(sval);
            if (!sout.empty())
                xtra.push_back( sout );
        }
        else if ( endptr == buf || *endptr != '\0' )
        {
            std::string sval(buf), sout;
            sout = trim_str(sval);
            if (!sout.empty())
                xtra.push_back( sout );
        }
        else
        {
            vout.push_back(atoi(buf) );
        }
    }
    return vout;
}

TbAsciiRoot::TbAsciiRoot(const char * /*nam*/, const char * /*pedfile*/, const char * /*gainfile*/) :
    _nchan(max_nchan),_seedcut(10.), _neighcut(5.), _average_gain(1.), _version(2), _polarity(1)
{
    int i;
    for (i=0;i<max_nchan;i++)
    {
        _ped[i] = 0.;
        _gain[i] = 1.;
        _noise[i] = 1.;
        _data.data[i] = 0;
        _mask[i] = false;
    }
    ifile=NULL;
}

TbAsciiRoot::~TbAsciiRoot()
{
    if (ifile)
    {
        ifile->close();
    }
}

void TbAsciiRoot::open(const char *name)
{
    ifile = new std::ifstream(name);
    if (!(*ifile))
    {
        std::cout << "Could not open data file: " << name << std::endl;
        delete ifile;
        ifile = 0;
		return;
    }
    std::string header;
    unsigned int ic, lheader;
    char c;
    ifile->read((char *)&_t0, sizeof(time_t));
    //std::cout << "64b: " << ctime(&_t0) << std::endl;
    ifile->read((char *)&_type, sizeof(int));
    //std::cout << "type_ " << _type << std::endl;
    if ( _type > 15 )
    {
        ifile->seekg(0, std::ios::beg);
        ifile->read((char *)&_t0, sizeof(int));
        ifile->read((char *)&_type, sizeof(int));
        //std::cout << "32b: " << ctime(&_t0) << std::endl;
    }


    ifile->read((char *)&lheader, sizeof(unsigned int));
    for (ic=0; ic<80; ic++)
    {
        ifile->read(&c, sizeof(char));
        header.append(1, c);
    }
    header = trim_str(header);

    if (header[0]!='V' && header[0]!='v')
    {
        _version = 0;
    }
    else
    {
        _version = int(header[1]-'0');
        header = header.substr(5);
    }

    std::cout << "type: " << _type << " header: " << header << std::endl;
    std::vector<int> param = decode_header(header, _xtra);
    ifile->read((char *)_ped, max_nchan*sizeof(double));
    ifile->read((char *)_noise, max_nchan*sizeof(double));
    switch (_type)
    {
        case 1: // calibration
        case 2: // laser sync
            _npoints = param[0];
            _from = param[1];
            _to = param[2];
            _step = param[3];
            break;
        case 3: // laser run
        case 4: // source run
        case 5: // pedestal run
            if (param.empty())
                _nevts = 100000;
            else
                _nevts = param[0];
            _npoints = _from = _to = _step = 0;
            break;
    }
    data_start = ifile->tellg();
}

void TbAsciiRoot::rewind()
{
    if (ifile)
    {
        ifile->clear();
        ifile->seekg(data_start, std::ios::beg);
    }
}

void TbAsciiRoot::close()
{
    if (ifile)
    {
        ifile->close();
        delete ifile;
        ifile = 0;
    }
}

void TbAsciiRoot::reset_data()
{
    memset(&_data, 0, sizeof(_data));
}

int TbAsciiRoot::read_event(std::string & error_code)
{
    if (ifile)
    {
        unsigned int header, size, user=0, code=0;
        char *block_data=0;
        if (_version)
        {
            do
            {
                do
                {
                    ifile->read((char *)&header, sizeof(unsigned int));
                    if (ifile->bad() || ifile->eof()){
		      error_code="read Header";
		      return -1;
		    }

                    code = (header>>16) & 0xFFFF;
                } while ( code != 0xcafe );

                code = header & 0x0fff;
                user = header & 0x1000;
                switch (code)
                {
                    case NewFile:
                        ifile->read((char *)&size, sizeof(unsigned int));
                        block_data = new char[size];
                        ifile->read(block_data, size);
                        new_file(size, block_data);
                        break;
                    case StartOfRun:
                        ifile->read((char *)&size, sizeof(unsigned int));
                        block_data = new char[size];
                        ifile->read(block_data, size);
                        start_of_run(size, block_data);
                        break;
                    case DataBlock:
                        ifile->read((char *)&size, sizeof(unsigned int));
                        if (user)
                        {
                            reset_data();
                            block_data = new char[size];
                            ifile->read(block_data, size);
                            new_data_block(size, block_data);
                        }
                        else
                        {
                            if ( _version == 1 )
                            {
                                ifile->read((char *)&_data, sizeof(EventData));
                                for (int ii=0; ii<2; ii++)
                                    memset(_header[ii], 0, 16*sizeof(unsigned short));

                            }
                            else
                            {
                                ifile->read((char *)&_data.value, sizeof(double));
                                ifile->read((char *)&_data.time, sizeof(unsigned int));
                                ifile->read((char *)&_data.temp, sizeof(unsigned short));
                                for (int ii=0; ii<2; ii++)
                                {
                                    ifile->read((char *)_header[ii], 16*sizeof(unsigned short));
                                    ifile->read((char *)&_data.data[ii*128], 128*sizeof(unsigned short));
                                }
                            }
                        }

                        break;
                    case CheckPoint:
                        ifile->read((char *)&size, sizeof(unsigned int));
                        block_data = new char[size];
                        ifile->read(block_data, size);
                        check_point(size, block_data);
                        break;
                    case EndOfRun:
                        ifile->read((char *)&size, sizeof(unsigned int));
                        block_data = new char[size];
                        ifile->read(block_data, size);
                        end_of_run(size, block_data);
                        break;
                    default:
                        std::cout << "Unknown block data type: " << std::hex << header << " - " << code << std::dec << std::endl;
                }
                if (block_data)
                {
                    delete [] block_data;
                    block_data = 0;
                }

            } while ( code != DataBlock && !(ifile->bad() || ifile->eof()) );
        }
        else
        {
            ifile->read((char *)&_data, sizeof(EventData));
            for (int ii=0; ii<2; ii++)
                memset(_header[ii], 0, 16*sizeof(unsigned short));
        }

        if (ifile->eof())
        {
            std::cout << "End of file" << std::endl;
	    error_code="End of file";
            return -1;
        }
        else if (ifile->bad())
        {
            std::cout << "Problems with data file" << std::endl;
	    error_code="Problems with data file";
            return -1;
        }
        else
	  {
	    error_code="No problem";
            //process_event();
            return 0;
        }
    }
    else{
      error_code="big file problem";
      return -1;
    }
}

void TbAsciiRoot::set_data(int nchan, const unsigned short int *data)
{
    int i;
    _nchan = nchan;
    for (i=0;i<_nchan;i++)
        _data.data[i] = data[i];
}


double TbAsciiRoot::time() const
{
    unsigned short fpart = _data.time & 0xffff;
    short ipart = (_data.time & 0xffff0000)>>16;
    if (ipart<0)
        fpart *= -1;
    //double tt = 100.*(1. -(ipart + (fpart/65535.)));
    double tt = 100.0*(ipart + (fpart/65535.));
    return tt;
}

double TbAsciiRoot::temp() const
{
    return 0.12*_data.temp - 39.8;
}


/*
TH1 *TbAsciiRoot::show_pedestals()
{
    int ic;
    TH1 *hst = create_h1("hPed","Pedestals",nchan(),-0.5, nchan()-0.5);
    hst->SetYTitle("ADCs");
    hst->SetXTitle("Channel no.");
    for (ic=0; ic<nchan(); ic++)
        hst->SetBinContent(ic+1, _ped[ic]);

    return hst;
}*/

/*
TH1 *TbAsciiRoot::show_noise()
{
    int ic;
    TH1 *hst = create_h1("hNoise","Noise",nchan(),-0.5, nchan()-0.5);
    if (gain()==1)
    {
        hst->SetYTitle("ADCs");
    }
    else
    {
        hst->SetYTitle("e^{-} ENC");
    }
    hst->SetXTitle("Channel no.");
    for (ic=0; ic<nchan(); ic++)
        hst->SetBinContent(ic+1, noise(ic));

    return hst;
}
*/

void TbAsciiRoot::compute_pedestals_fast(int mxevts, double wped, double wnoise)
{
    if (!ifile)
        return;

    if (mxevts<0)
        mxevts = 100000000;

    for (int i=0;i<max_nchan;i++)
        _ped[i] = _noise[i] = 0.;

    // std::ifstream::pos_type here = ifile->tellg();
    std::cout << "Computing fast pedestals..." << std::endl;
    std::string error="";
    for (int ievt=0; read_event(error)==0 && ievt<mxevts; ievt++)
    {
        if (!(ievt%100))
        {
            std::cout << "\revent " << std::setw(10) << ievt << std::flush;
        }
        common_mode();
        for (int i=0; i<nchan(); i++)
        {
            // TODO: figure out how to determine the chip number when
            //       Plugin::filter_event has been called
            int ichip = i/128;
            // IF noise is 0, set it arbitrarily to 1.
            if (_noise[i]==0.)
                _noise[i] = 1.;

            if (_ped[i]==0.)
            {
                // If pedestal is not yet computed we assume the current
                // channel value should not be too far
                _ped[i] = _data.data[i];
            }
            else
            {
                // Do the pedestal and noise correction
                double corr;
                double xs;

                _signal[i] = _data.data[i] - _ped[i];
                corr = _signal[i] * wped;

                xs = (_signal[i]-_cmmd[ichip])/_noise[i];
                if (corr > 1.)
                    corr = 1.;

                if (corr < -1)
                    corr = -1.;

                _ped[i] += corr;

                if (fabs(xs) < 3.)
                {
                    _noise[i] = _noise[i]*(1.0-wnoise) + xs*xs*wnoise;
                }
            }
        }
    }
    std::cout << "\nDone" << std::endl;
    rewind();
}

/*
TH2 *TbAsciiRoot::compute_pedestals(int mxevts, bool do_cmmd)
{
    if (!ifile)
        return 0;

    if (mxevts<0)
        mxevts = 100000000;

    int ievt, ichan;
    TH2 *hst = create_h2("hRaw","Raw data",nchan(), -0.5,nchan()-0.5, 256, -0.5,1023.5);
    TH2 *hsts = create_h2("hSig","Signal",nchan(), -0.5,nchan()-0.5,256, -127.5,127.5);


    std::ifstream::pos_type here = ifile->tellg();
    std::cout << "Computing pedestas..." << std::endl;
    for (ievt=0; read_event()==0 && ievt<mxevts; ievt++)
    {
        process_event(do_cmmd);
        for (ichan=0; ichan<nchan(); ichan++)
            // TODO: get right chip number in all situations (after calling set_data)
            hst->Fill(ichan, data(ichan)-get_cmmd(ichan/128));

        if (!(ievt%100))
        {
            std::cout << "\revent " << std::setw(10) << ievt << std::flush;
        }
    }
    std::cout << "\nDone" << std::endl;
    rewind();

    // TODO: _nchan can be updated in an event by event basis
    //       while here we are assuming that it is the same
    //       for all the events
    for (ichan=0; ichan<nchan(); ichan++)
    {
        TF1 *g = new TF1("g1", "gaus");
        TH1 *h1 = hst->ProjectionY("__hx__", ichan+1, ichan+1);
        g->SetParameters(h1->GetSumOfWeights(), h1->GetMean(), h1->GetRMS());
        g->SetRange(h1->GetMean()-2.5*h1->GetRMS(), h1->GetMean()+2.5*h1->GetRMS());
        h1->Fit("g1", "q0wr");
        _ped[ichan] = h1->GetFunction("g1")->GetParameter(1);
        _noise[ichan] = h1->GetFunction("g1")->GetParameter(2);
        delete h1;
        delete g;
    }

    rewind();
    for (ievt=0; read_event()==0 && ievt<mxevts; ievt++)
    {
        process_event(do_cmmd);
        for (ichan=0; ichan<nchan(); ichan++)
            hsts->Fill(ichan, signal(ichan));

        if (!(ievt%100))
        {
            std::cout << "\revent " << std::setw(10) << ievt << std::flush;
        }
    }
    std::cout << "\nDone" << std::endl;
    rewind();

    return hst;
}
*/



void TbAsciiRoot::find_clusters(int ichip)
{
    int chan0=0;
    int chan1=255;
    if (ichip>=0 && ichip<2)
        {
            chan0 = ichip*128;
            chan1 = (ichip+1)*128 -1;
        }

    std::ostringstream ostr;
    ostr << chan0 << '-' << chan1;
    ChanList C(ostr.str().c_str());

    clear();
    find_clusters(C);
    _hits = C.hit_list();
}

void TbAsciiRoot::find_clusters(ChanList &C)
{
    // TODO: figure out how to determine the chip number in
    //       all the situations
    int i, j, imax=-1, left, right;
    double mxsig=-1.e20, sg, val;
    std::vector<bool> used(C.Nch());

    for (i=0;i<C.Nch();i++)
    {
        used[i]= _mask[C[i]] ? true : false;
    }



    while (true)
    {
        /*
         * Find the highest
         */
        imax = -1;
        for (j=0; j<C.Nch(); j++)
        {
            i = C[j];
            if (used[j] || _signal[i]*polarity()<0.)
                continue;

            if ( polarity()*sn(i) > _seedcut)
            {
                val = fabs(signal(i));
                if (mxsig<val)
                {
                    mxsig = val;
                    imax = j;
                }
            }
        }

        if (imax<0 || imax >= C.Nch() )
            break;

        sg = signal(C[imax]);
        used[imax]=true;
        // Now look at neighbors
        // first to the left
        left = imax;
        for (j=imax-1;j>=0;j--)
        {
            i = C[j];
            if ( used[j] || _signal[i]*polarity()<0.)
                break;

            if ( fabs(sn(i)) > _neighcut )
            {
                used[j] = true;
                sg += signal(i);
                left = j;
            }
            else
                // TODO: this needs to be removed
                // The idea is to merge to clusters that have only one strip in between
                // In the laser runs this is a consequences of reflections...
            {
                int jx = j-1;
                if (jx>=0  )
                {
                    if ( fabs(sn(C[jx])) > _neighcut )
                        continue;
                }
                break;
            }
        }

        // now to the right
        right = imax;
        for (j=imax+1;j<C.Nch();j++)
        {
            i = C[j];
            if ( used[j] || _signal[i]*polarity()<0.)
                break;
            if ( fabs(sn(i))>_neighcut )
            {
                used[j] = true;
                sg += signal(i);
                right = j;
            }
            else
                // TODO: this needs to be removed
                // The idea is to merge to clusters that hanve only one strip in between
                // In the laser runs this is a consequences of reflections...
            {
                int jx = i+1;
                if (jx<C.Nch())
                {
                    if ( fabs(sn(C[jx])) > _neighcut )
                        continue;
                }
                break;
            }
        }
        C.add_hit(TbAlibavaHit(imax, left, right, sg));
    }
}

void TbAsciiRoot::save_pedestals(const char *fnam)
{
    std::ofstream ofile(fnam);
    if (!ofile)
    {
        std::cout << "Could not open " << fnam << " to save pedestals." << std::endl;
        return;
    }

    // TODO: _nchan can be updated in an event by event basis
    //       while here we are assuming that it is the same
    //       for all the events
    int i;
    for (i=0; i<nchan(); i++)
    {
        ofile << _ped[i] << "\t" << _noise[i] << "\n";
    }
    ofile.close();
}

void TbAsciiRoot::load_pedestals(const char *fnam)
{
    std::ifstream ifile(fnam);
    if (!ifile)
    {
        std::cout << "Could not open " << fnam << " to load pedestals." << std::endl;
        return;
    }
    int i;
    for (i=0; i<max_nchan; i++)
    {
        if (ifile.eof())
            break;

        ifile >> _ped[i] >> std::ws >> _noise[i] >> std::ws;
        _mask[i] = (_noise[i]>20. || _noise[i]<=0.);
    }
    ifile.close();
    /*
    TCanvas *pedcanvas = create_canvas("Pedcanvas", "Pedestal Values", 600, 400);
    TH1 *pedestalhisto = create_h1("pedestalhisto", "Pedestal Values", 256, -0.5, 255.5);
    for (i=0; i<256; i++)
    {
        pedestalhisto->Fill(i, _ped[i]);
    }
    pedcanvas->cd(1);
    pedestalhisto->Draw();
*/
}

void TbAsciiRoot::load_masking(const char *fnam)
{
    std::ifstream ifile(fnam);
    if (!ifile)
    {
        std::cout << "Could not open masked.txt. " << std::endl;
        return;
    }
    int val;
    for (int i=0; i<500; i++)
    {
        ifile >> val >> std::ws;
        if (ifile.eof())
            break;
        if (val>255)
        {
            std::cout << "A value is greater than 255, causing an overflow crash. Please check the text file again. It has been set to 1 for continuation purposes. " << std::endl;
            val = 1;
        }
        _mask[val] = true;
    }
}

void TbAsciiRoot::load_gain(const char *fnam)
{
    std::ifstream ifile(fnam);
    if (!ifile)
    {
        std::cout << "Could not open " << fnam << " to load the gain." << std::endl;
        return;
    }
    int i;
    int ichan;
    double val, xn, xm;
    xn=xm=0.;
    for (i=0; i<max_nchan; i++)
    {
        ifile >> ichan >> std::ws;
        if (ifile.eof())
            break;
        ifile >> val;
        if (ifile.eof())
            break;

        xn++;

        xm += val;
        _gain[ichan] = val;

        ifile >> std::ws;
        if (ifile.eof())
            break;
    }
    if (xn>0)
    {
        _average_gain = xm/xn;
    }
    ifile.close();
}

void TbAsciiRoot::process_event(bool do_cmmd)
{
    int i;
    for (i=0; i<nchan(); i++)
    {
        _signal[i] = _data.data[i]-_ped[i];
        _sn[i] = _noise[i]>1. && !_mask[i] ? _signal[i]/_noise[i] : 0.;
    }
    if (do_cmmd)
    {
        int ichip=-1;
        common_mode();

        for (i=0; i<nchan(); i++)
        {
            // TODO: figure out the right chip number
            if (!(i%128))
                ichip ++;

            _signal[i] = _data.data[i]-_ped[i] - _cmmd[ichip];
            _sn[i] = (_noise[i] >1. && !_mask[i] ? _signal[i]/_noise[i] : 0.);
        }
    }
}

void TbAsciiRoot::add_channel_list(const ChanList &C)
{
    chan_list.push_back(C);
}


void TbAsciiRoot::common_mode()
{
    ChanList C("0-127");
    common_mode(C);

    _cmmd[0] = C.CommonMode();
    _cnoise[0] = C.Noise();


    C.Set("128-255");
    common_mode(C);

    _cmmd[1] = C.CommonMode();
    _cnoise[1] = C.Noise();
}

void TbAsciiRoot::common_mode(ChanList &C, bool correct)
{
    int ip, i, j;

    double mean, sm, xn, xx, xs, xm, tmp;
    bool use_it;
    mean = sm = 0.;
    for (ip=0;ip<3;ip++)
    {
        xn = xs = xm = 0.;
        for (j=0; j<C.Nch(); j++)
        {
            i = C[j];
            if (_mask[i])
                continue;

            use_it = true;
            xx = data(i) - _ped[i];
            if (ip)
            {
                tmp = fabs((xx-mean)/sm);
                use_it = (tmp<2.5);
            }
            if (use_it)
            {
                xn++;
                xm += xx;
                xs += xx * xx;
            }
        }
        if (xn>0.)
        {
            mean = xm / xn;
            sm = sqrt( xs/xn - mean*mean);
        }
        //  std::cout << "...iter " << ip << ": xm " << mean << " xs: " << sm << std::endl;
    }
    C.CommonMode(mean);
    C.Noise(sm);

    if (correct)
    {
        for ( j=0; j<C.Nch(); j++ )
        {
            i = C[j];
            _signal[i] = _data.data[i]-_ped[i] - C.CommonMode();
            _sn[i] = (_noise[i] >1. && !_mask[i] ? _signal[i]/_noise[i] : 0.);
        }
    }
}




/*
void TbAsciiRoot::spy_data(bool with_signal, int nevt)
{
    TVirtualPad *pad;
    if (!ifile)
        return;

    sighandler_t old_handler = ::signal(SIGINT, _A_got_intr);
    _A_do_run = true;

    TCanvas *cnvs = (TCanvas *)gROOT->FindObject("cnvs");
    if (cnvs)
    {
        cnvs->Clear();
    }
    else
       cnvs = new TCanvas("cnvs","cnvs", 700, 800);

    cnvs->Divide(2,3);


    TH1 *hsignal = create_h1("hsignal","signal (ADC)",256, -0.5, 255.0);
    hsignal->SetXTitle("Channel");
    hsignal->SetYTitle("ADC");
    hsignal->SetMinimum(-300);
    hsignal->SetMaximum(300);

    TH1 *helec = create_h1("helec","signal (elec)", 256, -0.5, 255.5);
    helec->SetXTitle("Channel");
    helec->SetYTitle("electrons");
    helec->SetMinimum(-300/gain());
    helec->SetMaximum(300/gain());

    TH1 *hraw = create_h1("hraw","Raw Data (around 512.)",256, 0., 256.);
    hraw->SetXTitle("Channel");
    hraw->SetYTitle("ADC");
    hraw->SetMinimum(-300);
    hraw->SetMaximum(+300);

    TH1 *hrawc = create_h1("hrawc","Raw Data (no commd)",256, 0., 256.);
    hrawc->SetXTitle("Channel");
    hrawc->SetYTitle("ADC");
    hrawc->SetMinimum(-300);
    hrawc->SetMaximum(+300);


    TH1 *hcmmd[2];
    hcmmd[0] = create_h1("hcmmd0","Common mode (Chip 0)",50,-100.,100.);
    hcmmd[0]->SetXTitle("Common mode");
    hcmmd[1] = create_h1("hcmmd1","Common mode (Chip 1)",50,-100.,100.);
    hcmmd[1]->SetXTitle("Common mode");

    int ievt,jevt;
    for (ievt=jevt=0; read_event()==0 && _A_do_run && ievt<nevt;jevt++)
    {
        process_event();
        find_clusters();
        if ( with_signal && empty())
            continue;

        int i,ichip=-1;
        for (i=0; i<nchan(); i++)
        {
            // TODO: figure out chip number
            if (!(i%128))
                ichip++;

            hsignal->SetBinContent(i+1, _signal[i]);
            helec->SetBinContent(i+1, signal(i));
            hraw->SetBinContent(i+1,data(i)-512.);
            hrawc->SetBinContent(i+1, data(i)-_ped[i]);
            // TODO: why we draw the signal + common mode ?
            //       May be cause signal should be ~0...
            hcmmd[ichip]->Fill(_signal[i]+get_cmmd(ichip));
        }
        pad = cnvs->cd(1);
        pad->SetGrid(1,1);
        hsignal->Draw();
        pad = cnvs->cd(2);
        pad->SetGrid(1,1);
        helec->Draw();

        pad = cnvs->cd(3);
        pad->SetGrid(1,1);
        hraw->Draw();

        pad = cnvs->cd(4);
        pad->SetGrid(1,1);
        hrawc->Draw();

        pad = cnvs->cd(5);
        pad->SetGrid(1,1);
        hcmmd[0]->Draw();

        pad = cnvs->cd(6);
        pad->SetGrid(1,1);
        hcmmd[1]->Draw();

        std::cout << std::setiosflags(std::ios::fixed);
        std::cout << "*** Event " << jevt << " *****" << std::endl;
        std::cout << "Common Mode:" << std::endl
                  << "   Chip 0 " << std::setw(6) << std::setprecision(1) << get_cmmd(0) << " noise: " << get_cnoise(0)
                  << std::endl
                  << "   Chip 1 " << std::setw(6) << std::setprecision(1) << get_cmmd(1) << " noise: " << get_cnoise(1)
                  << std::endl;

        std::cout << "Time: " << time() << " ns" << std::endl;
        std::cout << "Signal chan(0) << " << signal(0) << " chan(1) " << signal(1) << std::endl;
        std::cout << "Clusters: " << std::endl;

        TbAlibavaHitList::iterator ip;
        for (ip=begin(); ip!=end(); ++ip)
        {
            std::cout << "   chan: " << ip->center()
                      << " sig: "
                      << std::setw(6) << std::setprecision(1) << ip->signal()
                      << " left: " << ip->left() << " right: " << ip->right()
                      << std::endl;
            std::cout << '\t' << "channels: " << std::endl;
            int j;
            for (j=ip->left();j<=ip->right();j++)
                std::cout << "\t   " << j << " sn: " << _sn[j] << " signal: " << _signal[j] << " noise: " << _noise[j] << '\n';
            std::cout << std::endl;
        }

        cnvs->Update();
        ievt++;
    }
    std::cout << std::endl;
    _A_do_run= true;
    ::signal(SIGINT, old_handler);
}
*/

bool is_text(const char *fnam)
{
    int nc;
    char buffer[1024];
    std::ifstream ifile(fnam);
    if (!fnam)
        return false;

    ifile.read(buffer, sizeof(buffer));
    nc = ifile.gcount();
    ifile.close();
    if (!nc) // empty files are text
    {
        return true;
    }

    std::string ss(buffer, nc);
    ifile.close();

    if ( ss.find('\0') != ss.npos )
        return false;

    double nontext = 0.;
    double ntotal = 0.;
    std::string::iterator ip;
    for (ip=ss.begin(); ip!=ss.end(); ++ip)
    {
        ntotal++;
        char c = *ip;
        if ( (c<' ' || c >'~') && !strchr("\n\t\r\b", c) )
            nontext++;
    }
    if ( nontext/ntotal > 0.3 )
        return false;

    return true;
}

