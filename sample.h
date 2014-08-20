#pragma once

#include <cmath>
#include <string>
#include <pulseaudio.h>

class Player;

class Sample {

 public:

  Sample(const std::string &filename);

  ~Sample();

  // begin playback at the specified volume. Volume == 1.0 is max
  Player* play(pa_threaded_mainloop *m, pa_context* c, float volume);

  const std::string& getFilename() const {
    return filename;
  }

  const pa_sample_spec& getSampleSpec() const {
    return sample_spec;
  }

  size_t getFrameSize() const {
    return frame_size;
  }

  size_t getTotalBytes() const {
    return total_bytes;
  }

  const short * const getFrames() const {
    return frames;
  }

  const pa_channel_map* const getChannelMap() const {
    return &channel_map;
  }

  // set the sample's playback pan. -1.0 is all left, 1.0 is all right,
  // 0.0 is in the middle.
  void setPan(float _pan) {
    pan = _pan;
    if (pan == 0.) {
      pan_correction = 1.0;
    }
    else {
      pan_correction = 1 + fabs(pan) / 2;
    }
  }

  float getPan() const {
    return pan;
  }

  float getPanCorrection() const {
    return pan_correction;
  }

 private:

  std::string filename;

  pa_sample_spec sample_spec;

  size_t frame_size;
  size_t total_bytes;
  short *frames;

  pa_channel_map channel_map;
  float pan;
  float pan_correction;
};
