#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TCanvas.h>

/**
 * Utils
 */
inline bool isws(char c, char const * const wstr=" \t\n")
{
    return (strchr(wstr, c) != NULL);
}

inline std::string trim_right(const std::string &s)
{
    std::string b=" \t\n";
    std::string str = s;
    return str.erase(str.find_last_not_of(b) +1);
}

inline std::string trim_left(const std::string &s)
{
    std::string b=" \t\n";
    std::string str = s;
    return str.erase( 0, str.find_first_not_of(b) );
}

inline std::string trim_str(const std::string &s)
{
    std::string str = s;
    return trim_left(trim_right(str) );
}

TCanvas *create_canvas(const char *name, const char *title, int wx=-1, int wy=-1);
TH1 *create_h1(const char *, const char *, int, double, double);
TH2 *create_h2(const char *, const char *, int, double, double, int, double, double);
TProfile *create_profile(const char *, const char *, int, double, double, double, double);
TProfile2D *create_profile2d(const char *name, const char *tit, int nx, double x1, double x2, int ny, double y1, double y2);



#endif /*UTILS_H_*/
