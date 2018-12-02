#ifndef AUDIO_H
#define AUDIO_H

/*
 * Define classes for handling audio input and output.  The example is taken from:
 *
 * https://www.linuxjournal.com/article/6735
 */

// using new ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

namespace nc {
    class AudioPlayer {
        snd_pcm_t *           handle;
        snd_pcm_hw_params_t * parameters;
        snd_pcm_uframes_t     frames;
        char *                buffer;

    public:
        AudioPlayer();
        ~AudioPlayer();

        void play();
    };

    class AudioRecorder {
        snd_pcm_t *           handle;
        snd_pcm_hw_params_t * parameters;
        snd_pcm_uframes_t     frames;
        char *                buffer;

    public:
        AudioRecorder();
        ~AudioRecorder();

        void record();
    };
} // namespace nc

#endif // AUDIO_H
