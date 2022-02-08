#define NUM_TEXTURES 4

void initialize_gfx(Game *game);

void draw_walls(Game *game);
void draw_snake(Game *game);
void draw_title_screen(Game *game);
void draw_paused(Game *game);
void draw_game_over(Game *game);
void draw_food(Game *game);
void draw_score(Game *game);

SDL_Texture* get_texture(Game *game, char* name);
char* path_for_image(char* name);

