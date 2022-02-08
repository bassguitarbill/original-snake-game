#include "game.h"
#include "gfx.h"

void initialize_gfx(Game *game) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("error: failed to initialize SDL: %s\n", SDL_GetError());
    terminate(game, EXIT_FAILURE);
  }

  game->window = SDL_CreateWindow(
      "Score: 0",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
      SDL_WINDOW_SHOWN
  );

  if (!game->window) {
    printf("error: failed to open a %d by %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
    terminate(game, EXIT_FAILURE);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);

  if (!game->renderer) {
    printf("error: failed to create renderer: %s\n", SDL_GetError());
    terminate(game, EXIT_FAILURE);
  }

  if (TTF_Init() < 0) {
    printf("error: failed to initialize TTF: %s\n", SDL_GetError());
    terminate(game, EXIT_FAILURE);
  }

  game->font = TTF_OpenFont("font.ttf", 18);
  if (!game->font) {
    printf("error: failed to load font: %s\n", SDL_GetError());
    terminate(game, EXIT_FAILURE);
  }

  game->textures = malloc(NUM_TEXTURES * sizeof(Tex));
  char* tex_names[] = {"title", "pause", "game_over", "hamburger"};

  SDL_Surface *image;
  SDL_Texture *texture;

  for (int i = 0; i < NUM_TEXTURES; i ++) {
    image = SDL_LoadBMP(path_for_image(tex_names[i]));
    if (!image) {
      printf("error: failed to load image '%s': %s\n", tex_names[i], SDL_GetError());
      terminate(game, EXIT_FAILURE);
    }
    texture = SDL_CreateTextureFromSurface(game->renderer, image);
    if (!texture) {
      printf("error: failed to create texture: %s\n", SDL_GetError());
      terminate(game, EXIT_FAILURE);
    }
    SDL_FreeSurface(image);
    game->textures[i].name = tex_names[i];
    game->textures[i].texture = texture;
  }
}

void draw_walls(Game *game) {
  SDL_SetRenderDrawColor(game->renderer, WALL_COLOR);

  SDL_Rect block = {
    .x = 0,
    .y = 0,
    .w = WALL_THICKNESS,
    .h = SCREEN_HEIGHT
  };

  SDL_RenderFillRect(game->renderer, &block);

  block.x = SCREEN_WIDTH - WALL_THICKNESS;
  SDL_RenderFillRect(game->renderer, &block);

  block.x = 0;
  block.w = SCREEN_WIDTH;
  block.h = WALL_THICKNESS;
  SDL_RenderFillRect(game->renderer, &block);

  block.y = SCREEN_HEIGHT - WALL_THICKNESS;
  SDL_RenderFillRect(game->renderer, &block);
}

#define SNAKE_COLOR 200, 188, 178, 255
#define DEAD_SNAKE_COLOR 210, 40, 30, 255
void draw_snake(Game *game) {
  SDL_SetRenderDrawColor(game->renderer, game->game_over ? DEAD_SNAKE_COLOR : SNAKE_COLOR);
  SDL_RenderFillRect(game->renderer, &game->snake[0]);

  for (int i = 1; i < SNAKE_SIZE; i++) {
    if (game->snake[i].w == 0) {
      break;
    }

    if (game->game_over) {
      SDL_SetRenderDrawColor(game->renderer, DEAD_SNAKE_COLOR);
    } else {
      SDL_SetRenderDrawColor(game->renderer, SNAKE_COLOR);
    }

    SDL_RenderFillRect(game->renderer, &game->snake[i]);

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(game->renderer, &game->snake[i]);
  }
}

void draw_food(Game *game) {
  SDL_RenderCopy(game->renderer, get_texture(game, "hamburger"), NULL, &game->food);
}

void draw_title_screen(Game *game) {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game->renderer, get_texture(game, "title"), NULL, &rect);
}

void draw_paused(Game *game) {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game->renderer, get_texture(game, "pause"), NULL, &rect);
}

void draw_game_over(Game *game) {
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = SCREEN_WIDTH;
  rect.h = SCREEN_HEIGHT;

  SDL_RenderCopy(game->renderer, get_texture(game, "game_over"), NULL, &rect);
}

void draw_score(Game *game) {
  if (game->score_dirty) {
    SDL_Surface* score;
    SDL_Color color = { 0, 0, 0 };

    char* score_string = malloc(30);
    snprintf(score_string, 30, "Score: %d", game->score);
    score = TTF_RenderText_Solid(game->font, score_string, color);
    if (!score) {
      printf("error: failed to create score texture: %s\n", SDL_GetError());
    }

    SDL_Rect dest = { 0, 0, score->w, score->h };
    game->score_size = dest;
    SDL_DestroyTexture(game->score_texture);
    game->score_texture = SDL_CreateTextureFromSurface(game->renderer, score);
    SDL_FreeSurface(score);
  }

  SDL_RenderCopy(game->renderer, game->score_texture, NULL, &(game->score_size));
  game->score_dirty = 0;
}
  
SDL_Texture* get_texture(Game *game, char* name) {
  for (int i = 0; i < NUM_TEXTURES; i++) {
    if (strcmp(game->textures[i].name, name) == 0) {
      return game->textures[i].texture;
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

