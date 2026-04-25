/*

  Flappy Bird.ino

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include "SpaceTrash/game/bitmaps.h"

#define FB_DRAW_NONE 0
#define FB_DRAW_BBOX 1
#define FB_DRAW_PLAYER1 3

#define FB_MOVE_NONE 0
#define FB_MOVE_PLAYER 4
#define FB_MOVE_WALL 9

#define FB_IS_HIT_NONE 0
#define FB_IS_HIT_BBOX 1

#define FB_DESTROY_NONE 0
#define FB_DESTROY_DISAPPEAR 1
#define FB_DESTROY_PLAYER 4

#define FB_FIRE_NONE 0

#define FB_OT_EMPTY 0
#define FB_OT_WALL_SOLID 1
#define FB_OT_PLAYER 2

#define FB_FP 4
#define FB_POINTS_PER_LEVEL 25
#define FB_STATE_PREPARE 0
#define FB_STATE_IPREPARE 1
#define FB_STATE_GAME 2
#define FB_STATE_END 3
#define FB_STATE_IEND 4
#define FB_DIFF_VIS_LEN 30
#define FB_DIFF_FP 5

/* object types */
struct _fb_ot_struct {
  uint8_t missle_mask;
  uint8_t hit_mask;
  uint8_t points;
  uint8_t draw_fn;
  uint8_t move_fn;
  uint8_t destroy_fn;
  uint8_t is_hit_fn;
  uint8_t fire_fn;
};
typedef struct _fb_ot_struct fb_ot;

/* objects */
struct _fb_obj_struct {
  uint8_t ot;
  int8_t tmp;
  int16_t x, y;
  int8_t x0, y0, x1, y1;
  uint8_t scored;
};
typedef struct _fb_obj_struct fb_obj;

int16_t fb_player_vel = 0;
uint8_t fb_last_fire = 0;

// External u8g2 object from AJBoy.ino
extern U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2;

// External control pins from AJBoy.ino
extern const uint8_t c_up;
extern const uint8_t c_down;
extern const uint8_t c_button1;

/*================================================================*/
/* graphics object */
/*================================================================*/

u8g2_t *fb_u8g2;

u8g2_uint_t u8g_height_minus_one;


#define FB_AREA_HEIGHT (fb_u8g2->height - 8)
#define FB_AREA_WIDTH (fb_u8g2->width)


/*================================================================*/
/* object types */
/*================================================================*/


const fb_ot fb_object_types[] U8X8_PROGMEM = {
  /* 0: empty object type */
  { 0, 0, 0, FB_DRAW_NONE, FB_MOVE_NONE, FB_DESTROY_NONE, FB_IS_HIT_NONE, FB_FIRE_NONE },
  /* 1: wall, player will be destroyed */
  { 0, 2, 0, FB_DRAW_BBOX, FB_MOVE_WALL, FB_DESTROY_NONE, FB_IS_HIT_BBOX, FB_FIRE_NONE },
  /* 2: FB_OT_PLAYER */
  { 2, 0, 0, FB_DRAW_PLAYER1, FB_MOVE_PLAYER, FB_DESTROY_PLAYER, FB_IS_HIT_NONE, FB_FIRE_NONE }
};

/*================================================================*/
/* list of all objects on the screen */
/*================================================================*/

/* use AVR RAMEND constant to derive the number of allowed objects */

#if RAMEND < 0x300
#define FB_OBJ_CNT 25
#else
//#define FB_OBJ_CNT 45
#define FB_OBJ_CNT 60
#endif

fb_obj fb_objects[FB_OBJ_CNT];

/*================================================================*/
/* about players space ship*/
/*================================================================*/

/* player position */
uint8_t fb_player_pos;

/* points */
uint16_t fb_player_points;
uint16_t fb_player_points_delayed;
uint16_t fb_highscore = 0;


uint8_t fb_state = FB_STATE_PREPARE;

/*================================================================*/
/* game difficulty */
/*================================================================*/
uint8_t fb_difficulty = 1;
uint16_t fb_to_diff_cnt = 0;


