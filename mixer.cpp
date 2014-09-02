#include "mixer.h"

//#include <algorithm>

#include <iostream>

#include "sample.h"

using std::cerr;
using std::cout;
using std::endl;
using std::deque;
using std::fill;

namespace {

const size_t NUM_CHANNELS = 2;

bool check_alsa_error(int error_code) {
  if (error_code < 0) {
    cerr << snd_strerror(error_code) << ": " << error_code << endl;
    return false;
  }

  return true;
}

} // anonymous namespace

Mixer::Mixer()
  : pcm_handle(NULL)
  , buffer_size(1024)
  , period_size(64)
  , next_buffer(NULL)
  , next_buffer_computed(false)
{}

Mixer::~Mixer() {
  if (pcm_handle != NULL) {
    snd_pcm_close(pcm_handle);
  }

  //delete next_buffer;
}


bool Mixer::init() {

  // TODO consider SND_PCM_NONBLOCK
  if (!check_alsa_error(
        snd_pcm_open(&pcm_handle, "default",
                     SND_PCM_STREAM_PLAYBACK,
                     0))) {
    return false;
  }

  // hardware parameters
  snd_pcm_hw_params_t *hw_params;
  if (!check_alsa_error(snd_pcm_hw_params_malloc(&hw_params))) {
    return false;
  }

  if (!check_alsa_error(snd_pcm_hw_params_any(pcm_handle, hw_params))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_hw_params_set_access(pcm_handle, hw_params,
                                     SND_PCM_ACCESS_RW_INTERLEAVED))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_hw_params_set_format(pcm_handle, hw_params,
                                     SND_PCM_FORMAT_S16_LE))) {
    return false;
  }

  unsigned int rrate = 44100;
  if (!check_alsa_error(
        snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rrate, NULL))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_hw_params_set_channels(pcm_handle, hw_params, NUM_CHANNELS))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params,
                                               &buffer_size))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params,
                                               &period_size, NULL))) {
    return false;
  }

  if (!check_alsa_error(snd_pcm_hw_params(pcm_handle, hw_params))) {
    return false;
  }

  snd_pcm_hw_params_free(hw_params);

  // software parameters
  snd_pcm_sw_params_t *sw_params;

  if (!check_alsa_error(snd_pcm_sw_params_malloc(&sw_params))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params,
                                              buffer_size - period_size))) {
    return false;
  }

  if (!check_alsa_error(
        snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size))) {
    return false;
  }

  if (!check_alsa_error(snd_pcm_sw_params(pcm_handle, sw_params))) {
    return false;
  }

  snd_pcm_sw_params_free(sw_params);

  if (!check_alsa_error(snd_pcm_prepare(pcm_handle))) {
    return false;
  }

  char *buffer = new char[NUM_CHANNELS * period_size]();
  if (!check_alsa_error(snd_pcm_writei(pcm_handle, buffer,
                                       NUM_CHANNELS * period_size))) {
    delete [] buffer;
    return false;
  }

  delete [] buffer;

  // create the buffer for storing the next buffer to be writen
  next_buffer = new short[buffer_size];

  return true;
}

bool Mixer::play(Sample *sample) {
  active_samples.push_back(ActiveSample(sample, 0));
  return true;
}

bool Mixer::tick() {

  if (!next_buffer_computed) {
    fill(next_buffer,
         next_buffer + buffer_size * NUM_CHANNELS * sizeof(short), 0);
    for (deque<ActiveSample>::iterator it = active_samples.begin();
         it != active_samples.end(); ) {
      Sample *sample = it->first;
      size_t next_frame = it->second;

      bool done = false;
      for (size_t i = 0; i < buffer_size; i++) {
        if (next_frame + i >= sample->getNumFrames()) {
          it = active_samples.erase(it);
          done = true;
          break;
        }

        for (size_t c = 0; c < NUM_CHANNELS; c++) {
          next_buffer[NUM_CHANNELS * i + c] +=
            sample->getFrameChannelVal(next_frame + i, c == 0);
        }
      }

      if (!done) {
        it->second += buffer_size;
        ++it;
      }
    }

    next_buffer_computed = true;
  }

  int error_code = snd_pcm_writei(pcm_handle, next_buffer, buffer_size);
  if (error_code == -EAGAIN) {
    // do nothing
  }
  else if (error_code == -EPIPE) {
    if (!check_alsa_error(snd_pcm_prepare(pcm_handle))) {
      return true;
    }
  }
  else if(!check_alsa_error(error_code)) {
    return false;
  }
  else {
    next_buffer_computed = false;
  }

  return true;
}
