#ifndef TB_CONSTANTS_H
#define TB_CONSTANTS_H 1

namespace Tb {

// Time units
static const uint64_t ToA = 4096;
static const uint64_t SpidrTime = 16384 * ToA;

static const double nanosecond = ToA / 25.;
static const uint64_t millisecond = 40000 * ToA;
static const uint64_t second = 1000 * millisecond;
static const uint64_t minute = 60 * second;

static const unsigned int NRows = 256;
static const unsigned int NCols = 256;
static const unsigned int NPixels = NRows * NCols;

static const double PixelPitch = 0.055;
}

#endif
