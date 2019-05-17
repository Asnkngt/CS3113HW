#pragma once
#include "Globals.h"

void AudioInit();

int LoadSound(const std::string& file);

void StopSound(int soundIndex);
void VolumeSound(int soundIndex, int volume);
void PlaySound(int soundIndex, bool loop, float volume);

void AudioHalt();