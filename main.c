#include <stdio.h>
#include <SDL.h>

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

#define NUM_TEXTURES 3

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

void handle_collisions(void);

void spawn_food(void);
void draw_food(void);

void display_score(void);
void check_game_over(void);

void start_game(void);
void pause_game(void);
void unpause_game(void);
void to_title_screen(void);

SDL_Texture* get_texture(char* name);
char* path_for_image(char* name);

typedef enum dir {UP, DOWN, LEFT, RIGHT} dir;

typedef enum gamestate {TITLE_SCREEN, PLAYING, PAUSED, GAME_OVER} game_state;

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
        check_game_over();
        break;
      case PAUSED:
        draw_walls();
        draw_food();
        draw_snake();
        draw_paused();
        break;
      case GAME_OVER:
        draw_walls();
        draw_food();
        draw_snake();
        draw_game_over();
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

  game.textures = malloc(NUM_TEXTURES * sizeof(Tex));
  char* tex_names[] = {"title", "pause", "game_over"};

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
      printf("error: failed to create title screen texture: %s\n", SDL_GetError());
      terminate(EXIT_FAILURE);
    }
    SDL_FreeSurface(image);
    game.textures[i].name = tex_names[i];
    game.textures[i].texture = texture;
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

  if (game.snake[0].x > SCREEN_WIDTH - WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }

  if (game.snake[0].y < WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }
  
  if (game.snake[0].y > SCREEN_HEIGHT - WALL_THICKNESS) {
    game.game_over = 1;
    return;
  }
}

// TODO: Implement a move buffer for quick 180-degree turns
void change_direction(SDL_KeyCode new_direction) {
  int going_up = game.last_move == UP;
  int going_down = game.last_move == DOWN;
  int going_left = game.last_move == LEFT;
  int going_right = game.last_move == RIGHT;

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

void move_snake() {
  // TODO: remove?
  if (game.game_over) {
    return;
  }

  for (int i = SNAKE_SIZE-1; i >= 0; i--) {
    game.snake[i] = game.snake[i-1];
  }

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
    display_score();
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
    puts("Does this happen for real?");
  } // ?????????

  if (game.food.y < WALL_THICKNESS) {
    game.food.y = WALL_THICKNESS;
    puts("Does this happen for real?");
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

#define FOOD_COLOR 50, 200, 20, 255
void draw_food() {
  SDL_SetRenderDrawColor(game.renderer, FOOD_COLOR);
  SDL_RenderFillRect(game.renderer, &game.food);
}

void display_score() {
  char buffer[20];
  snprintf(buffer, 20, "Score: %d", game.score);
  SDL_SetWindowTitle(game.window, buffer);
}

void check_game_over() {
  if (game.game_over) {
    game.state = GAME_OVER;
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
            change_direction(e.key.keysym.sym);
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