/*================================================================*/
/* forward definitions */
/*================================================================*/
uint8_t fb_rnd(void) U8X8_NOINLINE;
static fb_obj *fb_GetObj(uint8_t objnr) U8X8_NOINLINE;
uint8_t fb_GetMissleMask(uint8_t objnr);
uint8_t fb_GetHitMask(uint8_t objnr);
int8_t fb_FindObj(uint8_t ot) U8X8_NOINLINE;
void fb_ClrObjs(void) U8X8_NOINLINE;
int8_t fb_NewObj(void) U8X8_NOINLINE;
uint8_t fb_CntObj(uint8_t ot);
uint8_t fb_CalcXY(fb_obj *o) U8X8_NOINLINE;
void fb_SetXY(fb_obj *o, uint8_t x, uint8_t y) U8X8_NOINLINE;

void fb_SetupPlayer(uint8_t objnr, uint8_t ot);
void fb_NewPipePair(void);


/*================================================================*/
/* utility functions */
/*================================================================*/

char fb_itoa_buf[12];
char *fb_itoa(unsigned long v) {
  volatile unsigned char i = 11;
  fb_itoa_buf[11] = '\0';
  while (i > 0) {
    i--;
    fb_itoa_buf[i] = (v % 10) + '0';
    v /= 10;
    if (v == 0)
      break;
  }
  return fb_itoa_buf + i;
}


uint8_t fb_rnd(void) {
  return rand();
}

/*	
  for the specified index number, return the object
*/
static fb_obj *fb_GetObj(uint8_t objnr) {
  return fb_objects + objnr;
}


/*
  check, if this is a missle-like object (that is, can this object destroy something else)
*/
uint8_t fb_GetMissleMask(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  return u8x8_pgm_read(&(fb_object_types[o->ot].missle_mask));
}

/*
  check, if this is a missle-like object (that is, can this object destroy something else)
*/
uint8_t fb_GetHitMask(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  return u8x8_pgm_read(&(fb_object_types[o->ot].hit_mask));
}

/*
  search an empty object
*/
int8_t fb_FindObj(uint8_t ot) {
  int8_t i;
  for (i = 0; i < FB_OBJ_CNT; i++) {
    if (fb_objects[i].ot == ot)
      return i;
  }
  return -1;
}

/*
  delete all objects
*/

void fb_ClrObjs(void) {
  int8_t i;
  for (i = 0; i < FB_OBJ_CNT; i++)
    fb_objects[i].ot = 0;
}

/*
  search an empty object
*/
int8_t fb_NewObj(void) {
  int8_t i;
  for (i = 0; i < FB_OBJ_CNT; i++) {
    if (fb_objects[i].ot == 0)
      return i;
  }
  return -1;
}

/*
  count number of objectes of the provided type
  fb_CntObj(0) will return the number of empty objects, that means if
  fb_CntObj(0) > 0 then fb_NewObj() will return  a valid index
*/
uint8_t fb_CntObj(uint8_t ot) {
  uint8_t i;
  uint8_t cnt = 0;
  for (i = 0; i < FB_OBJ_CNT; i++) {
    if (fb_objects[i].ot == ot)
      cnt++;
  }
  return cnt;
}

/*
  calculate the pixel coordinates of the reference point of an object
  return rhe x value
*/
uint8_t fb_px_x, fb_px_y; /* pixel within area */
uint8_t fb_CalcXY(fb_obj *o) {
  //fb_obj *o = fb_GetObj(objnr);
  fb_px_y = o->y >> FB_FP;
  fb_px_x = o->x >> FB_FP;
  return fb_px_x;
}

void fb_SetXY(fb_obj *o, uint8_t x, uint8_t y) {
  o->x = ((int16_t)x) << FB_FP;
  o->y = ((int16_t)y) << FB_FP;
}

/*
  calculate the object bounding box and place it into some global variables
*/
int16_t fb_bbox_x0, fb_bbox_y0, fb_bbox_x1, fb_bbox_y1;

void fb_CalcBBOX(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);

  fb_bbox_x0 = (uint16_t)(o->x >> FB_FP);
  fb_bbox_x1 = fb_bbox_x0;
  fb_bbox_x0 += o->x0;
  fb_bbox_x1 += o->x1;

  fb_bbox_y0 = (uint16_t)(o->y >> FB_FP);
  fb_bbox_y1 = fb_bbox_y0;
  fb_bbox_y0 += o->y0;
  fb_bbox_y1 += o->y1;
}

