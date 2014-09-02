#pragma once

#include <deque>
#include <utility>

#include <alsa/asoundlib.h>

class Sample;

typedef std::pair<Sample*, size_t> ActiveSample;

class Mixer {

 public:

  Mixer();

  ~Mixer();

  bool init();

  bool play(Sample *sample);

  bool tick();

 private:

  snd_pcm_t *pcm_handle;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;

  std::deque<ActiveSample> active_samples;

  short *next_buffer;
  bool next_buffer_computed;
};
