#pragma once

#include <cmath>
#include <string>

class Sample {

 public:

  Sample(const std::string &filename);

  ~Sample();

  const std::string& getFilename() const {
    return filename;
  }

  size_t getNumFrames() const {
    return num_frames;
  }

  size_t getNumChannels() const {
    return num_channels;
  }

  const short * const getFrames() const {
    return frames;
  }

  const short * const getFrame(size_t frame) const {
    return frames + frame * num_channels;
  }

  short getFrameChannelVal(size_t frame, bool left) const;

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

  size_t num_frames;
  size_t num_channels;
  short *frames;

  float volume;

  float pan;
  float pan_correction;
};
