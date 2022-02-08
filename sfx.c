#ifndef SFX_C
#define SFX_C

#include <SDL.h>

#include "sfx.h"

void initialize_sfx(Game *game) {
  SDL_Init(SDL_INIT_AUDIO);

  game->sfx = malloc(NUM_SFX * sizeof(wav_buffer));
  char* sfx_names[] = { "chomp.wav", "die.wav" };
  SDL_AudioSpec wav_spec;
  for (int i = 0; i < NUM_SFX; i++) {
    SDL_LoadWAV(sfx_names[i], &wav_spec, &game->sfx[i].buffer, &game->sfx[i].length);
    game->sfx[i].name = sfx_names[i];
  }

  game->audio_device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
}

void play_sound(Game *game, char* file) {
  for (int i = 0; i < NUM_SFX; i++) {
    if (strcmp(file, game->sfx[i].name) == 0) {
      SDL_QueueAudio(game->audio_device_id, game->sfx[i].buffer, game->sfx[i].length);
      SDL_PauseAudioDevice(game->audio_device_id, 0);
      return;
    }
  }
}

#endif
