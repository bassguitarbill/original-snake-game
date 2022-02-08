#include <stdio.h>
#include <SDL.h>
#include <string.h>
#include <SDL2/SDL_ttf.h>

#include "game.h"


#include "input_buffer.c"
#include "gfx.c"
#include "sfx.c"

void initialize(void);
void handle_input(void);
void spawn_snake(int start_x, int start_y);
void move_snake(void);
void change_direction(SDL_KeyCode new_direction);
void add_control_input_to_buffer(SDL_KeyCode new_direction);

void handle_collisions(void);

void spawn_food(void);

void check_game_over(void);

void start_game(void);
void pause_game(void);
void unpause_game(void);
void to_title_screen(void);

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
        draw_walls(&game);
        draw_title_screen(&game);
        break;
      case PLAYING:
        move_snake();
        draw_walls(&game);
        draw_food(&game);
        draw_snake(&game);
        draw_score(&game);
        check_game_over();
        break;
      case PAUSED:
        draw_walls(&game);
        draw_food(&game);
        draw_snake(&game);
        draw_paused(&game);
        draw_score(&game);
        break;
      case GAME_OVER:
        draw_walls(&game);
        draw_food(&game);
        draw_snake(&game);
        draw_game_over(&game);
        draw_score(&game);
        break;
    }
    SDL_RenderPresent(game.renderer);

    SDL_Delay(100);
  }

  terminate(&game, EXIT_SUCCESS);
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
  spawn_snake(SNAKE_START_X, SNAKE_START_Y);
  game.state = TITLE_SCREEN;
  return;
}

void initialize() {
  game.input_buffer = initialize_input_buffer();

  initialize_gfx(&game);
  initialize_sfx(&game);
}

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
    play_sound(&game, "chomp.wav");
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

void check_game_over() {
  if (game.game_over) {
    game.state = GAME_OVER;
    play_sound(&game, "die.wav");
  }
}

void terminate(Game *game, int exit_code) {
  if (game->renderer) {
    SDL_DestroyRenderer(game->renderer);
  }
  if (game->window) {
    SDL_DestroyWindow(game->window);
  }
  for (int i = 0; i < NUM_TEXTURES; i ++) {
    SDL_DestroyTexture(game->textures[i].texture);
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
