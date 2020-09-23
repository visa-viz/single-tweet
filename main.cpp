#include "webgl.h"
#include <math.h>
#include <memory.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#define WIDTH 1024
#define HEIGHT 768

const char *line1 = "Request for Netflix show: ethnic aunties travelling and cooking";
const char *line2 = "together. An Indian aunty goes to Mexico and meets the local aunties";
const char *line3 = "qwertyuiopasdfghjklzxcvbnmqwertyuiopasdfghjklzxcvbnmqwertyuiop";
const char *line4 = "q w e r t y u i o p a s d f g h j k l z x c v b n m q w e r t y u i o p a s d f";
const char *lines[] = { line1, line2, line3, line4 };  

// Per-frame animation tick.
EM_BOOL draw_frame(double t, void *)
{
  clear_screen(1, 1, 1, 1);

  // Outline

  // Avatar
  fill_image((int)(30 * 1.75), (int)(HEIGHT - (25 + 49) * 1.75), 49.0 / 205 * 1.75, 1.0, 1.0, 1.0, 1.0, "avatar.png");

  // Header
  fill_text((int)(89 * 1.75), (int)(HEIGHT - (25 + 15 * 1.3125) * 1.75), 20.0 / 255, 23.0 / 255, 26.0 / 255, 1.0, "visa is cleaning out his notes", 15 * 1.75, 1);
  fill_text((int)(295 * 1.75), (int)(HEIGHT - (25 + 15 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, "@visakanv", 15 * 1.75, 0);
  fill_char((int)(370 * 1.75), (int)(HEIGHT - (25 + 15 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, 0xB7, 15 * 1.75, 0);
  fill_text((int)(379 * 1.75), (int)(HEIGHT - (25 + 15 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, "Jun 12, 2019", 15 * 1.75, 0);

  // Body
  for (int i = 0; i < 4; i++)
  {
    fill_text((int)(89 * 1.75), (int)(HEIGHT - (47 + (1 + i) * 15 * 1.3125) * 1.75), 20.0 / 255, 23.0 / 255, 26.0 / 255, 1.0, lines[i], 15 * 1.75, 0);
  }

  // Counters
  fill_image((int)(89 * 1.75), (int)(HEIGHT - (135 + 18.75) * 1.75), 18.75 / 96 * 1.75, 1.0, 1.0, 1.0, 1.0, "reply.png");
  fill_text((int)(118 * 1.75), (int)(HEIGHT - (136 + 13 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, "121", 13 * 1.75, 0);
  fill_image((int)(220 * 1.75), (int)(HEIGHT - (135 + 18.75) * 1.75), 18.75 / 96 * 1.75, 1.0, 1.0, 1.0, 1.0, "retweet.png");
  fill_text((int)(249 * 1.75), (int)(HEIGHT - (136 + 13 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, "3.6K", 13 * 1.75, 0);
  fill_image((int)(354 * 1.75), (int)(HEIGHT - (135 + 18.75) * 1.75), 18.75 / 96 * 1.75, 1.0, 1.0, 1.0, 1.0, "fav.png");
  fill_text((int)(383 * 1.75), (int)(HEIGHT - (136 + 13 * 1.3125) * 1.75), 101.0 / 255, 119.0 / 255, 134.0 / 255, 1.0, "13.4K", 13 * 1.75, 0);

  return EM_TRUE;
}

int main()
{
  init_webgl(WIDTH, HEIGHT);
  emscripten_request_animation_frame_loop(&draw_frame, 0);
}
