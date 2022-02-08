#ifndef GAME_H
#define GAME_H

#define SCREEN_WIDTH 680
#define SCREEN_HEIGHT 400

#define WALL_THICKNESS 20

#define CELL_WIDTH 20
#define CELL_HEIGHT 20

#define WALL_COLOR 110, 150, 170, 255

#define CELL_COUNT ((SCREEN_WIDTH - WALL_THICKNESS * 2) * \
(SCREEN_HEIGHT - WALL_THICKNESS * 2))/\
(CELL_WIDTH * CELL_HEIGHT)

#define SNAKE_SIZE CELL_COUNT

#define SNAKE_START_X 200
#define SNAKE_START_Y 200

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

void terminate(Game *game, int exit_code);

#endif
