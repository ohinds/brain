#include "player.h"

#include <cstring>
#include <iostream>

#include "sample.h"

using std::cerr;
using std::cout;
using std::endl;

namespace {

void stream_write_callback(pa_stream *s, size_t bytes, void *user_data) {
  Player *player = static_cast<Player*>(user_data);
  player->writeData(s, bytes);
}

void stream_drain_complete(pa_stream *s, int success, void *user_data) {
  // intentionally left blank
}

} // anonymous namespace


Player::Player(Sample *sample, pa_threaded_mainloop *m)
  : sample(sample)
  , mainloop(m)
  , stream(NULL)
  , next_frame(0)
  , done(false)
{}

void Player::start(pa_context * c, float vol_ratio) {
  pa_threaded_mainloop_lock(mainloop);

  stream = pa_stream_new(c, "", &sample->getSampleSpec(), NULL);
  if (stream == NULL) {
    pa_threaded_mainloop_unlock(mainloop);
    cerr << "Error creating stream" << endl;
    return;
  }

  pa_volume_t volume = sample->getPanCorrection() * vol_ratio * PA_VOLUME_NORM;
  pa_cvolume *cvolume = new pa_cvolume;
  cvolume = pa_cvolume_set(cvolume, sample->getSampleSpec().channels, volume);

  cvolume = pa_cvolume_set_balance(cvolume, sample->getChannelMap(),
                                   sample->getPan());

  pa_stream_set_write_callback(stream, stream_write_callback, this);
  pa_stream_connect_playback(
    stream, NULL, NULL, PA_STREAM_NOFLAGS, cvolume, NULL);

  delete cvolume;
  pa_threaded_mainloop_unlock(mainloop);
}

Player::~Player() {
  pa_threaded_mainloop_lock(mainloop);
  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
  pa_threaded_mainloop_unlock(mainloop);
  stream = NULL;
}

bool Player::writeData(pa_stream *stream, size_t num_bytes) {
  const bool STUPID = false;

  if (done) {
    return false;
  }

  if (next_frame * sizeof(short) + num_bytes > sample->getTotalBytes()) {
    num_bytes = sample->getTotalBytes() - next_frame * sizeof(short) - 1;
  }

  if (!STUPID) {
    pa_stream_write(stream, sample->getFrames() + next_frame, num_bytes,
                    NULL, 0, PA_SEEK_RELATIVE);
    next_frame += num_bytes / sizeof(short);
  }
  else {
    pa_stream_write(
      stream, sample->getFrames() + next_frame * sample->getFrameSize(),
      num_bytes * sample->getFrameSize() / 2, NULL, 0, PA_SEEK_RELATIVE);
    next_frame += num_bytes / sample->getFrameSize() + 1000;
  }

  if (next_frame * sizeof(short) >= sample->getTotalBytes() - 2) {
    pa_stream_set_write_callback(stream, NULL, NULL);
    pa_stream_drain(stream, stream_drain_complete, this);
    done = true;
  }

  return true;
}
