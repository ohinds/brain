#pragma once

#include <map>
#include <string>
#include <vector>

#include "ui.h"

struct pa_context;
struct pa_threaded_mainloop;
struct pa_mainloop_api;
class RtMidiIn;
class Sample;

typedef std::map<char, Sample*> KeySampleMap;
typedef std::map<unsigned char, Sample*> MidiSampleMap;

class Brain {

 public:

  Brain(const std::string &config_file);

  ~Brain();

  void setReady(bool ready);

  void setInitError(bool init_error) {
    pa_init_error = init_error;
  }

  bool getInitError() const {
    return pa_init_error;
  }

  bool init();

  bool run();

  void stop();

 private:

  pa_context *context;
  pa_threaded_mainloop *mainloop;
  pa_mainloop_api *mainloop_api;

  bool pa_context_ready;
  bool pa_init_error;
  bool should_stop;

  std::vector<Sample*> samples;
  KeySampleMap key_sample_map;
  MidiSampleMap midi_sample_map;

  RtMidiIn *midi_in;
  Ui ui;
};
