/*
glib_compat.c replacement functionality for glib code used in qemu
Copyright (C) 2016 Chris Eagle cseagle at gmail dot com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Part of this code was lifted from glib-2.28.0.
// Glib license is available in COPYING_GLIB file in root directory.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "glib_compat.h"
#include "qemu/atomic.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#ifndef _WIN64
#define GPOINTER_TO_UINT(p) ((guint)(uintptr_t)(p))
#else
#define GPOINTER_TO_UINT(p) ((guint) (guint64) (p))
#endif
#define G_MAXINT    INT_MAX

/* All functions below added to eliminate GLIB dependency */

/* hashing and equality functions */
// Hash functions lifted glib-2.28.0/glib/ghash.c

/**
 * g_direct_hash:
 * @v: a #gpointer key
 *
 * Converts a gpointer to a hash value.
 * It can be passed to g_hash_table_new() as the @hash_func parameter,
 * when using pointers as keys in a #GHashTable.
 *
 * Returns: a hash value corresponding to the key.
 */
guint g_direct_hash (gconstpointer v)
{
  return GPOINTER_TO_UINT (v);
}

/**
 * g_direct_equal:
 * @v1: a key.
 * @v2: a key to compare with @v1.
 *
 * Compares two #gpointer arguments and returns %TRUE if they are equal.
 * It can be passed to g_hash_table_new() as the @key_equal_func
 * parameter, when using pointers as keys in a #GHashTable.
 *
 * Returns: %TRUE if the two keys match.
 */
gboolean
g_direct_equal (gconstpointer v1,
    gconstpointer v2)
{
  return v1 == v2;
}

// g_str_hash() is lifted glib-2.28.0/glib/gstring.c
/**
 * g_str_hash:
 * @v: a string key
 *
 * Converts a string to a hash value.
 *
 * This function implements the widely used "djb" hash apparently posted
 * by Daniel Bernstein to comp.lang.c some time ago.  The 32 bit
 * unsigned hash value starts at 5381 and for each byte 'c' in the
 * string, is updated: <literal>hash = hash * 33 + c</literal>.  This
 * function uses the signed value of each byte.
 *
 * It can be passed to g_hash_table_new() as the @hash_func parameter,
 * when using strings as keys in a #GHashTable.
 *
 * Returns: a hash value corresponding to the key
 **/
guint g_str_hash (gconstpointer v)
{
  const signed char *p;
  guint32 h = 5381;

  for (p = v; *p != '\0'; p++)
    h = (h << 5) + h + *p;

  return h;
}

gboolean g_str_equal(gconstpointer v1, gconstpointer v2)
{
   return strcmp((const char*)v1, (const char*)v2) == 0;
}

/**
 * g_str_has_suffix:
 * @str: a nul-terminated string.
 * @suffix: the nul-terminated suffix to look for.
 *
 * Looks whether the string @str ends with @suffix.
 *
 * Return value: %TRUE if @str end with @suffix, %FALSE otherwise.
 *
 * Since: 2.2
 **/
gboolean
g_str_has_suffix(const gchar *str, const gchar *suffix)
{
  int str_len;
  int suffix_len;

  if (str == NULL || suffix == NULL) {
    return FALSE;
  }

  str_len = strlen (str);
  suffix_len = strlen (suffix);

  if (str_len < suffix_len)
    return FALSE;

  return strcmp (str + str_len - suffix_len, suffix) == 0;
}

/**
 * g_str_has_prefix:
 * @str: a nul-terminated string.
 * @prefix: the nul-terminated prefix to look for.
 *
 * Looks whether the string @str begins with @prefix.
 *
 * Return value: %TRUE if @str begins with @prefix, %FALSE otherwise.
 *
 * Since: 2.2
 **/
gboolean
g_str_has_prefix(const gchar *str, const gchar *prefix)
{
  int str_len;
  int prefix_len;

  if (str == NULL || prefix == NULL) {
    return FALSE;
  }

  str_len = strlen (str);
  prefix_len = strlen (prefix);

  if (str_len < prefix_len)
    return FALSE;

  return strncmp (str, prefix, prefix_len) == 0;
}

// g_int_hash() is lifted from glib-2.28.0/glib/gutils.c
/**
 * g_int_hash:
 * @v: a pointer to a #gint key
 *
 * Converts a pointer to a #gint to a hash value.
 * It can be passed to g_hash_table_new() as the @hash_func parameter,
 * when using pointers to integers values as keys in a #GHashTable.
 *
 * Returns: a hash value corresponding to the key.
 */
guint g_int_hash (gconstpointer v)
{
  return *(const gint*) v;
}

gboolean g_int_equal(gconstpointer v1, gconstpointer v2)
{
   return *((const gint*)v1) == *((const gint*)v2);
}

/* Doubly-linked list */

GList *g_list_first(GList *list)
{
   if (list == NULL) return NULL;
   while (list->prev) list = list->prev;
   return list;
}

void g_list_foreach(GList *list, GFunc func, gpointer user_data)
{
   GList *lp;
   for (lp = list; lp; lp = lp->next) {
      (*func)(lp->data, user_data);
   }
}

void g_list_free(GList *list)
{
   GList *lp, *next, *prev = NULL;
   if (list) prev = list->prev;
   for (lp = list; lp; lp = next) {
      next = lp->next;
      free(lp);
   }
   for (lp = prev; lp; lp = prev) {
      prev = lp->prev;
      free(lp);
   }
}

GList *g_list_insert_sorted(GList *list, gpointer data, GCompareFunc compare)
{
   GList *i;
   GList *n = (GList*)g_malloc(sizeof(GList));
   n->data = data;
   if (list == NULL) {
      n->next = n->prev = NULL;
      return n;
   }
   for (i = list; i; i = i->next) {
      n->prev = i->prev;
      if ((*compare)(data, i->data) <= 0) {
         n->next = i;
         i->prev = n;
         if (i == list) return n;
         else return list;
      }
   }
   n->prev = n->prev->next;
   n->next = NULL;
   n->prev->next = n;
   return list;
}

GList *g_list_prepend(GList *list, gpointer data)
{
   GList *n = (GList*)g_malloc(sizeof(GList));
   n->next = list;
   n->prev = NULL;
   n->data = data;
   return n;
}

GList *g_list_remove_link(GList *list, GList *llink)
{
   if (llink) {
      if (llink == list) list = list->next;
      if (llink->prev) llink->prev->next = llink->next;
      if (llink->next) llink->next->prev = llink->prev;
   }
   return list;
}

// code copied from glib/glist.c, version 2.28.0
static GList *g_list_sort_merge(GList *l1,
           GList     *l2,
           GFunc     compare_func,
           gpointer  user_data)
{
  GList list, *l, *lprev;
  gint cmp;

  l = &list;
  lprev = NULL;

  while (l1 && l2)
    {
      cmp = ((GCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

      if (cmp <= 0)
        {
      l->next = l1;
      l1 = l1->next;
        }
      else
    {
      l->next = l2;
      l2 = l2->next;
        }
      l = l->next;
      l->prev = lprev;
      lprev = l;
    }
  l->next = l1 ? l1 : l2;
  l->next->prev = l;

  return list.next;
}

static GList *g_list_sort_real(GList *list,
          GFunc     compare_func,
          gpointer  user_data)
{
  GList *l1, *l2;

  if (!list)
    return NULL;
  if (!list->next)
    return list;

  l1 = list;
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL)
    break;
      l1 = l1->next;
    }
  l2 = l1->next;
  l1->next = NULL;

  return g_list_sort_merge (g_list_sort_real (list, compare_func, user_data),
                g_list_sort_real (l2, compare_func, user_data),
                compare_func,
                user_data);
}

/**
 * g_list_sort:
 * @list: a #GList
 * @compare_func: the comparison function used to sort the #GList.
 *     This function is passed the data from 2 elements of the #GList
 *     and should return 0 if they are equal, a negative value if the
 *     first element comes before the second, or a positive value if
 *     the first element comes after the second.
 *
 * Sorts a #GList using the given comparison function.
 *
 * Returns: the start of the sorted #GList
 */
/**
 * GCompareFunc:
 * @a: a value.
 * @b: a value to compare with.
 * @Returns: negative value if @a &lt; @b; zero if @a = @b; positive
 *           value if @a > @b.
 *
 * Specifies the type of a comparison function used to compare two
 * values.  The function should return a negative integer if the first
 * value comes before the second, 0 if they are equal, or a positive
 * integer if the first value comes after the second.
 **/
GList *g_list_sort (GList *list, GCompareFunc  compare_func)
{
    return g_list_sort_real (list, (GFunc) compare_func, NULL);
}

static inline GList*
_g_list_remove_link (GList *list,
         GList *link)
{
  if (link)
    {
      if (link->prev)
        link->prev->next = link->next;
      if (link->next)
        link->next->prev = link->prev;

      if (link == list)
        list = list->next;

      link->next = NULL;
      link->prev = NULL;
    }

  return list;
}

/**
 * g_list_delete_link:
 * @list: a #GList, this must point to the top of the list
 * @link_: node to delete from @list
 *
 * Removes the node link_ from the list and frees it.
 * Compare this to g_list_remove_link() which removes the node 
 * without freeing it.
 *
 * Returns: the (possibly changed) start of the #GList
 */
GList *
g_list_delete_link (GList *list,
                    GList *link_)
{
  list = _g_list_remove_link (list, link_);
  //_g_list_free1 (link_);
  g_free (link_);

  return list;
}

/**
 * g_list_insert_before:
 * @list: a pointer to a #GList
 * @sibling: the list element before which the new element 
 *     is inserted or %NULL to insert at the end of the list
 * @data: the data for the new element
 *
 * Inserts a new element into the list before the given position.
 *
 * Returns: the new start of the #GList
 */
GList*
g_list_insert_before (GList   *list,
          GList   *sibling,
          gpointer data)
{
  if (!list)
    {
      list = g_malloc(sizeof(GList));
      list->data = data;
      return list;
    }
  else if (sibling)
    {
      GList *node;

      node = g_malloc(sizeof(GList));
      node->data = data;
      node->prev = sibling->prev;
      node->next = sibling;
      sibling->prev = node;
      if (node->prev)
  {
    node->prev->next = node;
    return list;
  }
      else
  {
    return node;
  }
    }
  else
    {
      GList *last;

      last = list;
      while (last->next)
  last = last->next;

      last->next = g_malloc(sizeof(GList));
      last->next->data = data;
      last->next->prev = last;
      last->next->next = NULL;

      return list;
    }
}

/* END of g_list related functions */

/* Singly-linked list */

GSList *g_slist_append(GSList *list, gpointer data)
{
   GSList *head = list;
   if (list) {
      while (list->next) list = list->next;
      list->next = (GSList*)g_malloc(sizeof(GSList));
      list = list->next;
   } else {
      head = list = (GSList*)g_malloc(sizeof(GSList));
   }
   list->data = data;
   list->next = NULL;
   return head;   
}