/*
  clip bbox with the view window. requires a call to fb_CalcBBOX
  return 0, if the bbox is totally outside the window
*/
uint8_t fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_x1, fb_cbbox_y1;
uint8_t fb_ClipBBOX(void) {
  if (fb_bbox_x0 >= FB_AREA_WIDTH)
    return 0;
  if (fb_bbox_x0 >= 0)
    fb_cbbox_x0 = (uint16_t)fb_bbox_x0;
  else
    fb_cbbox_x0 = 0;

  if (fb_bbox_x1 < 0)
    return 0;
  if (fb_bbox_x1 < FB_AREA_WIDTH)
    fb_cbbox_x1 = (uint16_t)fb_bbox_x1;
  else
    fb_cbbox_x1 = FB_AREA_WIDTH - 1;

  if (fb_bbox_y0 >= FB_AREA_HEIGHT)
    return 0;
  if (fb_bbox_y0 >= 0)
    fb_cbbox_y0 = (uint16_t)fb_bbox_y0;
  else
    fb_cbbox_y0 = 0;

  if (fb_bbox_y1 < 0)
    return 0;
  if (fb_bbox_y1 < FB_AREA_HEIGHT)
    fb_cbbox_y1 = (uint16_t)fb_bbox_y1;
  else
    fb_cbbox_y1 = FB_AREA_HEIGHT - 1;

  return 1;
}


/*================================================================*/
/* universal member functions */
/*================================================================*/


uint8_t fb_IsOut(uint8_t objnr) {
  fb_CalcBBOX(objnr);
  if (fb_bbox_x0 >= FB_AREA_WIDTH)
    return 1;
  if (fb_bbox_x1 < 0)
    return 1;
  if (fb_bbox_y0 >= FB_AREA_HEIGHT)
    return 1;
  if (fb_bbox_y1 < 0)
    return 1;
  return 0;
}

void fb_Disappear(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  fb_player_points += u8x8_pgm_read(&(fb_object_types[o->ot].points));
  o->ot = 0;
}

/*================================================================*/
/* type dependent member functions */
/*================================================================*/

void fb_Move(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  switch (u8x8_pgm_read(&(fb_object_types[o->ot].move_fn))) {
    case FB_MOVE_NONE:
      break;
    case FB_MOVE_PLAYER:
      fb_player_vel += 1;        // gravity
      o->y += fb_player_vel;

      if (o->y < 0) {
        o->y = 0;
        if (fb_player_vel < 0) fb_player_vel = 0;
      }

      if (o->y > ((FB_AREA_HEIGHT - 1) << FB_FP)) {
        o->y = (FB_AREA_HEIGHT - 1) << FB_FP;
        fb_Destroy(objnr);
      }
      break;
    case FB_MOVE_WALL:
      o->x -= 8;
      o->x -= (fb_difficulty >> 2);
      break;
  }
}

void fb_DrawBBOX(uint8_t objnr) {
  uint8_t y0, y1;
  /*fb_obj *o = fb_GetObj(objnr);*/
  fb_CalcBBOX(objnr);
  if (fb_ClipBBOX() == 0)
    return;
  /* fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_x1, fb_cbbox_y1; */


  // w = fb_cbbox_x1-fb_cbbox_x0;
  // w++;
  // h = fb_cbbox_y1-fb_cbbox_y0;
  // h++;


  //dog_SetVLine(fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_y1);
  //dog_SetVLine(fb_cbbox_x1, fb_cbbox_y0, fb_cbbox_y1);
  //dog_SetHLine(fb_cbbox_x0, fb_cbbox_x1, fb_cbbox_y0);
  //dog_SetHLine(fb_cbbox_x0, fb_cbbox_x1, fb_cbbox_y1);

  u8g2_SetDrawColor(fb_u8g2, 1);
  y0 = u8g_height_minus_one - fb_cbbox_y0;
  y1 = u8g_height_minus_one - fb_cbbox_y1;

  u8g2_DrawFrame(fb_u8g2, fb_cbbox_x0, y1, fb_cbbox_x1 - fb_cbbox_x0 + 1, y0 - y1 + 1);

  //dog_SetBox(fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_x1, fb_cbbox_y1);

  /*
  if ( o->ot == FB_OT_PLAYER )
  {
    dog_DrawStr(0, 26, font_4x6, fb_itoa(fb_cbbox_y0));
    dog_DrawStr(10, 26, font_4x6, fb_itoa(fb_cbbox_y1));
  }
  */
}

