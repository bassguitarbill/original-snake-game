#include <stdlib.h>
#include "input_buffer.h"

input_buffer* initialize_input_buffer() {
  return calloc(1, sizeof(input_buffer));
}

void clear_input_buffer(input_buffer* buffer) {
  for (int i = 0; i < buffer->count; i++) {
    buffer->data[i] = 0;
  }
  buffer->count = 0;
}

void push_to_input_buffer(input_buffer* buffer, SDL_Keycode key) {
  if (buffer->count >= BUFFER_CAPACITY) {
    printf("Input buffer over capacity!");
    return;
  }

  buffer->data[buffer->count] = key;
  buffer->count += 1;
}

SDL_Keycode pop_from_input_buffer(input_buffer* buffer) {
  if (buffer->count <= 0) {
    return 0;
  }

  SDL_Keycode key = buffer->data[0];
  for (int i = 1; i < buffer->count; i++) {
    buffer->data[i - 1] = buffer->data[i];
  }

  buffer->count -= 1;
  return key;
}







