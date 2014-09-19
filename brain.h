#pragma once

#include <map>
#include <string>
#include <vector>

#include "mixer.h"
#include "ui.h"

class Mixer;
class RtMidiIn;
class Sample;

typedef std::map<char, Sample*> KeySampleMap;
typedef std::map<unsigned char, Sample*> MidiSampleMap;

class Brain {

 public:

  Brain(const std::string &config_file);

  ~Brain();

  bool init();

  bool run();

  void handleKeyboardCommand(char c);

  void stop();

 private:

  bool init_error;
  bool should_stop;

  std::vector<Sample*> samples;
  KeySampleMap key_sample_map;
  MidiSampleMap midi_sample_map;

  Mixer mixer;

  bool use_midi;
  RtMidiIn *midi_in;

  Ui ui;
};
