#define BUFFER_CAPACITY 20

typedef struct input_buffer {
  SDL_Keycode data[BUFFER_CAPACITY];
  int count;
} input_buffer;

input_buffer* initialize_input_buffer(void);
void clear_input_buffer(input_buffer* buffer);
void push_to_input_buffer(input_buffer* buffer, SDL_Keycode key);
SDL_Keycode pop_from_input_buffer(input_buffer* buffer);

