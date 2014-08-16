#include "brain.h"

#include <iostream>
#include <unistd.h>
#include <vector>

#include <sndfile.h>
#include <pulse/pulseaudio.h>
#include <yaml.h>

#include "sample.h"
#include "player.h"
#include "terminal_input.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace {

void context_state_callback(pa_context *c, void *user_data) {
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

void exit_signal_callback(pa_mainloop_api *mainloop_api,
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
      cerr << "name and file are required fields for each sample" << endl;
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
  }
}

Brain::~Brain() {
  if (context) {
    pa_context_unref(context);
  }

  if (mainloop) {
    pa_signal_done();
    pa_threaded_mainloop_free(mainloop);
  }

  mainloop_api->quit(mainloop_api, 0);

  for (vector<Sample*>::iterator it = samples.begin();
       it != samples.end(); ++it) {
    delete *it;
  }
}

void Brain::setReady(bool ready) {
  pa_context_ready = ready;

  if (ready) {
    pa_threaded_mainloop_signal(mainloop, 0);
  }
}

bool Brain::init() {
  mainloop = pa_threaded_mainloop_new();
  if (mainloop == NULL) {
    cerr << "Creating pulseaudio mainloop failed" << endl;
    return false;
  }

  mainloop_api = pa_threaded_mainloop_get_api(mainloop);
  pa_signal_init(mainloop_api);
  pa_signal_new(SIGINT, exit_signal_callback, this);

  context = pa_context_new(mainloop_api, "brain");
  if (context == NULL) {
    cerr << "Creating pulseaudio context failed" << endl;
    return false;
  }

  pa_context_set_state_callback(context, context_state_callback, this);

  if (pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
    cerr << "Connecting to pulseaudio context failed" << endl;
    return false;
  }

  pa_threaded_mainloop_lock(mainloop);
  pa_threaded_mainloop_start(mainloop);

  do {
    pa_threaded_mainloop_wait(mainloop);
  } while (!pa_context_ready && !pa_init_error);

  pa_threaded_mainloop_unlock(mainloop);

  return true;
}

bool Brain::run() {
  vector<Player*> players;
  TerminalInput terminal_input;
  terminal_input.start();

  while (!should_stop) {
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

    char in = terminal_input.getNextChar();
    if (in == 'q') {
      stop();
      continue;
    }

    KeySampleMap::const_iterator key_sample = key_sample_map.find(in);
    if (key_sample != key_sample_map.end()) {
      players.push_back(key_sample->second->play(mainloop, context));
    }
  }

  cout << "quitting" << endl;
  terminal_input.stop();

  return true;
}

void Brain::stop() {
  should_stop = true;
}