#ifdef FN_IS_NOT_IN_USE
void fb_DrawFilledBox(uint8_t objnr) {
  fb_CalcBBOX(objnr);
  if (fb_ClipBBOX() == 0)
    return;
  /* fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_x1, fb_cbbox_y1; */
  dog_SetBox(fb_cbbox_x0, fb_cbbox_y0, fb_cbbox_x1, fb_cbbox_y1);
}
#endif

void fb_DrawBitmap(uint8_t objnr, const uint8_t *bm, uint8_t w, uint8_t h) {
  /* fb_obj *o = fb_GetObj(objnr); */
  fb_CalcBBOX(objnr);
  /* result is here: int16_t fb_bbox_x0, fb_bbox_y0, fb_bbox_x1, fb_bbox_y1 */
  //dog_SetBitmapP(fb_bbox_x0,fb_bbox_y1,bm,w,h);

  u8g2_DrawBitmap(fb_u8g2, fb_bbox_x0, u8g_height_minus_one - fb_bbox_y1, (w + 7) / 8, h, bm);
}

void fb_DrawObj(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  switch (u8x8_pgm_read(&(fb_object_types[o->ot].draw_fn))) {
    case FB_DRAW_NONE:
      break;
    case FB_DRAW_BBOX:
      fb_DrawBBOX(objnr);
      break;
    case FB_DRAW_PLAYER1:
      fb_DrawBitmap(objnr, st_bitmap_player1, 7, 5);
      break;
  }
}

uint8_t fb_IsHitBBOX(uint8_t objnr, uint8_t x, uint8_t y) {
  fb_CalcBBOX(objnr);
  if (fb_ClipBBOX() == 0)
    return 0; /* obj is outside (not visible) */
  if (x < fb_cbbox_x0)
    return 0;
  if (x > fb_cbbox_x1)
    return 0;
  if (y < fb_cbbox_y0)
    return 0;
  if (y > fb_cbbox_y1)
    return 0;
  return 1;
}

void fb_Destroy(uint8_t objnr) {
  fb_obj *o = fb_GetObj(objnr);
  switch (u8x8_pgm_read(&(fb_object_types[o->ot].destroy_fn))) {
    case FB_DESTROY_NONE: /* only usefull for missels or walls which stay alife */
      break;
    case FB_DESTROY_DISAPPEAR: /* this should be the default operation */
      fb_Disappear(objnr);
      break;
    case FB_DESTROY_PLAYER:
      fb_Disappear(objnr);
      fb_state = FB_STATE_END;
      o->tmp = 0;
      break;
  }
}

/*
  check if the target (objnr) has been hit.
  fb_IsHit() must also destroy the target.
  return value:
    0: do not destroy the missle
    1: destroy the missle
*/
uint8_t fb_IsHit(uint8_t objnr, uint8_t x, uint8_t y, uint8_t missle_mask) {
  uint8_t hit_mask = fb_GetHitMask(objnr);
  fb_obj *o;

  if ((hit_mask & missle_mask) == 0)
    return 0;

  o = fb_GetObj(objnr);

  switch (u8x8_pgm_read(&(fb_object_types[o->ot].is_hit_fn))) {
    case FB_IS_HIT_NONE:
      break;
    case FB_IS_HIT_BBOX:
      if (fb_IsHitBBOX(objnr, x, y) != 0) {
        fb_Destroy(objnr);
        return 1;
      }
      break;
  }
  return 0;
}



void fb_NewPipePair(void) {
  fb_obj *o_top;
  fb_obj *o_bottom;
  int8_t objnr_top = fb_NewObj();
  if (objnr_top < 0) return;
  o_top = fb_GetObj(objnr_top);
  o_top->ot = FB_OT_WALL_SOLID;

  int8_t objnr_bottom = fb_NewObj();
  if (objnr_bottom < 0) {
     o_top->ot = 0; 
     return;
  }
  o_bottom = fb_GetObj(objnr_bottom);
  o_bottom->ot = FB_OT_WALL_SOLID;

  int8_t gap_size = 24; 
  int8_t gap_y = 15 + (fb_rnd() % (FB_AREA_HEIGHT - 30)); 

  o_top->x0 = 0;
  o_top->x1 = 5;
  o_top->x = (FB_AREA_WIDTH - 1) << FB_FP;
  o_top->y = 0 << FB_FP;
  o_top->y0 = 0;
  o_top->y1 = gap_y - (gap_size/2);
  o_top->scored = 0;

  o_bottom->x0 = 0;
  o_bottom->x1 = 5;
  o_bottom->x = (FB_AREA_WIDTH - 1) << FB_FP;
  o_bottom->y = (FB_AREA_HEIGHT - 1) << FB_FP;
  o_bottom->y0 = -((FB_AREA_HEIGHT - 1) - (gap_y + (gap_size/2)));
  o_bottom->y1 = 0;
  o_bottom->scored = 1; 
}

