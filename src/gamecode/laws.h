/*
Copyright (c) 1998-2010, Enno Rehling <enno@eressea.de>
                         Katja Zedel <katze@felidae.kn-bremen.de
                         Christian Schlittchen <corwin@amber.kn-bremen.de>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#ifndef H_GC_LAWS
#define H_GC_LAWS
#ifdef __cplusplus
extern "C" {
#endif

  extern int writepasswd(void);
  int getoption(void);
  int wanderoff(struct region *r, int p);
  void demographics(void);
  void last_orders(void);
  void find_address(void);
  void update_guards(void);
  void update_subscriptions(void);
  void deliverMail(struct faction *f, struct region *r, struct unit *u,
    const char *s, struct unit *receiver);
  int init_data(const char *filename, const char *catalog);

  boolean renamed_building(const struct building * b);
  int rename_building(struct unit * u, struct order * ord, struct building * b, const char *name);
  void get_food(struct region * r);

/* eressea-specific. put somewhere else, please. */
  void processorders(void);
  extern struct attrib_type at_germs;

  extern int dropouts[2];
  extern int *age;

#ifdef __cplusplus
}
#endif
#endif
