#include <stdio.h>
#include <SDL.h>
#include <string.h>
#include <SDL2/SDL_ttf.h>

#include "input_buffer.c"

#define SCREEN_WIDTH 680
#define SCREEN_HEIGHT 400

#define WALL_THICKNESS 20

#define CELL_WIDTH 20
#define CELL_HEIGHT 20
#define CELL_COUNT ((SCREEN_WIDTH - WALL_THICKNESS * 2) * \
                    (SCREEN_HEIGHT - WALL_THICKNESS * 2))/\
                    (CELL_WIDTH * CELL_HEIGHT)
#define SNAKE_START_X 200
#define SNAKE_START_Y 200

#define NUM_TEXTURES 4
#define NUM_SFX 2

void initialize(void);
void terminate(int exit_code);
void handle_input(void);
void draw_walls(void);
void draw_snake(void);
void draw_title_screen(void);
void draw_paused(void);
void draw_game_over(void);
void spawn_snake(int start_x, int start_y);
void move_snake(void);
void change_direction(SDL_KeyCode new_direction);
void add_control_input_to_buffer(SDL_KeyCode new_direction);

void handle_collisions(void);

void spawn_food(void);
void draw_food(void);

void display_score(void);
void draw_score(void);
void check_game_over(void);

void start_game(void);
void pause_game(void);
void unpause_game(void);
void to_title_screen(void);

SDL_Texture* get_texture(char* name);
char* path_for_image(char* name);

typedef enum dir {UP, DOWN, LEFT, RIGHT} dir;

typedef enum gamestate {TITLE_SCREEN, PLAYING, PAUSED, GAME_OVER} game_state;

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

void initialize_game_object(void);

Game game;

void initialize_game_object() {
  if (game.snake) {
    game.snake = realloc(game.snake, sizeof(SDL_Rect) * CELL_COUNT);
  } else {
    game.snake = malloc(sizeof(SDL_Rect) * CELL_COUNT);
  }
  game.running = 1;
  game.dx = CELL_WIDTH;
  game.dy = 0;
  game.game_over = 0;
  game.food.w = CELL_WIDTH;
  game.food.h = CELL_HEIGHT;
  game.last_move = RIGHT;
  game.score = 0;
  game.state = TITLE_SCREEN;
  game.score_dirty = 1;
}

int main() {
  initialize_game_object();
  initialize();

  spawn_snake(SNAKE_START_X, SNAKE_START_Y);
  spawn_food();
  
  while (game.running) {
    SDL_SetRenderDrawColor(game.renderer, 30, 50, 60, 255);
    SDL_RenderClear(game.renderer);

    handle_input();

    switch (game.state) {
      case TITLE_SCREEN:
        draw_walls();
        draw_title_screen();
        break;
      case PLAYING:
        move_snake();
        draw_walls();
        draw_food();
        draw_snake();
        draw_score();
        check_game_over();
        break;
      case PAUSED:
        draw_walls();
        draw_food();
        draw_snake();
        draw_paused();
        draw_score();
        break;
      case GAME_OVER:
        draw_walls();
        draw_food();
        draw_snake();
        draw_game_over();
        draw_score();
        break;
    }
    SDL_RenderPresent(game.renderer);

    SDL_Delay(100);
  }

  terminate(EXIT_SUCCESS);
}

void start_game() {
  game.state = PLAYING;
  return;
}

void pause_game() {
  game.state = PAUSED;
  return;
}

void unpause_game() {
  game.state = PLAYING;
  return;
}

void to_title_screen() {
  initialize_game_object();
  display_score();
  spawn_snake(SNAKE_START_X, SNAKE_START_Y);
  game.state = TITLE_SCREEN;
  return;
}