void fb_SetupPlayer(uint8_t objnr, uint8_t ot) {
  fb_obj *o = fb_GetObj(objnr);
  o->ot = ot;
  o->y0 = -2;
  o->y1 = 2;
}

void fb_NewPlayer(void) {
  fb_obj *o;
  int8_t objnr = fb_NewObj();
  if (objnr < 0)
    return;
  o = fb_GetObj(objnr);
  o->x = 6 << FB_FP;
  o->y = 5 << FB_FP;
  o->x0 = -6;
  o->x1 = 0;
  fb_SetupPlayer(objnr, FB_OT_PLAYER);
}

/*================================================================*/
/* trash creation */
/*================================================================*/

void fb_InitDeltaWall(void) {
  uint8_t i;
  uint8_t max_x = 0;
  uint8_t max_l;

  uint8_t min_dist_for_new = 55;
  uint8_t my_difficulty = fb_difficulty;

  if (my_difficulty > 20)
    my_difficulty = 20;
  min_dist_for_new -= (my_difficulty >> 1);

  max_l = FB_AREA_WIDTH;
  max_l -= min_dist_for_new;

  for (i = 0; i < FB_OBJ_CNT; i++) {
    if (fb_objects[i].ot == FB_OT_WALL_SOLID) {
      if (max_x < (fb_objects[i].x >> FB_FP))
        max_x = (fb_objects[i].x >> FB_FP);
    }
  }
  if (max_x < max_l) {
    fb_NewPipePair();
  }
}

void fb_InitDelta(void) {
  fb_InitDeltaWall();
}

/*================================================================*/
/* API: game draw procedure */
/*================================================================*/

void fb_DrawInGame(uint8_t fps) {
  uint8_t i;
  /* draw all objects */
  for (i = 0; i < FB_OBJ_CNT; i++)
    fb_DrawObj(i);

  //dog_ClrBox(0, FB_AREA_HEIGHT, fb_u8g2->width-1, FB_AREA_HEIGHT+3);

  u8g2_SetDrawColor(fb_u8g2, 0);
  u8g2_DrawBox(fb_u8g2, 0, u8g_height_minus_one - FB_AREA_HEIGHT - 3, fb_u8g2->width, 4);

  u8g2_SetDrawColor(fb_u8g2, 1);
  u8g2_DrawHLine(fb_u8g2, 0, u8g_height_minus_one - FB_AREA_HEIGHT + 1, FB_AREA_WIDTH);
  u8g2_DrawHLine(fb_u8g2, 0, u8g_height_minus_one, FB_AREA_WIDTH);
  u8g2_SetFont(fb_u8g2, u8g_font_4x6r);
  u8g2_DrawStr(fb_u8g2, 0, u8g_height_minus_one - FB_AREA_HEIGHT, fb_itoa(fb_difficulty));
  u8g2_DrawHLine(fb_u8g2, 10, u8g_height_minus_one - FB_AREA_HEIGHT - 3, (fb_to_diff_cnt >> FB_DIFF_FP) + 1);
  u8g2_DrawVLine(fb_u8g2, 10, u8g_height_minus_one - FB_AREA_HEIGHT - 4, 3);
  u8g2_DrawVLine(fb_u8g2, 10 + FB_DIFF_VIS_LEN, u8g_height_minus_one - FB_AREA_HEIGHT - 4, 3);


  /* player points */
  u8g2_DrawStr(fb_u8g2, FB_AREA_WIDTH - 5 * 4 - 2, u8g_height_minus_one - FB_AREA_HEIGHT, fb_itoa(fb_player_points_delayed));


  /* FPS output */
  if (fps > 0) {
    //i = dog_DrawStr(FB_AREA_WIDTH-5*4-2-7*4, FB_AREA_HEIGHT, font_4x6, "FPS:");
    i = u8g2_DrawStr(fb_u8g2, FB_AREA_WIDTH - 5 * 4 - 2 - 7 * 4, u8g_height_minus_one - FB_AREA_HEIGHT, "FPS:");

    //dog_DrawStr(FB_AREA_WIDTH-5*4-2-7*4+i, FB_AREA_HEIGHT, font_4x6, fb_itoa(fps));
    u8g2_DrawStr(fb_u8g2, FB_AREA_WIDTH - 5 * 4 - 2 - 7 * 4 + i, u8g_height_minus_one - FB_AREA_HEIGHT, fb_itoa(fps));
  }
  /*dog_DrawStr(60+i, FB_AREA_HEIGHT, font_4x6, fb_itoa(fb_CntObj(0)));*/
}

