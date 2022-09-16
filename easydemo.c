/*
 * Copyright (c) 2021  Wu, Xingbo <wuxb45@gmail.com>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include "lib.h"
#include "kv.h"
#include "wh.h"
#include "hash.h"

  int
main(int argc, char ** argv)
{
  (void)argc;
  (void)argv;
  struct wormhole * const wh = wh_create();
  struct wormref * const ref = wh_ref(wh);

  // I added-
  int hash_size = 1000;
  int num_locks = 500;
  HashTable * const hash = hash_init(hash_size, num_locks);
  bool r;

  time_t currTime = time(NULL);

  r = wh_probe(ref, "7", 1, hash); //fail search '7', 7 is inserted to the hash with currTime and F
  printf("wh_probe 7 %c\n", r?'T':'F');

  sleep(2);
  
  currTime = time(NULL);
  r = wh_probe(ref, "7", 1, hash); //fail search '7', 7 is inserted to the hash again but now with currTime + 2 and F
  printf("wh_probe 7 %c\n", r?'T':'F');

  sleep(1);

  currTime = time(NULL);
  r = wh_put(ref, "7", 1, "110", 3, hash); //insert '7' to WH, 7 is inserted to the hash again but now with currTime + 3 and T
  printf("wh_put 7 inserted %c\n", r?'T':'F');

  r = was_inserted_since(currTime-2, 7, hash); //using was_inserted_since currTime - 2
  printf("Did 7 was inserted since time=%d (SHOULD BE 'T') %c\n", currTime-2, r?'T':'F');

  r = was_inserted_since(currTime+1, 7, hash); //using was_inserted_since currTime + 1 
  printf("Did 7 was inserted since time=%d (SHOULD BE 'F') %c\n",currTime+1, r?'T':'F');

  sleep(1);

  r = wh_put(ref, "1007", 4, "111", 3, hash); //insert '1007' to WH, 1007 is inserted to the hash to the same cell as 7, with time F
  printf("wh_put 1007 inserted %c\n", r?'T':'F');

  r = wh_del(ref, "1007", 4); ; //delete '1007' from WH
  printf("wh_put 1007 deleted %c\n", r?'T':'F');

  sleep(1);
  currTime = time(NULL);

  r = was_inserted_since(currTime-5, 7, hash); //using was_inserted_since after the key was deleted from WH
  printf("Did 7 was inserted since time = %d (SHOULD BE 'T') %c\n", currTime,  r?'T':'F');

  r = wh_probe(ref, "7", 1, hash); //success search '7'
  printf("wh_probe 7 %c\n", r?'T':'F');

  r = wh_probe(ref, "1007", 1, hash); //fail search '1007', 1007 is inserted to the hash again but now with time and F
  printf("wh_probe 1007 %c\n", r?'T':'F');

  r = was_inserted_since(currTime-7, 1007, hash);
  printf("Did 1007 was inserted since time = %d (SHOULD BE 'T') %c\n", currTime-7,  r?'T':'F');

  r = wh_put(ref, "7", 1, "110", 3, hash); //
  printf("wh_put 7 insert twice %c\n", r?'T':'F');

  sleep(1);
  currTime = time(NULL);

  r = was_inserted_since(currTime, 7, hash); //after the second insert
  printf("Did 7 was inserted since time = %d (SHOULD BE 'F') %c\n", currTime,  r?'T':'F');

    r = was_inserted_since(currTime-2, 7, hash); //before the second insert
  printf("Did 7 was inserted since time = %d (SHOULD BE 'T') %c\n", currTime-2,  r?'T':'F');

  printf("Before cleaning\n");
  hash_clean(hash);
  printf("After cleaning\n");
  int ss[2];
  printf("hash looking for 7, output is %d\n", hash_get(hash, 7, ss));
  /*
  r = wh_put(ref, "1234", 4, "1111", 4, hash);
  printf("wh_put 1234 easy %c\n", r?'T':'F');

  r = wh_put(ref, "88", 2, "110", 3, hash);
  printf("wh_put 88 impossible %c\n", r?'T':'F');

  r = wh_del(ref, "88", 2);
  printf("wh_del 88 %c\n", r?'T':'F');

  r = wh_probe(ref, "88", 2, hash);
  printf("wh_probe 88 %c\n", r?'T':'F');
  
  u32 klen_out = 0;
  char kbuf_out[8] = {};
  u32 vlen_out = 0;
  char vbuf_out[8] = {};
  r = wh_get(ref, "1234", 4, vbuf_out, 8, &vlen_out);
  printf("wh_get 1234 %c %u %.*s\n", r?'T':'F', vlen_out, vlen_out, vbuf_out);
  */
  //check if 7 was inserted


  /*
  // in a concurrent environment, the kvmap_api_wormhole need park&resume when a thread is about to go idle
  // don't need park&resume if you're using the default kvmap_api_whsafe in whwh.c!
  wh_park(ref);
  usleep(10);
  wh_resume(ref);

  // prepare a few keys for range ops
  wh_put(ref, "00", 2, "0_value", 7, hash);
  wh_put(ref, "11", 2, "1_value", 7, hash);
  wh_put(ref, "22", 2, "2_value", 7, hash);
  
  struct wormhole_iter * const iter = wh_iter_create(ref);
  
  wh_iter_seek(iter, NULL, 0); // seek to the head
  printf("wh_iter_seek \"\"\n");
  while (wh_iter_valid(iter)) {
    r = wh_iter_peek(iter, kbuf_out, 8, &klen_out, vbuf_out, 8, &vlen_out);
    if (r) {
      printf("wh_iter_peek klen=%u key=%.*s vlen=%u value=%.*s\n",
          klen_out, klen_out, kbuf_out, vlen_out, vlen_out, vbuf_out);
    } else {
      printf("ERROR!\n");
    }
    wh_iter_skip1(iter);
  }
  
  // call iter_park if you will go idle but want to use the iter later
  // don't need to call iter_park if you're actively using iter
  wh_iter_park(iter);
  usleep(10);

  wh_iter_seek(iter, "0", 1);
  printf("wh_iter_seek \"0\"\n");
  // this time we don't want to copy the value
  r = wh_iter_peek(iter, kbuf_out, 8, &klen_out, NULL, 0, NULL);
  if (r){
    printf("wh_iter_peek klen=%u key=%.*s\n", klen_out, klen_out, kbuf_out);
  } else {
    printf("ERROR: iter_peek failed\n");
  }

  
  wh_iter_destroy(iter);
  // there must be no active iter when calling unref()
  wh_unref(ref);

  */
  // unsafe operations: should have released all references
  wh_clean(wh); // just for demonstration
  wh_destroy(wh); // destroy also calls clean interally

  hash_destroy(hash); //I added- destroy hash
  return 0;
}
