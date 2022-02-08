typedef enum dir {UP, DOWN, LEFT, RIGHT} dir;

typedef enum gamestate {TITLE_SCREEN, PLAYING, PAUSED, GAME_OVER} game_state;

#define BUFFER_CAPACITY 20
typedef struct input_buffer {
  SDL_Keycode data[BUFFER_CAPACITY];
  int count;
} input_buffer;

typedef struct wav_buffer {
  Uint32 length;
  Uint8 *buffer;
  char* name;
} wav_buffer;

typedef struct {
  char* name;
  SDL_Texture *texture;
} Tex;

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  int running;
  SDL_Rect *snake;
  int dx;
  int dy;
  dir last_move;
  int game_over;
  SDL_Rect food;
  int score;
  game_state state;
  Tex *textures;
  int score_dirty;
  TTF_Font *font;
  SDL_Texture *score_texture;
  SDL_Rect score_size;
  input_buffer *input_buffer;
  wav_buffer *sfx;
  int audio_device_id;
} Game;