void fb_Draw(uint8_t fps) {
  switch (fb_state) {
    case FB_STATE_PREPARE:
    case FB_STATE_IPREPARE:
      //dog_DrawStr(0, (fb_u8g2->height-6)/2, font_4x6, "Flappy Bird");
      u8g2_SetFont(fb_u8g2, u8g_font_4x6r);
      u8g2_SetDrawColor(fb_u8g2, 1);
      //dog_DrawStrP(0, (fb_u8g2->height-6)/2, font_4x6, DOG_PSTR("Flappy Bird"));
      u8g2_DrawStr(fb_u8g2, 0, u8g_height_minus_one - (fb_u8g2->height - 6) / 2, "Flappy Bird");
      //dog_SetHLine(fb_u8g2->width-fb_to_diff_cnt-10, fb_u8g2->width-fb_to_diff_cnt, (fb_u8g2->height-6)/2-1);
      u8g2_DrawHLine(fb_u8g2, fb_u8g2->width - fb_to_diff_cnt - 10, u8g_height_minus_one - (fb_u8g2->height - 6) / 2 + 1, 11);
      break;
    case FB_STATE_GAME:
      fb_DrawInGame(fps);
      break;
    case FB_STATE_END:
    case FB_STATE_IEND:
      u8g2_SetFont(fb_u8g2, u8g_font_4x6r);
      u8g2_SetDrawColor(fb_u8g2, 1);
      //dog_DrawStr(0, (fb_u8g2->height-6)/2, font_4x6, "Game Over");
      //dog_DrawStrP(0, (fb_u8g2->height-6)/2, font_4x6, DOG_PSTR("Game Over"));
      u8g2_DrawStr(fb_u8g2, 0, u8g_height_minus_one - (fb_u8g2->height - 6) / 2, "Game Over");
      //dog_DrawStr(50, (fb_u8g2->height-6)/2, font_4x6, fb_itoa(fb_player_points));
      u8g2_DrawStr(fb_u8g2, 50, u8g_height_minus_one - (fb_u8g2->height - 6) / 2, fb_itoa(fb_player_points));
      //dog_DrawStr(75, (fb_u8g2->height-6)/2, font_4x6, fb_itoa(fb_highscore));
      u8g2_DrawStr(fb_u8g2, 75, u8g_height_minus_one - (fb_u8g2->height - 6) / 2, fb_itoa(fb_highscore));

      //dog_SetHLine(fb_to_diff_cnt, fb_to_diff_cnt+10, (fb_u8g2->height-6)/2-1);
      u8g2_DrawHLine(fb_u8g2, fb_to_diff_cnt, u8g_height_minus_one - (fb_u8g2->height - 6) / 2 + 1, 11);
      break;
  }
}

void fb_SetupInGame(void) {
  fb_player_points = 0;
  fb_player_points_delayed = 0;
  fb_difficulty = 1;
  fb_to_diff_cnt = 0;
  fb_ClrObjs();
  fb_NewPlayer();
  /* fb_InitBrick1(); */
}


/*================================================================*/
/* API: game setup */
/*================================================================*/

void fb_Setup(u8g2_t *u8g) {
  fb_u8g2 = u8g;
  u8g2_SetBitmapMode(u8g, 1);
  u8g_height_minus_one = u8g->height;
  u8g_height_minus_one--;
}

/*================================================================*/
/* API: game step execution */
/*================================================================*/