void g_slist_foreach(GSList *list, GFunc func, gpointer user_data)
{
   GSList *lp;
   for (lp = list; lp; lp = lp->next) {
      (*func)(lp->data, user_data);
   }
}

void g_slist_free(GSList *list)
{
   GSList *lp, *next;
   for (lp = list; lp; lp = next) {
      next = lp->next;
      free(lp);
   }
}

GSList *g_slist_prepend(GSList *list, gpointer data)
{
   GSList *head = (GSList*)g_malloc(sizeof(GSList));
   head->next = list;
   head->data = data;
   return head;   
}

static GSList *g_slist_sort_merge (GSList *l1,
                    GSList *l2,
                    GFunc compare_func,
                    gpointer user_data)
{
  GSList list, *l;
  gint cmp;

  l=&list;

  while (l1 && l2)
    {
      cmp = ((GCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

      if (cmp <= 0)
        {
          l=l->next=l1;
          l1=l1->next;
        }
      else
        {
          l=l->next=l2;
          l2=l2->next;
        }
    }
  l->next= l1 ? l1 : l2;

  return list.next;
}

static GSList *g_slist_sort_real (GSList *list,
                   GFunc compare_func,
                   gpointer user_data)
{
  GSList *l1, *l2;

  if (!list)
    return NULL;
  if (!list->next)
    return list;

  l1 = list;
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL)
        break;
      l1=l1->next;
    }
  l2 = l1->next;
  l1->next = NULL;

  return g_slist_sort_merge (g_slist_sort_real (list, compare_func, user_data),
                             g_slist_sort_real (l2, compare_func, user_data),
                             compare_func,
                             user_data);
}

/**
 * g_slist_sort:
 * @list: a #GSList
 * @compare_func: the comparison function used to sort the #GSList.
 *     This function is passed the data from 2 elements of the #GSList
 *     and should return 0 if they are equal, a negative value if the
 *     first element comes before the second, or a positive value if
 *     the first element comes after the second.
 *
 * Sorts a #GSList using the given comparison function.
 *
 * Returns: the start of the sorted #GSList
 */
GSList *g_slist_sort (GSList *list,
              GCompareFunc  compare_func)
{
  return g_slist_sort_real (list, (GFunc) compare_func, NULL);
}

/* END of g_slist related functions */

// String functions lifted from glib-2.28.0/glib/gstring.c

#define MY_MAXSIZE ((gsize)-1)

static inline gsize
nearest_power (gsize base, gsize num)
{
  if (num > MY_MAXSIZE / 2)
    {
      return MY_MAXSIZE;
    }
  else
    {
      gsize n = base;

      while (n < num)
  n <<= 1;

      return n;
    }
}

static void
g_string_maybe_expand (GString* string,
           gsize    len)
{
  if (string->len + len >= string->allocated_len)
    {
      string->allocated_len = nearest_power (1, string->len + len + 1);
      string->str = g_realloc (string->str, string->allocated_len);
    }
}

GString*
g_string_sized_new (gsize dfl_size)
{
  GString *string = malloc(sizeof(GString));

  string->allocated_len = 0;
  string->len   = 0;
  string->str   = NULL;

  g_string_maybe_expand (string, MAX (dfl_size, 2));
  string->str[0] = 0;

  return string;
}

/**
 * g_string_free:
 * @string: a #GString
 * @free_segment: if %TRUE the actual character data is freed as well
 *
 * Frees the memory allocated for the #GString.
 * If @free_segment is %TRUE it also frees the character data.  If
 * it's %FALSE, the caller gains ownership of the buffer and must
 * free it after use with g_free().
 *
 * Returns: the character data of @string
 *          (i.e. %NULL if @free_segment is %TRUE)
 */
gchar*
g_string_free (GString *string,
         gboolean free_segment)
{
  gchar *segment;

  if (string == NULL) {
    return NULL;
  }

  if (free_segment)
    {
      g_free (string->str);
      segment = NULL;
    }
  else
    segment = string->str;

  free(string);
  return segment;
}

/**
 * g_string_insert_len:
 * @string: a #GString
 * @pos: position in @string where insertion should
 *       happen, or -1 for at the end
 * @val: bytes to insert
 * @len: number of bytes of @val to insert
 *
 * Inserts @len bytes of @val into @string at @pos.
 * Because @len is provided, @val may contain embedded
 * nuls and need not be nul-terminated. If @pos is -1,
 * bytes are inserted at the end of the string.
 *
 * Since this function does not stop at nul bytes, it is
 * the caller's responsibility to ensure that @val has at
 * least @len addressable bytes.
 *
 * Returns: @string
 */
