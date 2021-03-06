#include "brain.h"

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <vector>

#include <sndfile.h>
#include <RtMidi.h>
#include <yaml.h>

#include "sample.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

Brain::Brain(const std::string &config_file)
  : init_error(false)
  , should_stop(false)
  , use_midi(true)
  , midi_in(NULL)
  , ui(true) {

  YAML::Node node = YAML::LoadFile(config_file);
  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    if (!(*it)["name"] || !(*it)["file"]) {
      ui << "name and file are required fields for each sample\n";
      continue;
    }

    string file((*it)["file"].as<string>());

    Sample *sample = new Sample(file);
    samples.push_back(sample);

    if ((*it)["key"]) {
      key_sample_map[(*it)["key"].as<char>()] = sample;
    }

    if (use_midi && (*it)["midi"]) {
      midi_sample_map[(*it)["midi"].as<int>()] = sample;
    }

    if ((*it)["pan"]) {
      sample->setPan((*it)["pan"].as<float>());
    }
  }
}

Brain::~Brain() {
  for (vector<Sample*>::iterator it = samples.begin();
       it != samples.end(); ++it) {
    delete *it;
  }

  if (midi_in != NULL) {
    delete midi_in;
  }
}

bool Brain::init() {
  // init the mixer
  if (!mixer.init()) {
    return false;
  }

  // initialize Midi
  if (use_midi) {
    midi_in = new RtMidiIn();
    if (midi_in == NULL) {
      ui << "Failed to initialize Midi input\n";
      return false;
    }

    size_t num_ports = midi_in->getPortCount();
    ui << "There are " << num_ports
       << " MIDI input sources available.\n";
    std::string portName;
    for ( unsigned int i=0; i < num_ports; i++ ) {
      portName = midi_in->getPortName(i);
      ui << "  Input Port #" << i << ": " << portName << "\n";
    }

    if (num_ports == 0) {
      ui << "No Midi ports were found\n";
      return false;
    }

    midi_in->openPort(num_ports - 1);
    midi_in->ignoreTypes(false, false, false);
  }

  return true;
}

bool Brain::run() {

  ui << "ready\n";
  while (!should_stop) {
    // tick the mixer
    mixer.tick();

    // check for new midi messages
    if (use_midi) {
      vector<unsigned char> message;
      midi_in->getMessage(&message);
      if (message.size() > 1) {
        const unsigned char NOTE_ON_OR_OFF = 144;
        if (message[0] == NOTE_ON_OR_OFF && message[2] > 0) { // note on
          MidiSampleMap::const_iterator midi_sample =
            midi_sample_map.find(message[1]);
          if (midi_sample != midi_sample_map.end()) {
            const char MAX_MIDI_VOL = 127;
            float velocity = ((float) message[2]) / MAX_MIDI_VOL;
            mixer.play(midi_sample->second, velocity);
          }
        }
      }
    }

    // check for keyboard input
    char in = ui.getInput();
    if (in == 255) {
      continue;
    }

    KeySampleMap::const_iterator key_sample = key_sample_map.find(in);
    if (key_sample != key_sample_map.end()) {
      mixer.play(key_sample->second, 1.0);
      continue;
    }

    handleKeyboardCommand(in);
  }

  ui << "quit\n";

  return true;
}

void Brain::handleKeyboardCommand(char c) {
  switch(c) {
      case 'q':
        stop();
        break;

      case '=':
      case '+':
        mixer.increaseMasterVolume();
        break;

      case '-':
      case '_':
        mixer.decreaseMasterVolume();
        break;

      default:
        break;
  }
}

void Brain::stop() {
  should_stop = true;
}
