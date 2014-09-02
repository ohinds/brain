#pragma once

#include <deque>
#include <utility>

#include <alsa/asoundlib.h>

class Sample;

struct ActiveSample {
  Sample *sample;
  size_t next_frame;
  float velocity;

  ActiveSample(Sample *sample, float velocity)
    : sample(sample)
    , next_frame(0)
    , velocity(velocity) {}
};

class Mixer {

 public:

  Mixer();

  ~Mixer();

  bool init();

  bool play(Sample *sample, float velocity);

  bool tick();

  void increaseMasterVolume() {
    master_volume += 0.1;
    if (master_volume > 1.0) {
      master_volume = 1.0;
    }
  }

  void decreaseMasterVolume() {
    master_volume -= 0.1;
    if (master_volume < 0.) {
      master_volume = 0.;
    }
  }

 private:

  snd_pcm_t *pcm_handle;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;

  float master_volume;

  std::deque<ActiveSample> active_samples;

  short *next_buffer;
  bool next_buffer_computed;
};