GString*
g_string_insert_len (GString     *string,
         gssize       pos,
         const gchar *val,
         gssize       len)
{
  if (string == NULL) {
    return NULL;
  }
  if (len != 0 || val == NULL) {
    return string;
  }

  if (len == 0)
    return string;

  if (len < 0)
    len = strlen (val);

  if (pos < 0)
    pos = string->len;
  else {
    if (pos > string->len) {
      return string;
    }
  }

  /* Check whether val represents a substring of string.  This test
     probably violates chapter and verse of the C standards, since
     ">=" and "<=" are only valid when val really is a substring.
     In practice, it will work on modern archs.  */
  if (val >= string->str && val <= string->str + string->len)
    {
      gsize offset = val - string->str;
      gsize precount = 0;

      g_string_maybe_expand (string, len);
      val = string->str + offset;
      /* At this point, val is valid again.  */

      /* Open up space where we are going to insert.  */
      if (pos < string->len)
        memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* Move the source part before the gap, if any.  */
      if (offset < pos)
        {
          precount = MIN (len, pos - offset);
          memcpy (string->str + pos, val, precount);
        }

      /* Move the source part after the gap, if any.  */
      if (len > precount)
        memcpy (string->str + pos + precount,
                val + /* Already moved: */ precount + /* Space opened up: */ len,
                len - precount);
    }
  else
    {
      g_string_maybe_expand (string, len);

      /* If we aren't appending at the end, move a hunk
       * of the old string to the end, opening up space
       */
      if (pos < string->len)
        memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* insert the new string */
      if (len == 1)
        string->str[pos] = *val;
      else
        memcpy (string->str + pos, val, len);
    }

  string->len += len;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_append_len:
 * @string: a #GString
 * @val: bytes to append
 * @len: number of bytes of @val to use
 *
 * Appends @len bytes of @val to @string. Because @len is
 * provided, @val may contain embedded nuls and need not
 * be nul-terminated.
 *
 * Since this function does not stop at nul bytes, it is
 * the caller's responsibility to ensure that @val has at
 * least @len addressable bytes.
 *
 * Returns: @string
 */
GString*
g_string_append_len (GString   *string,
                     const gchar *val,
                     gssize       len)
{
  if (string == NULL) {
    return  NULL;
  }
  if (len != 0 || val == NULL) {
    return string;
  }

  return g_string_insert_len (string, -1, val, len);
}

/**
 * g_string_prepend:
 * @string: a #GString
 * @val: the string to prepend on the start of @string
 *
 * Adds a string on to the start of a #GString, 
 * expanding it if necessary.
 *
 * Returns: @string
 */
GString*
g_string_prepend (GString     *string,
      const gchar *val)
{
  if (string == NULL) {
    return NULL;
  }
  if (val == NULL) {
    return string;
  }

  return g_string_insert_len (string, 0, val, -1);
}

/**
 * g_string_insert_c:
 * @string: a #GString
 * @pos: the position to insert the byte
 * @c: the byte to insert
 *
 * Inserts a byte into a #GString, expanding it if necessary.
 *
 * Returns: @string
 */
GString*
g_string_insert_c (GString *string,
       gssize   pos,
       gchar    c)
{
  if (string == NULL) {
    return NULL;
  }

  g_string_maybe_expand (string, 1);

  if (pos < 0)
    pos = string->len;
  else {
    if (pos > string->len) {
      return string;
    }
  }

  /* If not just an append, move the old stuff */
  if (pos < string->len)
    memmove (string->str + pos + 1, string->str + pos, string->len - pos);

  string->str[pos] = c;

  string->len += 1;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_prepend_c:
 * @string: a #GString
 * @c: the byte to prepend on the start of the #GString
 *
 * Adds a byte onto the start of a #GString,
 * expanding it if necessary.
 *
 * Returns: @string
 */
GString*
g_string_prepend_c (GString *string,
        gchar    c)
{
  if (string == NULL) {
    return NULL;
  }

  return g_string_insert_c (string, 0, c);
}

/**
 * g_string_truncate:
 * @string: a #GString
 * @len: the new size of @string
 *
 * Cuts off the end of the GString, leaving the first @len bytes. 
 *
 * Returns: @string
 */
GString*
g_string_truncate (GString *string,
       gsize    len)
{
  if (string == NULL) {
    return NULL;
  }

  string->len = MIN (len, string->len);
  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_set_size:
 * @string: a #GString
 * @len: the new length
 *
 * Sets the length of a #GString. If the length is less than
 * the current length, the string will be truncated. If the
 * length is greater than the current length, the contents
 * of the newly added area are undefined. (However, as
 * always, string->str[string->len] will be a nul byte.)
 *
 * Return value: @string
 **/
GString*
g_string_set_size (GString *string,
       gsize    len)
{
  if (string == NULL) {
    return NULL;
  }

  if (len >= string->allocated_len)
    g_string_maybe_expand (string, len - string->len);

  string->len = len;
  string->str[len] = 0;

  return string;
}

/**
 * g_string_new:
 * @init: the initial text to copy into the string
 *
 * Creates a new #GString, initialized with the given string.
 *
 * Returns: the new #GString
 */
GString*
g_string_new (const gchar *init)
{
  GString *string;

  if (init == NULL || *init == '\0')
    string = g_string_sized_new (2);
  else
    {
      gint len;

      len = strlen (init);
      string = g_string_sized_new (len + 2);

      g_string_append_len (string, init, len);
    }

  return string;
}


GString*
g_string_erase (GString *string,
    gssize   pos,
    gssize   len)
{
  if (string == NULL) {
   return NULL;
  }
  if (pos < 0) {
    return string;
  }
  if (pos > string->len) {
    return string;
  }

  if (len < 0)
    len = string->len - pos;
  else
    {
      if (pos + len > string->len) {
        return string;
      }

      if (pos + len < string->len)
    memmove (string->str + pos, string->str + pos + len, string->len - (pos + len));
    }

  string->len -= len;

  string->str[string->len] = 0;

  return string;
}

/* END of g_string related functions */

// Hash functions lifted glib-2.28.0/glib/ghash.c

#define HASH_TABLE_MIN_SHIFT 3  /* 1 << 3 == 8 buckets */

typedef struct _GHashNode GHashNode;

struct _GHashNode {
  gpointer   key;
  gpointer   value;

  /* If key_hash == 0, node is not in use
   * If key_hash == 1, node is a tombstone
   * If key_hash >= 2, node contains data */
  guint      key_hash;
};

struct _GHashTable {
  gint             size;
  gint             mod;
  guint            mask;
  gint             nnodes;
  gint             noccupied;  /* nnodes + tombstones */
  GHashNode       *nodes;
  GHashFunc        hash_func;
  GEqualFunc       key_equal_func;
  volatile gint    ref_count;
  GDestroyNotify   key_destroy_func;
  GDestroyNotify   value_destroy_func;
};

/**
 * g_hash_table_destroy:
 * @hash_table: a #GHashTable.
 *
 * Destroys all keys and values in the #GHashTable and decrements its
 * reference count by 1. If keys and/or values are dynamically allocated,
 * you should either free them first or create the #GHashTable with destroy
 * notifiers using g_hash_table_new_full(). In the latter case the destroy
 * functions you supplied will be called on all keys and values during the
 * destruction phase.
 **/
void g_hash_table_destroy (GHashTable *hash_table)
{
  if (hash_table == NULL) return;
  if (hash_table->ref_count == 0) return;

  g_hash_table_remove_all (hash_table);
  g_hash_table_unref (hash_table);
}

/**
 * g_hash_table_find:
 * @hash_table: a #GHashTable.
 * @predicate:  function to test the key/value pairs for a certain property.
 * @user_data:  user data to pass to the function.
 *
 * Calls the given function for key/value pairs in the #GHashTable until
 * @predicate returns %TRUE.  The function is passed the key and value of
 * each pair, and the given @user_data parameter. The hash table may not
 * be modified while iterating over it (you can't add/remove items).
 *
 * Note, that hash tables are really only optimized for forward lookups,
 * i.e. g_hash_table_lookup().
 * So code that frequently issues g_hash_table_find() or
 * g_hash_table_foreach() (e.g. in the order of once per every entry in a
 * hash table) should probably be reworked to use additional or different
 * data structures for reverse lookups (keep in mind that an O(n) find/foreach
 * operation issued for all n values in a hash table ends up needing O(n*n)
 * operations).
 *
 * Return value: The value of the first key/value pair is returned, for which
 * func evaluates to %TRUE. If no pair with the requested property is found,
 * %NULL is returned.
 *
 * Since: 2.4
 **/
gpointer g_hash_table_find (GHashTable      *hash_table,
                   GHRFunc          predicate,
                   gpointer         user_data)
{
  gint i;

  if (hash_table == NULL) return NULL;
  if (predicate == NULL) return NULL;

  for (i = 0; i < hash_table->size; i++)
    {
      GHashNode *node = &hash_table->nodes [i];

      if (node->key_hash > 1 && predicate (node->key, node->value, user_data))
        return node->value;
    }

  return NULL;
}

/**
 * g_hash_table_foreach:
 * @hash_table: a #GHashTable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the
 * #GHashTable.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * g_hash_table_foreach_remove().
 *
 * See g_hash_table_find() for performance caveats for linear
 * order searches in contrast to g_hash_table_lookup().
 **/
void g_hash_table_foreach (GHashTable *hash_table,
                      GHFunc      func,
                      gpointer    user_data)
{
  gint i;

  if (hash_table == NULL) return;
  if (func == NULL) return;

  for (i = 0; i < hash_table->size; i++)
    {
      GHashNode *node = &hash_table->nodes [i];

      if (node->key_hash > 1)
        (* func) (node->key, node->value, user_data);
    }
}

/*
 * g_hash_table_lookup_node_for_insertion:
 * @hash_table: our #GHashTable
 * @key: the key to lookup against
 * @hash_return: key hash return location
 * Return value: index of the described #GHashNode
 *
 * Performs a lookup in the hash table, preserving extra information
 * usually needed for insertion.
 *
 * This function first computes the hash value of the key using the
 * user's hash function.
 *
 * If an entry in the table matching @key is found then this function
 * returns the index of that entry in the table, and if not, the
 * index of an unused node (empty or tombstone) where the key can be
 * inserted.
 *
 * The computed hash value is returned in the variable pointed to
 * by @hash_return. This is to save insertions from having to compute
 * the hash record again for the new record.
 */
static inline guint g_hash_table_lookup_node_for_insertion (GHashTable    *hash_table,
                                        gconstpointer  key,
                                        guint         *hash_return)
{
  GHashNode *node;
  guint node_index;
  guint hash_value;
  guint first_tombstone;
  gboolean have_tombstone = FALSE;
  guint step = 0;

  /* Empty buckets have hash_value set to 0, and for tombstones, it's 1.
   * We need to make sure our hash value is not one of these. */

  hash_value = (* hash_table->hash_func) (key);
  if (hash_value <= 1)
    hash_value = 2;

  *hash_return = hash_value;

  node_index = hash_value % hash_table->mod;
  node = &hash_table->nodes [node_index];

  while (node->key_hash)
    {
      /*  We first check if our full hash values
       *  are equal so we can avoid calling the full-blown
       *  key equality function in most cases.
       */

      if (node->key_hash == hash_value)
        {
          if (hash_table->key_equal_func)
            {
              if (hash_table->key_equal_func (node->key, key))
                return node_index;
            }
          else if (node->key == key)
            {
              return node_index;
            }
        }
      else if (node->key_hash == 1 && !have_tombstone)
        {
          first_tombstone = node_index;
          have_tombstone = TRUE;
        }

      step++;
      node_index += step;
      node_index &= hash_table->mask;
      node = &hash_table->nodes [node_index];
    }

  if (have_tombstone)
    return first_tombstone;

  return node_index;
}

/* Each table size has an associated prime modulo (the first prime
 * lower than the table size) used to find the initial bucket. Probing
 * then works modulo 2^n. The prime modulo is necessary to get a
 * good distribution with poor hash functions. */
static const gint prime_mod [] = {
  1,          /* For 1 << 0 */
  2,
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,      /* For 1 << 16 */
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647  /* For 1 << 31 */
};

static void g_hash_table_set_shift (GHashTable *hash_table, gint shift)
{
  gint i;
  guint mask = 0;

  hash_table->size = 1 << shift;
  hash_table->mod  = prime_mod [shift];

  for (i = 0; i < shift; i++)
    {
      mask <<= 1;
      mask |= 1;
    }

  hash_table->mask = mask;
}

static gint g_hash_table_find_closest_shift (gint n)
{
  gint i;

  for (i = 0; n; i++)
    n >>= 1;

  return i;
}

static void g_hash_table_set_shift_from_size (GHashTable *hash_table, gint size)
{
  gint shift;

  shift = g_hash_table_find_closest_shift (size);
  shift = MAX (shift, HASH_TABLE_MIN_SHIFT);

  g_hash_table_set_shift (hash_table, shift);
}

/*
 * g_hash_table_resize:
 * @hash_table: our #GHashTable
 *
 * Resizes the hash table to the optimal size based on the number of
 * nodes currently held.  If you call this function then a resize will
 * occur, even if one does not need to occur.  Use
 * g_hash_table_maybe_resize() instead.
 *
 * This function may "resize" the hash table to its current size, with
 * the side effect of cleaning up tombstones and otherwise optimizing
 * the probe sequences.
 */
static void g_hash_table_resize (GHashTable *hash_table)
{
  GHashNode *new_nodes;
  gint old_size;
  gint i;

  old_size = hash_table->size;
  g_hash_table_set_shift_from_size (hash_table, hash_table->nnodes * 2);

  new_nodes = g_new0 (GHashNode, hash_table->size);

  for (i = 0; i < old_size; i++)
    {
      GHashNode *node = &hash_table->nodes [i];
      GHashNode *new_node;
      guint hash_val;
      guint step = 0;

      if (node->key_hash <= 1)
        continue;

      hash_val = node->key_hash % hash_table->mod;
      new_node = &new_nodes [hash_val];

      while (new_node->key_hash)
        {
          step++;
          hash_val += step;
          hash_val &= hash_table->mask; new_node = &new_nodes [hash_val];
        }

      *new_node = *node;
    }

  g_free (hash_table->nodes);
  hash_table->nodes = new_nodes;
  hash_table->noccupied = hash_table->nnodes;
}

/*
 * g_hash_table_maybe_resize:
 * @hash_table: our #GHashTable
 *
 * Resizes the hash table, if needed.
 *
 * Essentially, calls g_hash_table_resize() if the table has strayed
 * too far from its ideal size for its number of nodes.
 */
static inline void g_hash_table_maybe_resize (GHashTable *hash_table)
{
  gint noccupied = hash_table->noccupied;
  gint size = hash_table->size;

  if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) ||
      (size <= noccupied + (noccupied / 16)))
    g_hash_table_resize (hash_table);
}

/*
 * g_hash_table_insert_internal:
 * @hash_table: our #GHashTable
 * @key: the key to insert
 * @value: the value to insert
 * @keep_new_key: if %TRUE and this key already exists in the table
 *   then call the destroy notify function on the old key.  If %FALSE
 *   then call the destroy notify function on the new key.
 *
 * Implements the common logic for the g_hash_table_insert() and
 * g_hash_table_replace() functions.
 *
 * Do a lookup of @key.  If it is found, replace it with the new
 * @value (and perhaps the new @key).  If it is not found, create a
 * new node.
 */
static void g_hash_table_insert_internal (GHashTable *hash_table,
                              gpointer    key,
                              gpointer    value,
                              gboolean    keep_new_key)
{
  GHashNode *node;
  guint node_index;
  guint key_hash;
  guint old_hash;

  if (hash_table == NULL) return;
  if (hash_table->ref_count == 0) return;

  node_index = g_hash_table_lookup_node_for_insertion (hash_table, key, &key_hash);
  node = &hash_table->nodes [node_index];

  old_hash = node->key_hash;

  if (old_hash > 1)
    {
      if (keep_new_key)
        {
          if (hash_table->key_destroy_func)
            hash_table->key_destroy_func (node->key);
          node->key = key;
        }
      else
        {
          if (hash_table->key_destroy_func)
            hash_table->key_destroy_func (key);
        }

      if (hash_table->value_destroy_func)
        hash_table->value_destroy_func (node->value);

      node->value = value;
    }
  else
    {
      node->key = key;
      node->value = value;
      node->key_hash = key_hash;

      hash_table->nnodes++;

      if (old_hash == 0)
        {
          /* We replaced an empty node, and not a tombstone */
          hash_table->noccupied++;
          g_hash_table_maybe_resize (hash_table);
        }
    }
}

/**
 * g_hash_table_insert:
 * @hash_table: a #GHashTable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #GHashTable.
 *
 * If the key already exists in the #GHashTable its current value is replaced
 * with the new value. If you supplied a @value_destroy_func when creating the
 * #GHashTable, the old value is freed using that function. If you supplied
 * a @key_destroy_func when creating the #GHashTable, the passed key is freed
 * using that function.
 **/
void g_hash_table_insert (GHashTable *hash_table,
                     gpointer    key,
                     gpointer    value)
{
  g_hash_table_insert_internal (hash_table, key, value, FALSE);
}

/**
 * g_hash_table_replace:
 * @hash_table: a #GHashTable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 *
 * Inserts a new key and value into a #GHashTable similar to
 * g_hash_table_insert(). The difference is that if the key already exists
 * in the #GHashTable, it gets replaced by the new key. If you supplied a
 * @value_destroy_func when creating the #GHashTable, the old value is freed
 * using that function. If you supplied a @key_destroy_func when creating the
 * #GHashTable, the old key is freed using that function.
 **/
void
g_hash_table_replace (GHashTable *hash_table,
                      gpointer    key,
                      gpointer    value)
{
  g_hash_table_insert_internal (hash_table, key, value, TRUE);
}

/*
 * g_hash_table_lookup_node:
 * @hash_table: our #GHashTable
 * @key: the key to lookup against
 * @hash_return: optional key hash return location
 * Return value: index of the described #GHashNode
 *
 * Performs a lookup in the hash table.  Virtually all hash operations
 * will use this function internally.
 *
 * This function first computes the hash value of the key using the
 * user's hash function.
 *
 * If an entry in the table matching @key is found then this function
 * returns the index of that entry in the table, and if not, the
 * index of an empty node (never a tombstone).
 */
static inline guint g_hash_table_lookup_node (GHashTable    *hash_table,
                          gconstpointer  key)
{
  GHashNode *node;
  guint node_index;
  guint hash_value;
  guint step = 0;

  /* Empty buckets have hash_value set to 0, and for tombstones, it's 1.
   * We need to make sure our hash value is not one of these. */

  hash_value = (* hash_table->hash_func) (key);
  if (hash_value <= 1)
    hash_value = 2;

  node_index = hash_value % hash_table->mod;
  node = &hash_table->nodes [node_index];

  while (node->key_hash)
    {
      /*  We first check if our full hash values
       *  are equal so we can avoid calling the full-blown
       *  key equality function in most cases.
       */

      if (node->key_hash == hash_value)
        {
          if (hash_table->key_equal_func)
            {
              if (hash_table->key_equal_func (node->key, key))
                break;
            }
          else if (node->key == key)
            {
              break;
            }
        }

      step++;
      node_index += step;
      node_index &= hash_table->mask;
      node = &hash_table->nodes [node_index];
    }

  return node_index;
}

/**
 * g_hash_table_lookup:
 * @hash_table: a #GHashTable.
 * @key: the key to look up.
 *
 * Looks up a key in a #GHashTable. Note that this function cannot
 * distinguish between a key that is not present and one which is present
 * and has the value %NULL. If you need this distinction, use
 * g_hash_table_lookup_extended().
 *
 * Return value: the associated value, or %NULL if the key is not found.
 **/
gpointer g_hash_table_lookup (GHashTable   *hash_table,
                     gconstpointer key)
{
  GHashNode *node;
  guint      node_index;

  if (hash_table == NULL) return NULL;

  node_index = g_hash_table_lookup_node (hash_table, key);
  node = &hash_table->nodes [node_index];

  return node->key_hash ? node->value : NULL;
}

/**
 * g_hash_table_new:
 * @hash_func: a function to create a hash value from a key.
 *   Hash values are used to determine where keys are stored within the
 *   #GHashTable data structure. The g_direct_hash(), g_int_hash(),
 *   g_int64_hash(), g_double_hash() and g_str_hash() functions are provided
 *   for some common types of keys.
 *   If hash_func is %NULL, g_direct_hash() is used.
 * @key_equal_func: a function to check two keys for equality.  This is
 *   used when looking up keys in the #GHashTable.  The g_direct_equal(),
 *   g_int_equal(), g_int64_equal(), g_double_equal() and g_str_equal()
 *   functions are provided for the most common types of keys.
 *   If @key_equal_func is %NULL, keys are compared directly in a similar
 *   fashion to g_direct_equal(), but without the overhead of a function call.
 *
 * Creates a new #GHashTable with a reference count of 1.
 *
 * Return value: a new #GHashTable.
 **/
GHashTable *g_hash_table_new(GHashFunc hash_func, GEqualFunc key_equal_func)
{
   return g_hash_table_new_full(hash_func, key_equal_func, NULL, NULL);
}

/**
 * g_hash_table_new_full:
 * @hash_func: a function to create a hash value from a key.
 * @key_equal_func: a function to check two keys for equality.
 * @key_destroy_func: a function to free the memory allocated for the key
 *   used when removing the entry from the #GHashTable or %NULL if you
 *   don't want to supply such a function.
 * @value_destroy_func: a function to free the memory allocated for the
 *   value used when removing the entry from the #GHashTable or %NULL if
 *   you don't want to supply such a function.
 *
 * Creates a new #GHashTable like g_hash_table_new() with a reference count
 * of 1 and allows to specify functions to free the memory allocated for the
 * key and value that get called when removing the entry from the #GHashTable.
 *
 * Return value: a new #GHashTable.
 **/
GHashTable* g_hash_table_new_full (GHashFunc       hash_func,
                       GEqualFunc      key_equal_func,
                       GDestroyNotify  key_destroy_func,
                       GDestroyNotify  value_destroy_func)
{
  GHashTable *hash_table;

  hash_table = (GHashTable*)g_malloc(sizeof(GHashTable));
  //hash_table = g_slice_new (GHashTable);
  g_hash_table_set_shift (hash_table, HASH_TABLE_MIN_SHIFT);
  hash_table->nnodes             = 0;
  hash_table->noccupied          = 0;
  hash_table->hash_func          = hash_func ? hash_func : g_direct_hash;
  hash_table->key_equal_func     = key_equal_func;
  hash_table->ref_count          = 1;
  hash_table->key_destroy_func   = key_destroy_func;
  hash_table->value_destroy_func = value_destroy_func;
  hash_table->nodes              = g_new0 (GHashNode, hash_table->size);

  return hash_table;
}

/*
 * g_hash_table_remove_all_nodes:
 * @hash_table: our #GHashTable
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes all nodes from the table.  Since this may be a precursor to
 * freeing the table entirely, no resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 */
static void g_hash_table_remove_all_nodes (GHashTable *hash_table,
                               gboolean    notify)
{
  int i;

  for (i = 0; i < hash_table->size; i++)
    {
      GHashNode *node = &hash_table->nodes [i];

      if (node->key_hash > 1)
        {
          if (notify && hash_table->key_destroy_func)
            hash_table->key_destroy_func (node->key);

          if (notify && hash_table->value_destroy_func)
            hash_table->value_destroy_func (node->value);
        }
    }

  /* We need to set node->key_hash = 0 for all nodes - might as well be GC
   * friendly and clear everything */
  memset (hash_table->nodes, 0, hash_table->size * sizeof (GHashNode));

  hash_table->nnodes = 0;
  hash_table->noccupied = 0;
}

/**
 * g_hash_table_remove_all:
 * @hash_table: a #GHashTable
 *
 * Removes all keys and their associated values from a #GHashTable.
 *
 * If the #GHashTable was created using g_hash_table_new_full(), the keys
 * and values are freed using the supplied destroy functions, otherwise you
 * have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Since: 2.12
 **/
void g_hash_table_remove_all (GHashTable *hash_table)
{
  if (hash_table == NULL) return;

  g_hash_table_remove_all_nodes (hash_table, TRUE);
  g_hash_table_maybe_resize (hash_table);
}

/*
 * g_hash_table_remove_node:
 * @hash_table: our #GHashTable
 * @node: pointer to node to remove
 * @notify: %TRUE if the destroy notify handlers are to be called
 *
 * Removes a node from the hash table and updates the node count.
 * The node is replaced by a tombstone. No table resize is performed.
 *
 * If @notify is %TRUE then the destroy notify functions are called
 * for the key and value of the hash node.
 */
static void g_hash_table_remove_node (GHashTable   *hash_table,
                          GHashNode    *node,
                          gboolean      notify)
{
  if (notify && hash_table->key_destroy_func)
    hash_table->key_destroy_func (node->key);

  if (notify && hash_table->value_destroy_func)
    hash_table->value_destroy_func (node->value);

  /* Erect tombstone */
  node->key_hash = 1;

  /* Be GC friendly */
  node->key = NULL;
  node->value = NULL;

  hash_table->nnodes--;
}
/*
 * g_hash_table_remove_internal:
 * @hash_table: our #GHashTable
 * @key: the key to remove
 * @notify: %TRUE if the destroy notify handlers are to be called
 * Return value: %TRUE if a node was found and removed, else %FALSE
 *
 * Implements the common logic for the g_hash_table_remove() and
 * g_hash_table_steal() functions.
 *
 * Do a lookup of @key and remove it if it is found, calling the
 * destroy notify handlers only if @notify is %TRUE.
 */
static gboolean g_hash_table_remove_internal (GHashTable *hash_table,
                gconstpointer  key,
                gboolean       notify)
{
  GHashNode *node;
  guint node_index;

  if (hash_table == NULL) return FALSE;

  node_index = g_hash_table_lookup_node (hash_table, key);
  node = &hash_table->nodes [node_index];

  /* g_hash_table_lookup_node() never returns a tombstone, so this is safe */
  if (!node->key_hash)
    return FALSE;

  g_hash_table_remove_node (hash_table, node, notify);
  g_hash_table_maybe_resize (hash_table);

  return TRUE;
}
/**
 * g_hash_table_remove:
 * @hash_table: a #GHashTable.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #GHashTable.
 *
 * If the #GHashTable was created using g_hash_table_new_full(), the
 * key and value are freed using the supplied destroy functions, otherwise
 * you have to make sure that any dynamically allocated values are freed
 * yourself.
 *
 * Return value: %TRUE if the key was found and removed from the #GHashTable.
 **/
gboolean g_hash_table_remove (GHashTable    *hash_table,
                     gconstpointer  key)
{
  return g_hash_table_remove_internal (hash_table, key, TRUE);
}

/**
 * g_hash_table_unref:
 * @hash_table: a valid #GHashTable.
 *
 * Atomically decrements the reference count of @hash_table by one.
 * If the reference count drops to 0, all keys and values will be
 * destroyed, and all memory allocated by the hash table is released.
 * This function is MT-safe and may be called from any thread.
 *
 * Since: 2.10
 **/
void g_hash_table_unref (GHashTable *hash_table)
{
  if (hash_table == NULL) return;
  if (hash_table->ref_count == 0) return;

  hash_table->ref_count--;
  if (hash_table->ref_count == 0) {
      g_hash_table_remove_all_nodes (hash_table, TRUE);
      g_free (hash_table->nodes);
      g_free (hash_table);
  }
}

/**
 * g_hash_table_ref:
 * @hash_table: a valid #GHashTable.
 *
 * Atomically increments the reference count of @hash_table by one.
 * This function is MT-safe and may be called from any thread.
 *
 * Return value: the passed in #GHashTable.
 *
 * Since: 2.10
 **/
GHashTable *g_hash_table_ref (GHashTable *hash_table)
{
  if (hash_table == NULL) return NULL;
  if (hash_table->ref_count == 0) return hash_table;

  //g_atomic_int_add (&hash_table->ref_count, 1);
  hash_table->ref_count++;
  return hash_table;
}

guint g_hash_table_size(GHashTable *hash_table)
{
  if (hash_table == NULL) return 0;

  return hash_table->nnodes;
}

typedef struct
{
  GHashTable  *hash_table;
  gpointer     dummy1;
  gpointer     dummy2;
  int          position;
  gboolean     dummy3;
  int          version;
} RealIter;

#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)
#define HASH_IS_REAL(h_) ((h_) >= 2)

void g_hash_table_iter_init(GHashTableIter *iter, GHashTable *hash_table)
{
  RealIter *ri = (RealIter *) iter;

  if (iter == NULL) {
    return;
  }
  if (hash_table == NULL) {
    return;
  }

  ri->hash_table = hash_table;
  ri->position = -1;
}

gboolean g_hash_table_iter_next(GHashTableIter *iter, gpointer *key, gpointer *value)
{
  RealIter *ri = (RealIter *) iter;
  GHashNode *node;
  gint position;

  if (iter == NULL)
  {
    return FALSE;
  }
  if (ri->position >= ri->hash_table->size)
  {
    return FALSE;
  }

  position = ri->position;

  do
  {
    position++;
    if (position >= ri->hash_table->size)
    {
      ri->position = position;
      return FALSE;
    }

    node = &ri->hash_table->nodes [position];
  }
  while (node->key_hash <= 1);

  if (key != NULL)
    *key = node->key;
  if (value != NULL)
    *value = node->value;

  ri->position = position;
  return TRUE;
}

GHashTable *g_hash_table_iter_get_hash_table(GHashTableIter *iter)
{
  if (iter == NULL) {
    return NULL;
  }

  return ((RealIter *) iter)->hash_table;
}

static void iter_remove_or_steal(RealIter *ri, gboolean notify)
{
  if (ri == NULL) {
    return;
  }
  if (ri->position < 0) {
    return;
  }
  if (ri->position >= ri->hash_table->size) {
    return;
  }

g_hash_table_remove_node (ri->hash_table, &ri->hash_table->nodes[ri->position], notify);
}

void g_hash_table_iter_remove(GHashTableIter *iter)
{
  iter_remove_or_steal((RealIter *) iter, TRUE);
}

void g_hash_table_iter_steal(GHashTableIter *iter)
{
  iter_remove_or_steal((RealIter *) iter, FALSE);
}

/* END of g_hash_table related functions */

/* START of GTree related functions */

#define MAX_GTREE_HEIGHT 40
typedef struct _GTreeNode  GTreeNode;

static void
g_tree_insert_internal (GTree    *tree,
                        gpointer  key,
                        gpointer  value,
                        gboolean  replace);

static GTreeNode*
g_tree_node_rotate_left (GTreeNode *node);
static GTreeNode*
g_tree_node_rotate_right (GTreeNode *node);
static GTreeNode *
g_tree_find_node (GTree *tree, gconstpointer key);
static gint
g_tree_node_pre_order (GTreeNode     *node,
           GTraverseFunc  traverse_func,
           gpointer       data);
static gint
g_tree_node_in_order (GTreeNode     *node,
          GTraverseFunc  traverse_func,
          gpointer       data);
static gint
g_tree_node_post_order (GTreeNode     *node,
      GTraverseFunc  traverse_func,
      gpointer       data);
static GTreeNode*
g_tree_node_balance(GTreeNode *node);
static gpointer
g_tree_node_search (GTreeNode     *node,
        GCompareFunc   search_func,
        gconstpointer  data);

/**
 * GTree:
 *
 * The <structname>GTree</structname> struct is an opaque data
 * structure representing a <link
 * linkend="glib-Balanced-Binary-Trees">Balanced Binary Tree</link>. It
 * should be accessed only by using the following functions.
 **/
struct _GTree
{
  GTreeNode        *root;
  GCompareDataFunc  key_compare;
  GDestroyNotify    key_destroy_func;
  GDestroyNotify    value_destroy_func;
  gpointer          key_compare_data;
  guint             nnodes;
  gint              ref_count;
};

struct _GTreeNode
{
  gpointer   key;         /* key for this node */
  gpointer   value;       /* value stored at this node */
  GTreeNode *left;        /* left subtree */
  GTreeNode *right;       /* right subtree */
  gint8      balance;     /* height (left) - height (right) */
  guint8     left_child;
  guint8     right_child;
};

static GTreeNode*
g_tree_node_new (gpointer key, gpointer value)
{
  GTreeNode *node = malloc(sizeof(GTreeNode));

  node->balance = 0;
  node->left = NULL;
  node->right = NULL;
  node->left_child = FALSE;
  node->right_child = FALSE;
  node->key = key;
  node->value = value;

  return node;
}

/**
 * g_tree_new:
 * @key_compare_func: the function used to order the nodes in the #GTree.
 *   It should return values similar to the standard strcmp() function -
 *   0 if the two arguments are equal, a negative value if the first argument 
 *   comes before the second, or a positive value if the first argument comes 
 *   after the second.
 *
 * Creates a new #GTree.
 *
 * Return value: a new #GTree.
 **/
GTree*
g_tree_new (GCompareFunc key_compare_func)
{
  if (key_compare_func == NULL) {
    return NULL;
  }

  return g_tree_new_full ((GCompareDataFunc) key_compare_func, NULL,
                          NULL, NULL);
}

/**
 * g_tree_new_with_data:
 * @key_compare_func: qsort()-style comparison function.
 * @key_compare_data: data to pass to comparison function.
 *
 * Creates a new #GTree with a comparison function that accepts user data.
 * See g_tree_new() for more details.
 *
 * Return value: a new #GTree.
 **/
GTree*
g_tree_new_with_data (GCompareDataFunc key_compare_func,
          gpointer         key_compare_data)
{
  if (key_compare_func == NULL) {
    return NULL;
  }

  return g_tree_new_full (key_compare_func, key_compare_data,
        NULL, NULL);
}

/**
 * g_tree_new_full:
 * @key_compare_func: qsort()-style comparison function.
 * @key_compare_data: data to pass to comparison function.
 * @key_destroy_func: a function to free the memory allocated for the key
 *   used when removing the entry from the #GTree or %NULL if you don't
 *   want to supply such a function.
 * @value_destroy_func: a function to free the memory allocated for the
 *   value used when removing the entry from the #GTree or %NULL if you
 *   don't want to supply such a function.
 *
 * Creates a new #GTree like g_tree_new() and allows to specify functions
 * to free the memory allocated for the key and value that get called when
 * removing the entry from the #GTree.
 *
 * Return value: a new #GTree.
 **/
GTree*
g_tree_new_full (GCompareDataFunc key_compare_func,
     gpointer         key_compare_data,
                 GDestroyNotify   key_destroy_func,
     GDestroyNotify   value_destroy_func)
{
  GTree *tree;

  if (key_compare_func == NULL) {
    return NULL;
  }

  tree = malloc(sizeof(GTree));
  tree->root               = NULL;
  tree->key_compare        = key_compare_func;
  tree->key_destroy_func   = key_destroy_func;
  tree->value_destroy_func = value_destroy_func;
  tree->key_compare_data   = key_compare_data;
  tree->nnodes             = 0;
  tree->ref_count          = 1;

  return tree;
}

static inline GTreeNode *
g_tree_first_node (GTree *tree)
{
  GTreeNode *tmp;

  if (!tree->root)
    return NULL;

  tmp = tree->root;

  while (tmp->left_child)
    tmp = tmp->left;

  return tmp;
}

static inline GTreeNode *
g_tree_node_previous (GTreeNode *node)
{
  GTreeNode *tmp;

  tmp = node->left;

  if (node->left_child)
    while (tmp->right_child)
      tmp = tmp->right;

  return tmp;
}

static inline GTreeNode *
g_tree_node_next (GTreeNode *node)
{
  GTreeNode *tmp;

  tmp = node->right;

  if (node->right_child)
    while (tmp->left_child)
      tmp = tmp->left;

  return tmp;
}

static void
g_tree_remove_all (GTree *tree)
{
  GTreeNode *node;
  GTreeNode *next;

  if (tree == NULL) {
    return;
  }

  node = g_tree_first_node (tree);

  while (node)
    {
      next = g_tree_node_next (node);

      if (tree->key_destroy_func)
  tree->key_destroy_func (node->key);
      if (tree->value_destroy_func)
  tree->value_destroy_func (node->value);
      free (node);

      node = next;
    }

  tree->root = NULL;
  tree->nnodes = 0;
}

/**
 * g_tree_ref:
 * @tree: a #GTree.
 *
 * Increments the reference count of @tree by one.  It is safe to call
 * this function from any thread.
 *
 * Return value: the passed in #GTree.
 *
 * Since: 2.22
 **/
GTree *
g_tree_ref (GTree *tree)
{
  if (tree == NULL) {
    return NULL;
  }

  atomic_inc(&tree->ref_count);

  return tree;
}

/**
 * g_tree_unref:
 * @tree: a #GTree.
 *
 * Decrements the reference count of @tree by one.  If the reference count
 * drops to 0, all keys and values will be destroyed (if destroy
 * functions were specified) and all memory allocated by @tree will be
 * released.
 *
 * It is safe to call this function from any thread.
 *
 * Since: 2.22
 **/
void
g_tree_unref (GTree *tree)
{
  if (tree == NULL) {
    return;
  }

  if (atomic_dec_fetch (&tree->ref_count) == 0)
    {
      g_tree_remove_all (tree);
      free(tree);
    }
}

/**
 * g_tree_destroy:
 * @tree: a #GTree.
 *
 * Removes all keys and values from the #GTree and decreases its
 * reference count by one. If keys and/or values are dynamically
 * allocated, you should either free them first or create the #GTree
 * using g_tree_new_full().  In the latter case the destroy functions
 * you supplied will be called on all keys and values before destroying
 * the #GTree.
 **/
void
g_tree_destroy (GTree *tree)
{
  if (tree == NULL) {
    return;
  }

  g_tree_remove_all (tree);
  g_tree_unref (tree);
}

/**
 * g_tree_insert:
 * @tree: a #GTree.
 * @key: the key to insert.
 * @value: the value corresponding to the key.
 *
 * Inserts a key/value pair into a #GTree. If the given key already exists 
 * in the #GTree its corresponding value is set to the new value. If you 
 * supplied a value_destroy_func when creating the #GTree, the old value is 
 * freed using that function. If you supplied a @key_destroy_func when 
 * creating the #GTree, the passed key is freed using that function.
 *
 * The tree is automatically 'balanced' as new key/value pairs are added,
 * so that the distance from the root to every leaf is as small as possible.
 **/
void
g_tree_insert (GTree    *tree,
         gpointer  key,
         gpointer  value)
{
  if (tree == NULL) {
    return;
  }

  g_tree_insert_internal (tree, key, value, FALSE);
}

/**
 * g_tree_replace:
 * @tree: a #GTree.
 * @key: the key to insert.
 * @value: the value corresponding to the key.
 *
 * Inserts a new key and value into a #GTree similar to g_tree_insert().
 * The difference is that if the key already exists in the #GTree, it gets
 * replaced by the new key. If you supplied a @value_destroy_func when
 * creating the #GTree, the old value is freed using that function. If you
 * supplied a @key_destroy_func when creating the #GTree, the old key is
 * freed using that function.
 *
 * The tree is automatically 'balanced' as new key/value pairs are added,
 * so that the distance from the root to every leaf is as small as possible.
 **/
void
g_tree_replace (GTree    *tree,
    gpointer  key,
    gpointer  value)
{
  if (tree == NULL) {
    return;
  }

  g_tree_insert_internal (tree, key, value, TRUE);
}

/* internal insert routine */
static void
g_tree_insert_internal (GTree    *tree,
                        gpointer  key,
                        gpointer  value,
                        gboolean  replace)
{
  GTreeNode *node;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;

  if (tree == NULL) {
    return;
  }

  if (!tree->root)
    {
      tree->root = g_tree_node_new (key, value);
      tree->nnodes++;
      return;
    }

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1)
    {
      int cmp = tree->key_compare (key, node->key, tree->key_compare_data);

      if (cmp == 0)
        {
          if (tree->value_destroy_func)
            tree->value_destroy_func (node->value);

          node->value = value;

          if (replace)
            {
              if (tree->key_destroy_func)
                tree->key_destroy_func (node->key);

              node->key = key;
            }
          else
            {
              /* free the passed key */
              if (tree->key_destroy_func)
                tree->key_destroy_func (key);
            }

          return;
        }
      else if (cmp < 0)
        {
          if (node->left_child)
            {
              path[idx++] = node;
              node = node->left;
            }
          else
            {
              GTreeNode *child = g_tree_node_new (key, value);

              child->left = node->left;
              child->right = node;
              node->left = child;
              node->left_child = TRUE;
              node->balance -= 1;

        tree->nnodes++;

              break;
            }
        }
      else
        {
          if (node->right_child)
            {
              path[idx++] = node;
              node = node->right;
            }
          else
            {
              GTreeNode *child = g_tree_node_new (key, value);

              child->right = node->right;
              child->left = node;
              node->right = child;
              node->right_child = TRUE;
              node->balance += 1;

        tree->nnodes++;

              break;
            }
        }
    }

  /* restore balance. This is the goodness of a non-recursive
     implementation, when we are done with balancing we 'break'
     the loop and we are done. */
  while (1)
    {
      GTreeNode *bparent = path[--idx];
      gboolean left_node = (bparent && node == bparent->left);
      g_assert (!bparent || bparent->left == node || bparent->right == node);

      if (node->balance < -1 || node->balance > 1)
        {
          node = g_tree_node_balance (node);
          if (bparent == NULL)
            tree->root = node;
          else if (left_node)
            bparent->left = node;
          else
            bparent->right = node;
        }

      if (node->balance == 0 || bparent == NULL)
        break;

      if (left_node)
        bparent->balance -= 1;
      else
        bparent->balance += 1;

      node = bparent;
    }
}

