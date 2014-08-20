#pragma once

#include <pulseaudio.h>

class Sample;

class Player {

 public:

  Player(Sample *sample, pa_threaded_mainloop *mainloop);

  ~Player();

  // begin sample playback. Volume == 1.0 is full volume.
  void start(pa_context* c, float volume);

  bool writeData(pa_stream *stream, size_t num_frames);

  bool isFinished() const {
    return done;
  }

 private:

  Sample *sample;
  pa_threaded_mainloop *mainloop;
  pa_stream *stream;
  size_t next_frame;
  bool done;
};