/*
  player_pos: 0..255
*/
void fb_StepInGame(uint8_t player_pos, uint8_t is_auto_fire, uint8_t is_fire) {
  uint8_t i, j;
  uint8_t missle_mask;

  /* rescale player pos */
  //fb_player_pos = ((uint16_t)player_pos * (uint16_t)FB_AREA_HEIGHT)/256;
  if (player_pos < 64)
    fb_player_pos = 0;
  else if (player_pos >= 192)
    fb_player_pos = FB_AREA_HEIGHT - 2 - 1;
  else
    fb_player_pos = ((uint16_t)((player_pos - 64)) * (uint16_t)(FB_AREA_HEIGHT - 2)) / 128;
  fb_player_pos += 1;
  /* move all objects */
  for (i = 0; i < FB_OBJ_CNT; i++)
    fb_Move(i);

  /* check for objects which left the play area */
  for (i = 0; i < FB_OBJ_CNT; i++)
    if (fb_objects[i].ot != 0)
      if (fb_IsOut(i) != 0)
        fb_Disappear(i);

  /* missle and destruction handling */
  for (i = 0; i < FB_OBJ_CNT; i++) {
    missle_mask = fb_GetMissleMask(i);
    if (missle_mask != 0)                                        /* should we apply missle handling? */
      if (fb_CalcXY(fb_objects + i) != 0)                        /* yes: calculate pixel reference point (fb_px_x, fb_px_y) */
        for (j = 0; j < FB_OBJ_CNT; j++)                         /* has any other object been hit? */
          if (i != j)                                            /* except missle itself... */
            if (fb_IsHit(j, fb_px_x, fb_px_y, missle_mask) != 0) /* let the member function decide */
            {                                                    /* let the member function destroy the object if required */
              fb_Destroy(i);
            }
  }

  for (i = 0; i < FB_OBJ_CNT; i++) {
    fb_obj *o = fb_GetObj(i);

    if (o->ot == FB_OT_WALL_SOLID && o->scored == 0) {
      uint8_t x = o->x >> FB_FP;

      if (x < 6) {
        fb_player_points++;
        o->scored = 1;
      }
    }
  }

  /* handle jump */
  if (is_fire != 0 && fb_last_fire == 0) {
    fb_player_vel = -16; // jump force
  }
  fb_last_fire = is_fire;

  /* fire */
  // for (i = 0; i < FB_OBJ_CNT; i++)
  //   fb_Fire(i);

  /* create new objects */
  fb_InitDelta();

  /* increase difficulty */

  fb_to_diff_cnt++;
  if (fb_to_diff_cnt == (FB_DIFF_VIS_LEN << FB_DIFF_FP)) {
    fb_to_diff_cnt = 0;
    fb_difficulty++;
    fb_player_points += FB_POINTS_PER_LEVEL;
  }

  /* update visible player points */
  if (fb_player_points_delayed < fb_player_points)
    fb_player_points_delayed++;
}

void fb_Step(uint8_t player_pos, uint8_t is_auto_fire, uint8_t is_fire) {
  switch (fb_state) {
    case FB_STATE_PREPARE:
      fb_to_diff_cnt = fb_u8g2->width - 10; /* reuse fb_to_diff_cnt */
      fb_state = FB_STATE_IPREPARE;
      break;
    case FB_STATE_IPREPARE:
      fb_to_diff_cnt--;
      if (fb_to_diff_cnt == 0) {
        fb_state = FB_STATE_GAME;
        fb_SetupInGame();
      }
      break;
    case FB_STATE_GAME:
      fb_StepInGame(player_pos, is_auto_fire, is_fire);
      break;
    case FB_STATE_END:
      fb_to_diff_cnt = fb_u8g2->width - 10; /* reuse fb_to_diff_cnt */
      if (fb_highscore < fb_player_points)
        fb_highscore = fb_player_points;
      fb_state = FB_STATE_IEND;
      break;
    case FB_STATE_IEND:
      fb_to_diff_cnt--;
      if (fb_to_diff_cnt == 0)
        fb_state = FB_STATE_PREPARE;
      break;
  }
}

uint8_t fb_y = 128;
#define FB_Y_MIN 70
#define FB_Y_MAX 185

void flappybird_start() {
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setFontDirection(0);
  u8g2.setFontRefHeightAll();

  fb_Setup(u8g2.getU8g2());
  for (;;) {
    fb_Step(0, /* is_auto_fire */ 0, /* is_fire */ digitalRead(c_button1) == LOW);
    u8g2.firstPage();
    do {
      fb_Draw(0);
    } while (u8g2.nextPage());

    if (digitalRead(c_up) == LOW && digitalRead(c_down) == LOW) {
      // Secret way to exit back to menu?
      return;
    }
    
    // Check if game is over and user wants to exit
    if (fb_state == FB_STATE_PREPARE && digitalRead(c_up) == LOW) {
        return;
    }

    delay(10);
  }
}