static gboolean
g_tree_remove_internal (GTree         *tree,
                        gconstpointer  key,
                        gboolean       steal)
{
  GTreeNode *node, *parent, *balance;
  GTreeNode *path[MAX_GTREE_HEIGHT];
  int idx;
  gboolean left_node;

  if (tree == NULL) {
    return FALSE;
  }

  if (!tree->root)
    return FALSE;

  idx = 0;
  path[idx++] = NULL;
  node = tree->root;

  while (1)
    {
      int cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      
      if (cmp == 0)
        break;
      else if (cmp < 0)
        {
          if (!node->left_child)
            return FALSE;
    
    path[idx++] = node;
    node = node->left;
        }
      else
        {
          if (!node->right_child)
            return FALSE;
    
    path[idx++] = node;
    node = node->right;
        }
    }

  /* the following code is almost equal to g_tree_remove_node,
     except that we do not have to call g_tree_node_parent. */
  balance = parent = path[--idx];
  g_assert (!parent || parent->left == node || parent->right == node);
  left_node = (parent && node == parent->left);

  if (!node->left_child)
    {
      if (!node->right_child)
        {
          if (!parent)
            tree->root = NULL;
          else if (left_node)
            {
              parent->left_child = FALSE;
              parent->left = node->left;
              parent->balance += 1;
            }
          else
            {
              parent->right_child = FALSE;
              parent->right = node->right;
              parent->balance -= 1;
            }
        }
      else /* node has a right child */
        {
          GTreeNode *tmp = g_tree_node_next (node);
    tmp->left = node->left;

          if (!parent)
            tree->root = node->right;
          else if (left_node)
            {
              parent->left = node->right;
              parent->balance += 1;
            }
          else
            {
              parent->right = node->right;
              parent->balance -= 1;
            }
        }
    }
  else /* node has a left child */
    {
      if (!node->right_child)
        {
          GTreeNode *tmp = g_tree_node_previous (node);
          tmp->right = node->right;
    
          if (parent == NULL)
            tree->root = node->left;
          else if (left_node)
            {
              parent->left = node->left;
              parent->balance += 1;
            }
          else
            {
              parent->right = node->left;
              parent->balance -= 1;
            }
        }
      else /* node has a both children (pant, pant!) */
        {
    GTreeNode *prev = node->left;
    GTreeNode *next = node->right;
    GTreeNode *nextp = node;
    int old_idx = idx + 1;
    idx++;
    
    /* path[idx] == parent */
    /* find the immediately next node (and its parent) */
    while (next->left_child)
            {
        path[++idx] = nextp = next;
        next = next->left;
            }
    
    path[old_idx] = next;
    balance = path[idx];
    
    /* remove 'next' from the tree */
    if (nextp != node)
      {
        if (next->right_child)
    nextp->left = next->right;
        else
    nextp->left_child = FALSE;
        nextp->balance += 1;
        
        next->right_child = TRUE;
        next->right = node->right;
      }
    else
      node->balance -= 1;
      
    /* set the prev to point to the right place */
    while (prev->right_child)
      prev = prev->right;
    prev->right = next;
      
    /* prepare 'next' to replace 'node' */
    next->left_child = TRUE;
    next->left = node->left;
    next->balance = node->balance;
    
    if (!parent)
      tree->root = next;
    else if (left_node)
      parent->left = next;
    else
      parent->right = next;
        }
    }
  
  /* restore balance */
  if (balance)
    while (1)
      {
  GTreeNode *bparent = path[--idx];
  g_assert (!bparent || bparent->left == balance || bparent->right == balance);
  left_node = (bparent && balance == bparent->left);
            
  if(balance->balance < -1 || balance->balance > 1)
    {
      balance = g_tree_node_balance (balance);
      if (!bparent)
        tree->root = balance;
      else if (left_node)
        bparent->left = balance;
      else
        bparent->right = balance;
    }
  
  if (balance->balance != 0 || !bparent)
    break;
  
  if (left_node)
    bparent->balance += 1;
  else
    bparent->balance -= 1;
  
  balance = bparent;
      }
  
  if (!steal)
    {
      if (tree->key_destroy_func)
        tree->key_destroy_func (node->key);
      if (tree->value_destroy_func)
        tree->value_destroy_func (node->value);
    }

  free(node);

  tree->nnodes--;

  return TRUE;
}