void initialize() {
  game.input_buffer = initialize_input_buffer();

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("error: failed to initialize SDL: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  game.window = SDL_CreateWindow(
      "Score: 0",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN
  );

  if (!game.window) {
    printf("error: failed to open a %d by %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);

  if (!game.renderer) {
    printf("error: failed to create renderer: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  if (TTF_Init() < 0) {
    printf("error: failed to initialize TTF: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  game.font = TTF_OpenFont("font.ttf", 18);
  if (!game.font) {
    printf("error: failed to load font: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  game.textures = malloc(NUM_TEXTURES * sizeof(Tex));
  char* tex_names[] = {"title", "pause", "game_over", "hamburger"};

  SDL_Surface *image;
  SDL_Texture *texture;

  for (int i = 0; i < NUM_TEXTURES; i ++) {
    image = SDL_LoadBMP(path_for_image(tex_names[i]));
    if (!image) {
      printf("error: failed to load image '%s': %s\n", tex_names[i], SDL_GetError());
      terminate(EXIT_FAILURE);
    }
    texture = SDL_CreateTextureFromSurface(game.renderer, image);
    if (!texture) {
      printf("error: failed to create texture: %s\n", SDL_GetError());
      terminate(EXIT_FAILURE);
    }
    SDL_FreeSurface(image);
    game.textures[i].name = tex_names[i];
    game.textures[i].texture = texture;
  }

    
  SDL_Init(SDL_INIT_AUDIO);

  game.sfx = malloc(NUM_SFX * sizeof(wav_buffer));
  char* sfx_names[] = { "chomp.wav", "die.wav" };
  SDL_AudioSpec wav_spec;
  for (int i = 0; i < NUM_SFX; i++) {
    SDL_LoadWAV(sfx_names[i], &wav_spec, &game.sfx[i].buffer, &game.sfx[i].length);
    game.sfx[i].name = sfx_names[i];
  }

  game.audio_device_id = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
}

void play_sound(char* file) {
  for (int i = 0; i < NUM_SFX; i++) {
    if (strcmp(file, game.sfx[i].name) == 0) {
      SDL_QueueAudio(game.audio_device_id, game.sfx[i].buffer, game.sfx[i].length);
      SDL_PauseAudioDevice(game.audio_device_id, 0);
      return;
    }
  }
}


SDL_Texture* get_texture(char* name) {
  for (int i = 0; i < NUM_TEXTURES; i++) {
    if (strcmp(game.textures[i].name, name) == 0) {
      return game.textures[i].texture;
    }
  }
  return NULL;
}

char* path_for_image(char* name) {
  char* path = calloc(50, sizeof(char));
  strcat(path, "./");
  strcat(path, name);
  strcat(path, ".bmp");
  return path;
}

void draw_title_screen() {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game.renderer, get_texture("title"), NULL, &rect);
}

void draw_paused() {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game.renderer, get_texture("pause"), NULL, &rect);
}

void draw_game_over() {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game.renderer, get_texture("game_over"), NULL, &rect);
}

void draw_score() {
  if (game.score_dirty) {
    SDL_Surface* score;
    SDL_Color color = { 0, 0, 0 };

    char* score_string = malloc(30);
    snprintf(score_string, 30, "Score: %d", game.score);
    score = TTF_RenderText_Solid(game.font, score_string, color);
    if (!score) {
      printf("error: failed to create score texture: %s\n", SDL_GetError());
      // terminate(EXIT_FAILURE);
    }

    SDL_Rect dest = { 0, 0, score->w, score->h };
    game.score_size = dest;
    SDL_DestroyTexture(game.score_texture);
    game.score_texture = SDL_CreateTextureFromSurface(game.renderer, score);
    SDL_FreeSurface(score);
  }

  SDL_RenderCopy(game.renderer, game.score_texture, NULL, &(game.score_size));
  game.score_dirty = 0;
}
  

#define WALL_COLOR 110, 150, 170, 255
void draw_walls() {
  SDL_SetRenderDrawColor(game.renderer, WALL_COLOR);

  SDL_Rect block = {
    .x = 0,
    .y = 0,
    .w = WALL_THICKNESS,
    .h = SCREEN_HEIGHT
  };

  SDL_RenderFillRect(game.renderer, &block);

  block.x = SCREEN_WIDTH - WALL_THICKNESS;
  SDL_RenderFillRect(game.renderer, &block);

  block.x = 0;
  block.w = SCREEN_WIDTH;
  block.h = WALL_THICKNESS;
  SDL_RenderFillRect(game.renderer, &block);

  block.y = SCREEN_HEIGHT - WALL_THICKNESS;
  SDL_RenderFillRect(game.renderer, &block);
}

//#define SNAKE_SIZE sizeof(game.snake)/sizeof(game.snake[0])
#define SNAKE_SIZE CELL_COUNT

void spawn_snake(int start_x, int start_y) {
  for (int i = 0; i < SNAKE_SIZE; i++) {
    game.snake[i].x = 0;
    game.snake[i].y = 0;
    game.snake[i].w = 0;
    game.snake[i].h = 0;
  }

  game.snake[0].x = start_x;
  game.snake[0].y = start_y;
  game.snake[0].w = CELL_WIDTH;
  game.snake[0].h = CELL_HEIGHT;

  for (int i = 0; i < 5; i ++) {
    game.snake[i] = game.snake[0];
    game.snake[i].x = game.snake[0].x - (CELL_WIDTH * i);
  }
}

void handle_collisions() {
  for (int i = 1; i < SNAKE_SIZE; i++) {
    if (game.snake[i].w == 0) {
      break;
    }

    if (game.snake[0].x == game.snake[i].x && game.snake[0].y == game.snake[i].y) {
      game.game_over = 1;
      return;
    }
  }

  if (game.snake[0].x < WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }

  if (game.snake[0].x >= SCREEN_WIDTH - WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }

  if (game.snake[0].y < WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }
  
  if (game.snake[0].y >= SCREEN_HEIGHT - WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }
}

void add_control_input_to_buffer(SDL_KeyCode new_direction) {
  push_to_input_buffer(game.input_buffer, new_direction);
}

void change_direction(SDL_KeyCode new_direction) {
  int going_up, going_down, going_left, going_right;
  if (game.input_buffer->count > 0) {
    SDL_KeyCode last_move = game.input_buffer->data[game.input_buffer->count - 1];
    going_up = last_move == SDLK_UP;
    going_down = last_move == SDLK_DOWN;
    going_left = last_move == SDLK_LEFT;
    going_right = last_move == SDLK_RIGHT;
  } else {
    going_up = game.last_move == UP;
    going_down = game.last_move == DOWN;
    going_left = game.last_move == LEFT;
    going_right = game.last_move == RIGHT;
  }

  if (new_direction == SDLK_UP && !going_down) {
    game.dx = 0;
    game.dy = -CELL_HEIGHT;
  }

  if (new_direction == SDLK_DOWN && !going_up) {
    game.dx = 0;
    game.dy = CELL_HEIGHT;
  }

  if (new_direction == SDLK_LEFT && !going_right) {
    game.dx = -CELL_WIDTH;
    game.dy = 0;
  }

  if (new_direction == SDLK_RIGHT && !going_left) {
    game.dx = CELL_WIDTH;
    game.dy = 0;
  }
}

void pop_movement() {
  if (game.input_buffer->count == 0) return;
  change_direction(pop_from_input_buffer(game.input_buffer));
}


void move_snake() {
  for (int i = SNAKE_SIZE-1; i >= 0; i--) {
    game.snake[i] = game.snake[i-1];
  }

  pop_movement();

  if      (game.dx > 0) game.last_move = RIGHT;
  else if (game.dx < 0) game.last_move = LEFT;
  else if (game.dy < 0) game.last_move = UP;
  else if (game.dy > 0) game.last_move = DOWN;

  game.snake[0].x = game.snake[1].x + game.dx;
  game.snake[0].y = game.snake[1].y + game.dy;
  game.snake[0].w = CELL_WIDTH;
  game.snake[0].h = CELL_HEIGHT;

  if (game.snake[0].x == game.food.x && game.snake[0].y == game.food.y) {
    spawn_food();
    game.score += 1;
    game.score_dirty = 1;
    play_sound("chomp.wav");
  } else {
    for (int i = 5; i < SNAKE_SIZE; i++) {
      if (game.snake[i].w == 0) {
        game.snake[i-1].x = 0;
        game.snake[i-1].y = 0;
        game.snake[i-1].w = 0;
        game.snake[i-1].h = 0;
        break;
      }
    }
  }
  handle_collisions();
}

#define SNAKE_COLOR 200, 188, 178, 255
#define DEAD_SNAKE_COLOR 210, 40, 30, 255
void draw_snake(void) {
  SDL_SetRenderDrawColor(game.renderer, game.game_over ? DEAD_SNAKE_COLOR : SNAKE_COLOR);
  SDL_RenderFillRect(game.renderer, &game.snake[0]);

  for (int i = 1; i < SNAKE_SIZE; i++) {
    if (game.snake[i].w == 0) {
      break;
    }

    if (game.game_over) {
      SDL_SetRenderDrawColor(game.renderer, DEAD_SNAKE_COLOR);
    } else {
      SDL_SetRenderDrawColor(game.renderer, SNAKE_COLOR);
    }

    SDL_RenderFillRect(game.renderer, &game.snake[i]);

    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(game.renderer, &game.snake[i]);
  }
}

void spawn_food() {
  game.food.x = (rand() % (((SCREEN_WIDTH - CELL_WIDTH - WALL_THICKNESS) / CELL_WIDTH)+1)*CELL_WIDTH);
  game.food.y = (rand() % (((SCREEN_HEIGHT - CELL_HEIGHT - WALL_THICKNESS) / CELL_HEIGHT)+1)*CELL_HEIGHT);


  // TODO: Figure out how to get rid of these checks
  if (game.food.x < WALL_THICKNESS) {
    game.food.x = WALL_THICKNESS;
    //puts("Does this happen for real?");
  } 

  if (game.food.y < WALL_THICKNESS) {
    game.food.y = WALL_THICKNESS;
    //puts("Does this happen for real?");
  }

  // printf("%d vs %d, %d vs %d\n", game.snake[4].x, game.food.x, game.snake[4].y, game.food.y);
  for (int i = 0; i < SNAKE_SIZE; i++) {
    if (game.snake[i].w == 0) {
      break;
    }

    if (game.snake[i].x == game.food.x && game.snake[i].y == game.food.y) {
      spawn_food();
      break;
    }
  }
}

void draw_food() {
  SDL_RenderCopy(game.renderer, get_texture("hamburger"), NULL, &game.food);
}

void display_score() {
  char buffer[20];
  snprintf(buffer, 20, "Score: %d", game.score);
  SDL_SetWindowTitle(game.window, buffer);
}

void check_game_over() {
  if (game.game_over) {
    game.state = GAME_OVER;
    play_sound("die.wav");
  }
}

void terminate(int exit_code) {
  if (game.renderer) {
    SDL_DestroyRenderer(game.renderer);
  }
  if (game.window) {
    SDL_DestroyWindow(game.window);
  }
  for (int i = 0; i < NUM_TEXTURES; i ++) {
    SDL_DestroyTexture(game.textures[i].texture);
  }
  SDL_Quit();
  exit(exit_code);
}

void handle_input() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
      game.running = 0;
      return;
    }

    switch (game.state) {
      case TITLE_SCREEN:
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
          start_game();
        }
        break;
      case PLAYING:
        if (e.type == SDL_KEYDOWN) {
          if (e.key.keysym.sym == SDLK_UP
           || e.key.keysym.sym == SDLK_DOWN
           || e.key.keysym.sym == SDLK_LEFT
           || e.key.keysym.sym == SDLK_RIGHT) {
            add_control_input_to_buffer(e.key.keysym.sym);
            //change_direction(e.key.keysym.sym);
          } else if (e.key.keysym.sym == SDLK_p) {
            pause_game();
          }
        }
        break;
      case PAUSED:
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p) {
          unpause_game();
        }
        break;
      case GAME_OVER:
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
          to_title_screen();
        }
        break;
    }
  }
}
