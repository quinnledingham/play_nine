/*
internal String_Draw_Info
get_string_draw_info(u32 id, const char *string, s32 length, float32 pixel_height) {
  Font *font = find_font(id);
  return get_string_draw_info(font, string, length, pixel_height);
}
*/

internal void
draw_text(const char *text, Vector2 coords, float32 pixel_height, Color_RGBA color) {
  Font *font = find_font(draw_ctx.font_id);

  //SDL_SetRenderDrawColor(sdl_ctx.renderer, color.r, color.g, color.b, color.a);

  TTF_Text *ttf_text = TTF_CreateText(sdl_ctx.text_engine, font->ttf_font, text, 0);

  #if DEBUG

  if (!ttf_text) {
    app_log("draw_text(): failed to create text (for %s)\n", text);
    return;
  }

  #endif // DEBUG

  TTF_SetTextColor(ttf_text, color.r, color.g, color.b, color.a);
  bool result = TTF_DrawRendererText(ttf_text, coords.x, coords.y);
  if (!result) {
    sdl_log("draw_text(), TTF_DrawRendererText failed (%s)\n", SDL_GetError());
  }

  if (ttf_text) {
    TTF_DestroyText(ttf_text);
  }
}
