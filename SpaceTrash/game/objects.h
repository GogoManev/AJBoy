#pragma once
#include <Arduino.h>

/* object types */
struct _st_ot_struct {
  /* 
    missle and hit: 
      bit 0: player missle and trash
      bit 1: trash, which might hit the player
  */

  uint8_t missle_mask; /* this object is a missle: it might destroy something if the target is_hit_fn says so */
  uint8_t hit_mask;    /* if missle_mask & hit_mask is != 0  then the object can be destroyed */
  uint8_t points;
  uint8_t draw_fn;
  uint8_t move_fn;
  /* ST_MOVE_FN_NONE, ST_MOVE_FN_X_SLOW */
  uint8_t destroy_fn; /* object can be destroyed by a missle (e.g. a missle from the space ship) */
                      /* ST_DESTROY_FN_NONE, ST_DESTROY_FN_SPLIT */
  uint8_t is_hit_fn;  /* is hit procedure */
                      /* ST_IS_HIT_FN_NONE, ST_IS_HIT_BBOX */
  uint8_t fire_fn;
  /* ST_FIRE_FN_NONE, ST_FIRE_FN_X_LEFT */
};
typedef struct _st_ot_struct st_ot;

/*
  objects, which are visible at the play area
*/
struct _st_obj_struct {
  uint8_t ot; /* object type: zero means, object is not used */
  int8_t tmp; /* generic value, used by ST_MOVE_IMPLODE */
  /* absolute position */
  /* LCD pixel position is x>>ST_FP and y>>ST_FP */
  int16_t x, y;
  int8_t x0, y0, x1, y1; /* object outline in pixel, reference point is at 0,0 */
};
typedef struct _st_obj_struct st_obj;
