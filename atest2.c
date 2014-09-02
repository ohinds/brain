#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

char buffer[1000000];
snd_pcm_uframes_t buffer_size = 1024;
snd_pcm_uframes_t period_size = 64;
snd_pcm_t *pcm_handle = NULL;


void callback(snd_async_handler_t *pcm_callback) {
  snd_pcm_t *pcm_handle = snd_async_handler_get_pcm(pcm_callback);
  snd_pcm_sframes_t avail;
  int err;

  avail = snd_pcm_avail_update(pcm_handle);
  while (avail >= period_size) {
     snd_pcm_writei(pcm_handle, buffer, period_size);
     avail = snd_pcm_avail_update(pcm_handle);
  }

  printf ("what\n");

}


int bad(int err) {
  /* Error check */
  if (err < 0) {
    fprintf (stderr, "%s: %d\n", snd_strerror (err), err);
    return 1;
  }

  return 0;
}


int main() {
  /* This holds the error code returned */
  int err, i;

  for (i = 0; i < 2 * buffer_size; i++) {
    buffer[i] = i;
  }

  /* The device name */
  const char *device_name = "default";

  err = snd_pcm_open (&pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
  if (bad(err)) {
    return 1;
  }

  snd_pcm_hw_params_t *hw_params;
  err = snd_pcm_hw_params_malloc (&hw_params);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_any(pcm_handle, hw_params);
  if (bad(err)) {
    return 1;
  }

  unsigned int rrate = 44100;
  err = snd_pcm_hw_params_set_access (pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_set_format (pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_set_rate_near (pcm_handle, hw_params, &rrate, NULL);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_set_channels (pcm_handle, hw_params, 2);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_set_buffer_size_near (pcm_handle, hw_params, &buffer_size);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params_set_period_size_near (pcm_handle, hw_params, &period_size, NULL);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_hw_params (pcm_handle, hw_params);
  if (bad(err)) {
    return 1;
  }

  snd_pcm_hw_params_free (hw_params);

  snd_pcm_sw_params_t *sw_params;

  snd_pcm_sw_params_malloc (&sw_params);
  err = snd_pcm_sw_params_current (pcm_handle, sw_params);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, buffer_size - period_size);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_sw_params(pcm_handle, sw_params);
  if (bad(err)) {
    return 1;
  }

  snd_pcm_sw_params_free (sw_params);

  err = snd_pcm_prepare (pcm_handle);
  if (bad(err)) {
    return 1;
  }

  err = snd_pcm_writei (pcm_handle, buffer, 2 * period_size);
  if (bad(err)) {
    return 1;
  }

  /* snd_async_handler_t *pcm_callback; */
  /* snd_async_add_pcm_handler(&pcm_callback, pcm_handle, callback, NULL); */


  /* snd_pcm_start(pcm_handle); */

  while (1) {
    err = snd_pcm_writei (pcm_handle, buffer, 2 * period_size);
    if (err == -32) {
      err = snd_pcm_prepare (pcm_handle);
      if (bad(err)) {
        return 1;
      }
    }
    else if (bad(err)) {
      return 1;
    }
    usleep(10);
  }

  //snd_async_del_handler (pcm_callback);

  //err = snd_pcm_close (pcm_handle);
  /* if (bad(err)) { */
  /*   return 1; */
  /* } */

  return 0;
}
