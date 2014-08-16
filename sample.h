#pragma once

#include<string>
#include <pulseaudio.h>

class Player;

class Sample {

 public:

  Sample(const std::string &filename);

  ~Sample();

  Player* play(pa_threaded_mainloop *m, pa_context* c);

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

 private:

  std::string filename;

  pa_sample_spec sample_spec;
  size_t frame_size;
  size_t total_bytes;
  short *frames;

};
