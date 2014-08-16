#include "sample.h"

#include <iostream>
#include <string>

#include <sndfile.h>

#include "player.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

Sample::Sample(const string &filename)
  : filename(filename)
  , frame_size(0)
  , total_bytes(0)
  , frames(NULL)
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

  sample_spec.rate = (uint32_t) info.samplerate;
  sample_spec.channels = (uint8_t) info.channels;
  sample_spec.format = PA_SAMPLE_S16NE;

  frame_size = pa_frame_size(&sample_spec);

  // pulse audio refuses to play samples with a total number of frames
  // fewer than that requested in the first stream_write_callback. Add
  // some empty audio to the end of short samples so they'll play.
  const int MIN_FRAMES = 30000;
  if (info.frames < MIN_FRAMES) {
    info.frames = MIN_FRAMES;
  }
  total_bytes = info.frames * frame_size;

  frames = (short*) pa_xmalloc(total_bytes);
  sf_readf_short(snd_file, frames, info.frames);
  sf_close(snd_file);
}

Sample::~Sample() {
  pa_xfree(frames);
}

Player* Sample::play(pa_threaded_mainloop *m, pa_context *c) {
  Player *player = new Player(this, m);
  player->start(c);
  return player;
}