/**
 * g_tree_remove:
 * @tree: a #GTree.
 * @key: the key to remove.
 *
 * Removes a key/value pair from a #GTree.
 *
 * If the #GTree was created using g_tree_new_full(), the key and value
 * are freed using the supplied destroy functions, otherwise you have to
 * make sure that any dynamically allocated values are freed yourself.
 * If the key does not exist in the #GTree, the function does nothing.
 *
 * Returns: %TRUE if the key was found (prior to 2.8, this function returned
 *   nothing)
 **/
gboolean
g_tree_remove (GTree         *tree,
         gconstpointer  key)
{
  gboolean removed;

  if (tree == NULL) {
    return FALSE;
  }

  removed = g_tree_remove_internal (tree, key, FALSE);

  return removed;
}

/**
 * g_tree_steal:
 * @tree: a #GTree.
 * @key: the key to remove.
 *
 * Removes a key and its associated value from a #GTree without calling
 * the key and value destroy functions.
 *
 * If the key does not exist in the #GTree, the function does nothing.
 *
 * Returns: %TRUE if the key was found (prior to 2.8, this function returned
 *    nothing)
 **/
gboolean
g_tree_steal (GTree         *tree,
              gconstpointer  key)
{
  gboolean removed;

  if (tree == NULL) {
    return FALSE;
  }

  removed = g_tree_remove_internal (tree, key, TRUE);

  return removed;
}

