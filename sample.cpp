#include "sample.h"

#include <iostream>
#include <string>

#include <sndfile.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Sample::Sample(const string &filename)
  : filename(filename)
  , num_frames(0)
  , num_channels(0)
  , frames(NULL)
  , volume(1.)
  , pan(0.)
  , pan_correction(1.)
{
  SF_INFO info;
  SNDFILE *snd_file = sf_open(filename.c_str(), SFM_READ, &info);

  if (!snd_file) {
    cerr << "Couldn't open " << filename << endl;
    return;
  }

  if ((info.format & 0xFF) != SF_FORMAT_PCM_16) {
    cerr << "Unsupported audio format in " << filename << endl;
    return;
  }

  num_frames = info.frames;
  num_channels = info.channels;

  frames = new short[info.frames * info.channels];
  sf_readf_short(snd_file, frames, info.frames);
  sf_close(snd_file);
}

Sample::~Sample() {
  delete [] frames;
}

short Sample::
getFrameChannelVal(size_t frame, bool left) const {
  short base_val = 0;

  if (num_channels == 1 || left) {
    base_val = *(frames + frame * num_channels);
  }
  else {
    base_val = *(frames + frame * num_channels + 1);
  }

  if (left) {
    return (1 - pan) * 0.5 * volume * base_val;
  }
  else {
    return (pan + 1) * 0.5 * volume * base_val;
  }
}
