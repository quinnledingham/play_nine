/*
  -2 -1 0 1, 0
  -2 -1 0 1, 1
*/

void init_hand_coords() {
  ASSERT(HAND_SIZE == 8);

  //float32 card_width = 2.0f;
  //float32 card_height = 3.2f;
  float32 padding = 0.2f;
  hand_width = card_dim.width * 4.0f + padding * 3.0f;

  u32 card_index = 0;
  for (s32 y = 1; y >= 0; y--) {
      for (s32 x = -2; x <= 1; x++) {
          hand_coords[card_index++] = { 
              float32(x) * card_dim.width  + (float32(x) * padding) + (card_dim.width / 2.0f)  + (padding / 2.0f), 
              float32(y) * card_dim.height + (float32(y) * padding) - (card_dim.height / 2.0f) - (padding / 2.0f)
          };
      }
  }
}

Vector4 card_color = {255, 0, 0, 1};

Vector2 get_draw_coords(Vector2 in) {
  return (in * card_scale) + Vector2{300, 300};
}

void draw_card_hand_2D(Player_Card *cards) {
  for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
    sdl_draw_rect(get_draw_coords(hand_coords[card_index]), card_scale, card_color);
  }

}

s32 draw_game_2D(Game *game) {
  init_hand_coords();

  SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
  SDL_RenderClear(sdl_renderer);

  draw_card_hand_2D(game->players[0].cards);
  //sdl_draw_rect({200, 200}, {300, 300}, {255, 0, 0, 1});

  SDL_RenderPresent(sdl_renderer);
  return SUCCESS;
}