/**
 * g_tree_lookup:
 * @tree: a #GTree.
 * @key: the key to look up.
 *
 * Gets the value corresponding to the given key. Since a #GTree is
 * automatically balanced as key/value pairs are added, key lookup is very
 * fast.
 *
 * Return value: the value corresponding to the key, or %NULL if the key was
 * not found.
 **/
gpointer
g_tree_lookup (GTree         *tree,
         gconstpointer  key)
{
  GTreeNode *node;

  if (tree == NULL) {
    return NULL;
  }

  node = g_tree_find_node (tree, key);

  return node ? node->value : NULL;
}

/**
 * g_tree_lookup_extended:
 * @tree: a #GTree.
 * @lookup_key: the key to look up.
 * @orig_key: returns the original key.
 * @value: returns the value associated with the key.
 *
 * Looks up a key in the #GTree, returning the original key and the
 * associated value and a #gboolean which is %TRUE if the key was found. This
 * is useful if you need to free the memory allocated for the original key,
 * for example before calling g_tree_remove().
 *
 * Return value: %TRUE if the key was found in the #GTree.
 **/
gboolean
g_tree_lookup_extended (GTree         *tree,
                        gconstpointer  lookup_key,
                        gpointer      *orig_key,
                        gpointer      *value)
{
  GTreeNode *node;

  if (tree == NULL) {
    return FALSE;
  }

  node = g_tree_find_node (tree, lookup_key);

  if (node)
    {
      if (orig_key)
        *orig_key = node->key;
      if (value)
        *value = node->value;
      return TRUE;
    }
  else
    return FALSE;
}

/**
 * g_tree_foreach:
 * @tree: a #GTree.
 * @func: the function to call for each node visited. If this function
 *   returns %TRUE, the traversal is stopped.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each of the key/value pairs in the #GTree.
 * The function is passed the key and value of each pair, and the given
 * @data parameter. The tree is traversed in sorted order.
 *
 * The tree may not be modified while iterating over it (you can't
 * add/remove items). To remove all items matching a predicate, you need
 * to add each item to a list in your #GTraverseFunc as you walk over
 * the tree, then walk the list and remove each item.
 **/
void
g_tree_foreach (GTree         *tree,
                GTraverseFunc  func,
                gpointer       user_data)
{
  GTreeNode *node;

  if (tree == NULL) {
    return;
  }

  if (!tree->root)
    return;

  node = g_tree_first_node (tree);

  while (node)
    {
      if ((*func) (node->key, node->value, user_data))
  break;

      node = g_tree_node_next (node);
    }
}

