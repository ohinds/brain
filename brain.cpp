#include "brain.h"

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <vector>

#include <sndfile.h>
#include <pulse/pulseaudio.h>
#include <RtMidi.h>
#include <yaml.h>

#include "sample.h"
#include "player.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace {

void pa_context_state_callback(pa_context *c, void *user_data) {
  assert(c);

  Brain *brain = static_cast<Brain*>(user_data);

  switch (pa_context_get_state(c)) {
      case PA_CONTEXT_CONNECTING:
      case PA_CONTEXT_AUTHORIZING:
      case PA_CONTEXT_SETTING_NAME: {
        break;
      }
      case PA_CONTEXT_READY: {
        brain->setReady(true);
        break;
      }
      case PA_CONTEXT_TERMINATED: {
        brain->setInitError(true);
        break;
      }
      case PA_CONTEXT_FAILED:
      default: {
        brain->setInitError(true);
      }
  }
}

void pa_exit_signal_callback(pa_mainloop_api *mainloop_api,
                             pa_signal_event *event,
                             int signal,
                             void *user_data) {
  static_cast<Brain*>(user_data)->stop();
}

} // anonymous namespace

Brain::Brain(const std::string &config_file)
  : context(NULL)
  , mainloop(NULL)
  , mainloop_api(NULL)
  , pa_context_ready(false)
  , pa_init_error(false)
  , should_stop(false) {

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

    if ((*it)["midi"]) {
      midi_sample_map[(*it)["midi"].as<int>()] = sample;
    }

    if ((*it)["pan"]) {
      sample->setPan((*it)["pan"].as<float>());
    }
  }
}

Brain::~Brain() {
  if (context) {
    pa_context_unref(context);
  }

  if (mainloop) {
    pa_signal_done();
    mainloop_api->quit(mainloop_api, 0);
    pa_threaded_mainloop_free(mainloop);
  }

  for (vector<Sample*>::iterator it = samples.begin();
       it != samples.end(); ++it) {
    delete *it;
  }

  delete midi_in;
}

void Brain::setReady(bool ready) {
  pa_context_ready = ready;

  if (ready) {
    pa_threaded_mainloop_signal(mainloop, 0);
  }
}

bool Brain::init() {
  // initialize pulse audio
  mainloop = pa_threaded_mainloop_new();
  if (mainloop == NULL) {
    ui << "Creating pulseaudio mainloop failed\n";
    return false;
  }

  mainloop_api = pa_threaded_mainloop_get_api(mainloop);
  pa_signal_init(mainloop_api);
  pa_signal_new(SIGINT, pa_exit_signal_callback, this);

  context = pa_context_new(mainloop_api, "brain");
  if (context == NULL) {
    ui << "Creating pulseaudio context failed\n";
    return false;
  }

  pa_context_set_state_callback(context, pa_context_state_callback, this);

  if (pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
    ui << "Connecting to pulseaudio context failed\n";
    return false;
  }

  pa_threaded_mainloop_lock(mainloop);
  pa_threaded_mainloop_start(mainloop);

  do {
    pa_threaded_mainloop_wait(mainloop);
  } while (!pa_context_ready && !pa_init_error);

  pa_threaded_mainloop_unlock(mainloop);

  // initialize Midi
  midi_in = new RtMidiIn();
  if (midi_in == NULL) {
    ui << "Failed to initialize Midi input\n";
    return false;
  }

  size_t num_ports = midi_in->getPortCount();
  ui << "There are " << num_ports
     << " MIDI input sources available.\n";
  std::string portName;
  for ( unsigned int i=0; i<num_ports; i++ ) {
    portName = midi_in->getPortName(i);
    ui << "  Input Port #" << i << ": " << portName << "\n";
  }

  if (num_ports == 0) {
    ui << "No Midi ports were found\n";
    return false;
  }

  midi_in->openPort(1);
  midi_in->ignoreTypes(false, false, false);

  return true;
}

bool Brain::run() {
  vector<Player*> players;

  ui << "ready\n";
  while (!should_stop) {
    // check for done samples
    for (vector<Player*>::iterator it = players.begin();
         it != players.end();) {
      if ((*it)->isFinished()) {
        Player *done_player = *it;
        it = players.erase(it);
        delete done_player;
      }
      else {
        ++it;
      }
    }

    // check for new midi messages
    vector<unsigned char> message;
    midi_in->getMessage(&message);
    if (message.size() > 1) {
      const unsigned char NOTE_ON_OR_OFF = 144;
      if (message[0] == NOTE_ON_OR_OFF && message[2] > 0) { // note on
        MidiSampleMap::const_iterator midi_sample =
          midi_sample_map.find(message[1]);
        if (midi_sample != midi_sample_map.end()) {
          const char MAX_MIDI_VOL = 127;
          float volume = ((float) message[2]) / MAX_MIDI_VOL;
          players.push_back(
            midi_sample->second->play(mainloop, context, volume));
        }
      }
    }

    // check for keyboard input
    char in = ui.getInput();
    if (in == 'q') {
      stop();
      continue;
    }

    KeySampleMap::const_iterator key_sample = key_sample_map.find(in);
    if (key_sample != key_sample_map.end()) {
      players.push_back(key_sample->second->play(mainloop, context, 1.0));
    }
  }

  ui << "quit\n";

  return true;
}

void Brain::stop() {
  should_stop = true;
}
