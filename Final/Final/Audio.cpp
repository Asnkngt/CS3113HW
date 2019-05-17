#include "Audio.h"
#include <SDL_mixer.h>

SDL_AudioSpec deviceSpec;
SDL_AudioDeviceID device;
unsigned int numSamples = 0;

class MixerSound {
public:
    Uint32 offset;
    Uint32 length;
    Uint8 *buffer;
    float volume;
    SDL_AudioFormat format;
    bool loaded;
    bool playing;
    bool loop;
};
void appAudioCallback(void *userdata, Uint8 *stream, int len);
std::vector<MixerSound> mixerSounds;

void ChannelCallback(int chan) {}

float getAudioForTime(long numSamples) {
    double elapsed = ((double)numSamples) / 44100.0;
    return (float)sin(elapsed * 2.0 * M_PI * 440.0);
}

void AudioCallback(void *userdata, Uint8 *stream, int len) {
    for (int i = 0; i < len / 4; i++) {
        ((float*)stream)[i] = getAudioForTime(numSamples);
        numSamples++;
    }
}

void AudioInit(){
    deviceSpec.freq = 44100; // sampling rate (samples a second)
    deviceSpec.format = AUDIO_F32; // audio format
    deviceSpec.channels = 1; // how many channels (1 = mono, 2 = stereo)
    deviceSpec.samples = 512; // audio buffer size in samples (power of 2)
    deviceSpec.callback = appAudioCallback; // our callback function

    // open new audio device with our requested settings
    device = SDL_OpenAudioDevice(NULL, 0, &deviceSpec, 0,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    SDL_PauseAudioDevice(device, 0);
}

int LoadSound(const std::string& file) {
    Uint8 *buffer;
    SDL_AudioSpec spec;
    Uint32 bufferSize;

    if (SDL_LoadWAV(file.c_str(), &spec, &buffer, &bufferSize) == NULL) {
        return -1;
    }

    SDL_AudioCVT cvt;
    SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
        deviceSpec.format, deviceSpec.channels, deviceSpec.freq);
    cvt.len = bufferSize;
    cvt.buf = new Uint8[bufferSize * cvt.len_mult];
    memcpy(cvt.buf, buffer, bufferSize);

    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(buffer);

    MixerSound sound;
    sound.buffer = cvt.buf;
    sound.length = cvt.len_cvt;
    sound.loaded = true;
    sound.offset = 0;
    sound.format = deviceSpec.format;
    sound.volume = 1.0;
    sound.playing = false;
    mixerSounds.push_back(sound);

    return mixerSounds.size() - 1;
}


void AudioHalt() {
    for (MixerSound& sound : mixerSounds) {
        sound.playing = false;
        sound.loop = false;
        sound.offset = 0;
    }
}

float mixSamples(float A, float B) {
    if (A < 0 && B < 0) { return (float)((A + B) - (A * B) / -1.0); }
    else if (A > 0 && B > 0) { return (float)((A + B) - (A * B) / 1.0); }
    else { return A + B; }
}

void appAudioCallback(void *userdata, Uint8 *stream, int len) {
    memset(stream, 0, len);
    for (size_t i = 0; i < mixerSounds.size(); i++) {
        MixerSound *sound = &mixerSounds[i];
        if (sound->loaded && sound->playing) {
            for (int s = 0; s < len / 4 && sound->offset + s * 4 < sound->length; s++) {
                float *sourceBuffer = (float*)(sound->buffer + sound->offset);
                ((float*)stream)[s] = mixSamples(((float*)stream)[s], (sourceBuffer[s] * sound->volume));
            }
            sound->offset += len;
            if (sound->offset >= sound->length - len) {
                if (sound->loop) {
                    sound->offset = 0;
                }
                else {
                    sound->playing = false;
                }
            }
        }
    }
}

void StopSound(int soundIndex) {
    mixerSounds[soundIndex].playing = true;
    mixerSounds[soundIndex].offset = 0;
}

void VolumeSound(int soundIndex, int volume) {
    mixerSounds[soundIndex].volume = volume;
    if (volume = 0) {
        mixerSounds[soundIndex].playing = false;
    }
}

void PlaySound(int soundIndex, bool loop, float volume) {
    if (soundIndex >= 0 && soundIndex < mixerSounds.size()) {
        mixerSounds[soundIndex].playing = true;
        mixerSounds[soundIndex].offset = 0;
        mixerSounds[soundIndex].loop = loop;
        mixerSounds[soundIndex].volume = volume;
    }
}