/**
 * g_tree_traverse:
 * @tree: a #GTree.
 * @traverse_func: the function to call for each node visited. If this
 *   function returns %TRUE, the traversal is stopped.
 * @traverse_type: the order in which nodes are visited, one of %G_IN_ORDER,
 *   %G_PRE_ORDER and %G_POST_ORDER.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each node in the #GTree.
 *
 * Deprecated:2.2: The order of a balanced tree is somewhat arbitrary. If you
 * just want to visit all nodes in sorted order, use g_tree_foreach()
 * instead. If you really need to visit nodes in a different order, consider
 * using an <link linkend="glib-N-ary-Trees">N-ary Tree</link>.
 **/
/**
 * GTraverseFunc:
 * @key: a key of a #GTree node.
 * @value: the value corresponding to the key.
 * @data: user data passed to g_tree_traverse().
 * @Returns: %TRUE to stop the traversal.
 *
 * Specifies the type of function passed to g_tree_traverse(). It is
 * passed the key and value of each node, together with the @user_data
 * parameter passed to g_tree_traverse(). If the function returns
 * %TRUE, the traversal is stopped.
 **/
/**
 * GTraverseType:
 * @G_IN_ORDER: vists a node's left child first, then the node itself,
 *              then its right child. This is the one to use if you
 *              want the output sorted according to the compare
 *              function.
 * @G_PRE_ORDER: visits a node, then its children.
 * @G_POST_ORDER: visits the node's children, then the node itself.
 * @G_LEVEL_ORDER: is not implemented for <link
 *                 linkend="glib-Balanced-Binary-Trees">Balanced Binary
 *                 Trees</link>.  For <link
 *                 linkend="glib-N-ary-Trees">N-ary Trees</link>, it
 *                 vists the root node first, then its children, then
 *                 its grandchildren, and so on. Note that this is less
 *                 efficient than the other orders.
 *
 * Specifies the type of traveral performed by g_tree_traverse(),
 * g_node_traverse() and g_node_find().
 **/
void
g_tree_traverse (GTree         *tree,
     GTraverseFunc  traverse_func,
     GTraverseType  traverse_type,
     gpointer       user_data)
{
  if (tree == NULL) {
    return;
  }

  if (!tree->root)
    return;

  switch (traverse_type)
    {
    case G_PRE_ORDER:
      g_tree_node_pre_order (tree->root, traverse_func, user_data);
      break;

    case G_IN_ORDER:
      g_tree_node_in_order (tree->root, traverse_func, user_data);
      break;

    case G_POST_ORDER:
      g_tree_node_post_order (tree->root, traverse_func, user_data);
      break;

    case G_LEVEL_ORDER:
      //g_warning ("g_tree_traverse(): traverse type G_LEVEL_ORDER isn't implemented.");
      break;
    }
}

/**
 * g_tree_search:
 * @tree: a #GTree.
 * @search_func: a function used to search the #GTree. 
 * @user_data: the data passed as the second argument to the @search_func 
 * function.
 * 
 * Searches a #GTree using @search_func.
 *
 * The @search_func is called with a pointer to the key of a key/value pair in 
 * the tree, and the passed in @user_data. If @search_func returns 0 for a 
 * key/value pair, then g_tree_search_func() will return the value of that 
 * pair. If @search_func returns -1,  searching will proceed among the 
 * key/value pairs that have a smaller key; if @search_func returns 1, 
 * searching will proceed among the key/value pairs that have a larger key.
 *
 * Return value: the value corresponding to the found key, or %NULL if the key 
 * was not found.
 **/
gpointer
g_tree_search (GTree         *tree,
         GCompareFunc   search_func,
         gconstpointer  user_data)
{
  if (tree == NULL) {
    return NULL;
  }

  if (tree->root)
    return g_tree_node_search (tree->root, search_func, user_data);
  else
    return NULL;
}

/**
 * g_tree_height:
 * @tree: a #GTree.
 *
 * Gets the height of a #GTree.
 *
 * If the #GTree contains no nodes, the height is 0.
 * If the #GTree contains only one root node the height is 1.
 * If the root node has children the height is 2, etc.
 *
 * Return value: the height of the #GTree.
 **/
gint
g_tree_height (GTree *tree)
{
  GTreeNode *node;
  gint height;

  if (tree == NULL) {
    return 0;
  }

  if (!tree->root)
    return 0;

  height = 0;
  node = tree->root;

  while (1)
    {
      height += 1 + MAX(node->balance, 0);

      if (!node->left_child)
  return height;

      node = node->left;
    }
}

/**
 * g_tree_nnodes:
 * @tree: a #GTree.
 *
 * Gets the number of nodes in a #GTree.
 *
 * Return value: the number of nodes in the #GTree.
 **/
gint
g_tree_nnodes (GTree *tree)
{
  if (tree == NULL) {
    return 0;
  }

  return tree->nnodes;
}

static GTreeNode*
g_tree_node_balance (GTreeNode *node)
{
  if (node->balance < -1)
    {
      if (node->left->balance > 0)
  node->left = g_tree_node_rotate_left (node->left);
      node = g_tree_node_rotate_right (node);
    }
  else if (node->balance > 1)
    {
      if (node->right->balance < 0)
  node->right = g_tree_node_rotate_right (node->right);
      node = g_tree_node_rotate_left (node);
    }

  return node;
}

static GTreeNode *
g_tree_find_node (GTree        *tree,
      gconstpointer key)
{
  GTreeNode *node;
  gint cmp;

  node = tree->root;
  if (!node)
    return NULL;

  while (1)
    {
      cmp = tree->key_compare (key, node->key, tree->key_compare_data);
      if (cmp == 0)
  return node;
      else if (cmp < 0)
  {
    if (!node->left_child)
      return NULL;

    node = node->left;
  }
      else
  {
    if (!node->right_child)
      return NULL;

    node = node->right;
  }
    }
}

static gint
g_tree_node_pre_order (GTreeNode     *node,
           GTraverseFunc  traverse_func,
           gpointer       data)
{
  if ((*traverse_func) (node->key, node->value, data))
    return TRUE;

  if (node->left_child)
    {
      if (g_tree_node_pre_order (node->left, traverse_func, data))
  return TRUE;
    }

  if (node->right_child)
    {
      if (g_tree_node_pre_order (node->right, traverse_func, data))
  return TRUE;
    }

  return FALSE;
}

static gint
g_tree_node_in_order (GTreeNode     *node,
          GTraverseFunc  traverse_func,
          gpointer       data)
{
  if (node->left_child)
    {
      if (g_tree_node_in_order (node->left, traverse_func, data))
  return TRUE;
    }

  if ((*traverse_func) (node->key, node->value, data))
    return TRUE;

  if (node->right_child)
    {
      if (g_tree_node_in_order (node->right, traverse_func, data))
  return TRUE;
    }

  return FALSE;
}

static gint
g_tree_node_post_order (GTreeNode     *node,
      GTraverseFunc  traverse_func,
      gpointer       data)
{
  if (node->left_child)
    {
      if (g_tree_node_post_order (node->left, traverse_func, data))
  return TRUE;
    }

  if (node->right_child)
    {
      if (g_tree_node_post_order (node->right, traverse_func, data))
  return TRUE;
    }

  if ((*traverse_func) (node->key, node->value, data))
    return TRUE;

  return FALSE;
}

static gpointer
g_tree_node_search (GTreeNode     *node,
        GCompareFunc   search_func,
        gconstpointer  data)
{
  gint dir;

  if (!node)
    return NULL;

  while (1)
    {
      dir = (* search_func) (node->key, data);
      if (dir == 0)
  return node->value;
      else if (dir < 0)
  {
    if (!node->left_child)
      return NULL;

    node = node->left;
  }
      else
  {
    if (!node->right_child)
      return NULL;

    node = node->right;
  }
    }
}

static GTreeNode*
g_tree_node_rotate_left (GTreeNode *node)
{
  GTreeNode *right;
  gint a_bal;
  gint b_bal;

  right = node->right;

  if (right->left_child)
    node->right = right->left;
  else
    {
      node->right_child = FALSE;
      node->right = right;
      right->left_child = TRUE;
    }
  right->left = node;

  a_bal = node->balance;
  b_bal = right->balance;

  if (b_bal <= 0)
    {
      if (a_bal >= 1)
  right->balance = b_bal - 1;
      else
  right->balance = a_bal + b_bal - 2;
      node->balance = a_bal - 1;
    }
  else
    {
      if (a_bal <= b_bal)
  right->balance = a_bal - 2;
      else
  right->balance = b_bal - 1;
      node->balance = a_bal - b_bal - 1;
    }

  return right;
}

static GTreeNode*
g_tree_node_rotate_right (GTreeNode *node)
{
  GTreeNode *left;
  gint a_bal;
  gint b_bal;

  left = node->left;

  if (left->right_child)
    node->left = left->right;
  else
    {
      node->left_child = FALSE;
      node->left = left;
      left->right_child = TRUE;
    }
  left->right = node;

  a_bal = node->balance;
  b_bal = left->balance;

  if (b_bal <= 0)
    {
      if (b_bal > a_bal)
  left->balance = b_bal + 1;
      else
  left->balance = a_bal + 2;
      node->balance = a_bal - b_bal + 1;
    }
  else
    {
      if (a_bal <= -1)
  left->balance = b_bal + 1;
      else
  left->balance = a_bal + b_bal + 2;
      node->balance = a_bal + 1;
    }

  return left;
}

/* END of GTree related functions*/

/* general g_XXX substitutes */

void g_free(gpointer ptr)
{
   free(ptr);
}

gpointer g_malloc(size_t size)
{
   void *res;
    if (size == 0) return NULL;
   res = malloc(size);
   if (res == NULL) exit(1);
   return res;
}

gpointer g_malloc0(size_t size)
{
   void *res;
   if (size == 0) return NULL;
   res = calloc(size, 1);
   if (res == NULL) exit(1);
   return res;
}

gpointer g_try_malloc0(size_t size)
{
   if (size == 0) return NULL;
   return calloc(size, 1);
}

gpointer g_realloc(gpointer ptr, size_t size)
{
   void *res;
   if (size == 0) {
      free(ptr);
      return NULL;
   }
   res = realloc(ptr, size);
   if (res == NULL) exit(1);
   return res;
}

char *g_strdup(const char *str)
{
#ifdef _MSC_VER
    return str ? _strdup(str) : NULL;
#else
    return str ? strdup(str) : NULL;
#endif
}

char *g_strdup_printf(const char *format, ...)
{
   va_list ap;
   char *res;
   va_start(ap, format);
   res = g_strdup_vprintf(format, ap);
   va_end(ap);
   return res;
}

char *g_strdup_vprintf(const char *format, va_list ap)
{
   char *str_res = NULL;
#ifdef _MSC_VER
   int len = _vscprintf(format, ap);
   if( len < 0 )
       return NULL;
   str_res = (char *)malloc(len+1);
   if(str_res==NULL)
       return NULL;
   vsnprintf(str_res, len+1, format, ap);
#else
   vasprintf(&str_res, format, ap);
#endif
   return str_res;
}

char *g_strndup(const char *str, size_t n)
{
   /* try to mimic glib's g_strndup */
   char *res = calloc(n + 1, 1);
   strncpy(res, str, n);
   return res;
}

void g_strfreev(char **str_array)
{
   char **p = str_array;
   if (p) {
      while (*p) {
         free(*p++);
      }
   }
   free(str_array);
}

gpointer g_memdup(gconstpointer mem, size_t byte_size)
{
   if (mem) {
      void *res = g_malloc(byte_size);
      memcpy(res, mem, byte_size);
      return res;
   }
   return NULL; 
}

gpointer g_new_(size_t sz, size_t n_structs)
{
   size_t need = sz * n_structs;
   if ((need / sz) != n_structs) return NULL;
   return g_malloc(need);
}

gpointer g_new0_(size_t sz, size_t n_structs)
{
   size_t need = sz * n_structs;
   if ((need / sz) != n_structs) return NULL;
   return g_malloc0(need);
}

gpointer g_renew_(size_t sz, gpointer mem, size_t n_structs)
{
   size_t need = sz * n_structs;
   if ((need / sz) != n_structs) return NULL;
   return g_realloc(mem, need);
}

/**
 * g_strconcat:
 * @string1: the first string to add, which must not be %NULL
 * @Varargs: a %NULL-terminated list of strings to append to the string
 *
 * Concatenates all of the given strings into one long string.
 * The returned string should be freed with g_free() when no longer needed.
 *
 * Note that this function is usually not the right function to use to
 * assemble a translated message from pieces, since proper translation
 * often requires the pieces to be reordered.
 *
 * <warning><para>The variable argument list <emphasis>must</emphasis> end
 * with %NULL. If you forget the %NULL, g_strconcat() will start appending
 * random memory junk to your string.</para></warning>
 *
 * Returns: a newly-allocated string containing all the string arguments
 */
gchar* g_strconcat (const gchar *string1, ...)
{
   va_list ap;
   char *res;
   size_t sz = strlen(string1);
   va_start(ap, string1);
   while (1) {
      char *arg = va_arg(ap, char*);
      if (arg == NULL) break;
      sz += strlen(arg);
   }
   va_end(ap);
   res = g_malloc(sz + 1);
   strcpy(res, string1);
   va_start(ap, string1);
   while (1) {
      char *arg = va_arg(ap, char*);
      if (arg == NULL) break;
      strcat(res, arg);
   }
   va_end(ap);
   return res;
}

/**
 * g_strsplit:
 * @string: a string to split.
 * @delimiter: a string which specifies the places at which to split the string.
 *     The delimiter is not included in any of the resulting strings, unless
 *     @max_tokens is reached.
 * @max_tokens: the maximum number of pieces to split @string into. If this is
 *              less than 1, the string is split completely.
 *
 * Splits a string into a maximum of @max_tokens pieces, using the given
 * @delimiter. If @max_tokens is reached, the remainder of @string is appended
 * to the last token.
 *
 * As a special case, the result of splitting the empty string "" is an empty
 * vector, not a vector containing a single string. The reason for this
 * special case is that being able to represent a empty vector is typically
 * more useful than consistent handling of empty elements. If you do need
 * to represent empty elements, you'll need to check for the empty string
 * before calling g_strsplit().
 *
 * Return value: a newly-allocated %NULL-terminated array of strings. Use
 *    g_strfreev() to free it.
 **/
gchar** g_strsplit (const gchar *string,
            const gchar *delimiter,
            gint         max_tokens)
{
  GSList *string_list = NULL, *slist;
  gchar **str_array, *s;
  guint n = 0;
  const gchar *remainder;

  if (string == NULL) return NULL;
  if (delimiter == NULL) return NULL;
  if (delimiter[0] == '\0') return NULL;

  if (max_tokens < 1)
    max_tokens = G_MAXINT;

  remainder = string;
  s = strstr (remainder, delimiter);
  if (s)
    {
      gsize delimiter_len = strlen (delimiter);

      while (--max_tokens && s)
        {
          gsize len;

          len = s - remainder;
          string_list = g_slist_prepend (string_list,
                                         g_strndup (remainder, len));
          n++;
          remainder = s + delimiter_len;
          s = strstr (remainder, delimiter);
        }
    }
  if (*string)
    {
      n++;
      string_list = g_slist_prepend (string_list, g_strdup (remainder));
    }

  str_array = g_new (gchar*, n + 1);

  str_array[n--] = NULL;
  for (slist = string_list; slist; slist = slist->next)
    str_array[n--] = slist->data;

  g_slist_free (string_list);

  return str_array;
}

static const char base64_alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static gsize g_base64_encode_step(const guchar *in, gsize len,
                                  gboolean break_lines,
                                  gchar *out, gint *state,
                                  gint *save)
{
  char *outptr;
  const guchar *inptr;

  if (in == NULL || out == NULL || state == NULL || save == NULL) {
    return 0;
  }

  if (len <= 0) {
    return 0;
  }

  inptr = in;
  outptr = out;

  if (len + ((char *) save) [0] > 2)
  {
    const guchar *inend = in + len - 2;
    int c1, c2, c3;
    int already;

    already = *state;

    switch (((char *) save)[0])
    {
    case 1:
      c1 = ((unsigned char *) save)[1];
      goto skip1;
    case 2:
      c1 = ((unsigned char *) save)[1];
      c2 = ((unsigned char *) save)[2];
      goto skip2;
    }

    /*
     * yes, we jump into the loop, no i'm not going to change it,
     * it's beautiful!
     */
    while (inptr < inend)
    {
      c1 = *inptr++;
    skip1:
      c2 = *inptr++;
    skip2:
      c3 = *inptr++;
      *outptr++ = base64_alphabet[c1 >> 2];
      *outptr++ = base64_alphabet[c2 >> 4 | ((c1 & 0x3) << 4)];
      *outptr++ = base64_alphabet[((c2 & 0x0f) << 2) | (c3 >> 6)];
      *outptr++ = base64_alphabet[c3 & 0x3f];
      /* this is a bit ugly ... */
      if (break_lines && (++already) >= 19)
      {
          *outptr++ = '\n';
          already = 0;
      }
    }

    ((char *)save)[0] = 0;
    len = 2 - (inptr - inend);
    *state = already;
  }

  if (len > 0)
  {
      char *saveout;

      /* points to the slot for the next char to save */
      saveout = & (((char *)save)[1]) + ((char *)save)[0];

      /* len can only be 0 1 or 2 */
      switch (len)
      {
      case 2: *saveout++ = *inptr++;
      case 1: *saveout++ = *inptr++;
      }
      ((char *) save)[0] += len;
  }

  return outptr - out;
}

gsize g_base64_encode_close(gboolean break_lines, gchar *out,
                            gint *state, gint *save)
{
  int c1, c2;
  char *outptr = out;

  if (out == NULL || state == NULL || save == NULL) {
    return 0;
  }

  c1 = ((unsigned char *) save)[1];
  c2 = ((unsigned char *) save)[2];

  switch (((char *) save)[0])
  {
  case 2:
    outptr[2] = base64_alphabet[((c2 &0x0f) << 2)];
    g_assert(outptr[2] != 0);
    goto skip;
  case 1:
    outptr[2] = '=';
    c2 = 0;  /* saved state here is not relevant */
  skip:
    outptr[0] = base64_alphabet[c1 >> 2 ];
    outptr[1] = base64_alphabet[c2 >> 4 | ((c1 & 0x3) << 4)];
    outptr[3] = '=';
    outptr += 4;
    break;
  }
  if (break_lines) {
    *outptr++ = '\n';
  }

  *save = 0;
  *state = 0;

  return outptr - out;
}

gchar *g_base64_encode(const guchar *data, gsize len)
{
  gchar *out;
  gint state = 0, outlen;
  gint save = 0;

  if (data == NULL && len != 0) {
    return NULL;
  }

  /* We can use a smaller limit here, since we know the saved state is 0,
     +1 is needed for trailing \0, also check for unlikely integer overflow */
  if (len >= ((SIZE_MAX - 1) / 4 - 1) * 3) {
    //g_error("%s: input too large for Base64 encoding (%"G_GSIZE_FORMAT" chars)",
    //    G_STRLOC, len);
    return NULL;
  }

  out = g_malloc((len / 3 + 1) * 4 + 1);

  outlen = g_base64_encode_step(data, len, FALSE, out, &state, &save);
  outlen += g_base64_encode_close(FALSE, out + outlen, &state, &save);
  out[outlen] = '\0';

  return (gchar *) out;
}

static const unsigned char mime_base64_rank[256] = {
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
  255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

static gsize g_base64_decode_step(const gchar  *in, gsize len,
                                  guchar *out, gint *state,
                                  guint *save)
{
  const guchar *inptr;
  guchar *outptr;
  const guchar *inend;
  guchar c, rank;
  guchar last[2];
  unsigned int v;
  int i;

  if (in == NULL || out == NULL || state == NULL || save == NULL) {
    return 0;
  }

  if (len <= 0) {
    return 0;
  }

  inend = (const guchar *)in+len;
  outptr = out;

  /* convert 4 base64 bytes to 3 normal bytes */
  v = *save;
  i = *state;

  last[0] = last[1] = 0;

  /* we use the sign in the state to determine if we got a padding character
     in the previous sequence */
  if (i < 0)
  {
    i = -i;
    last[0] = '=';
  }

  inptr = (const guchar *)in;
  while (inptr < inend)
  {
    c = *inptr++;
    rank = mime_base64_rank[c];
    if (rank != 0xff)
    {
      last[1] = last[0];
      last[0] = c;
      v = (v << 6) | rank;
      i++;
      if (i == 4)
      {
        *outptr++ = v >> 16;
        if (last[1] != '=') {
          *outptr++ = v >> 8;
        }
        if (last[0] != '=') {
          *outptr++ = v;
        }
        i = 0;
      }
    }
  }

  *save = v;
  *state = last[0] == '=' ? -i : i;

  return outptr - out;
}

guchar *g_base64_decode(const gchar *text, gsize *out_len)
{
  guchar *ret;
  gsize input_length;
  gint state = 0;
  guint save = 0;

  if (text == NULL || out_len == NULL) {
    return NULL;
  }

  input_length = strlen(text);

  /* We can use a smaller limit here, since we know the saved state is 0,
     +1 used to avoid calling g_malloc0(0), and hence returning NULL */
  ret = g_malloc0((input_length / 4) * 3 + 1);

  *out_len = g_base64_decode_step(text, input_length, ret, &state, &save);

  return ret;
}

guchar *g_base64_decode_inplace(gchar *text, gsize *out_len)
{
  gint input_length, state = 0;
  guint save = 0;

  if (text == NULL || out_len == NULL) {
    return NULL;
  }

  input_length = strlen(text);

  if (input_length <= 1) {
    return NULL;
  }

  *out_len = g_base64_decode_step(text, input_length, (guchar *) text, &state, &save);

  return (guchar *) text;
}
