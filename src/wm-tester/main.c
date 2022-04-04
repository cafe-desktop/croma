/* WM tester main() */

/*
 * Copyright (C) 2001 Havoc Pennington
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <ctk/ctk.h>
#include <cdk/cdkx.h>

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void set_up_the_evil (void);
static void set_up_icon_windows (void);

static void
usage (void)
{
  g_print ("wm-tester [--evil] [--icon-windows]\n");
  exit (0);
}

int
main (int argc, char **argv)
{
  int i;
  gboolean do_evil;
  gboolean do_icon_windows;

  ctk_init (&argc, &argv);

  do_evil = FALSE;
  do_icon_windows = FALSE;

  i = 1;
  while (i < argc)
    {
      const char *arg = argv[i];

      if (strcmp (arg, "--help") == 0 ||
          strcmp (arg, "-h") == 0 ||
          strcmp (arg, "-?") == 0)
        usage ();
      else if (strcmp (arg, "--evil") == 0)
        do_evil = TRUE;
      else if (strcmp (arg, "--icon-windows") == 0)
        do_icon_windows = TRUE;
      else
        usage ();

      ++i;
    }

  /* Be sure some option was provided */
  if (! (do_evil || do_icon_windows))
    return 1;

  if (do_evil)
    set_up_the_evil ();

  if (do_icon_windows)
    set_up_icon_windows ();

  ctk_main ();

  return 0;
}

static GSList *evil_windows = NULL;

static gint
evil_timeout (gpointer data)
{
  int i;
  int n_windows;
  int len;
  int create_count;
  int destroy_count;

  len = g_slist_length (evil_windows);

  if (len > 35)
    {
      create_count = 2;
      destroy_count = 5;
    }
  else
    {
      create_count = 5;
      destroy_count = 5;
    }

  /* Create some windows */
  n_windows = g_random_int_range (0, create_count);

  i = 0;
  while (i < n_windows)
    {
      CtkWidget *w;
      CtkWidget *c;
      int t;
      CtkWidget *parent;

      w = ctk_window_new (CTK_WINDOW_TOPLEVEL);

      ctk_window_move (CTK_WINDOW (w),
                       g_random_int_range (0,
                                           WidthOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ()))),
                       g_random_int_range (0,
                                           HeightOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ()))));

      parent = NULL;

      /* set transient for random window (may create all kinds of weird cycles) */
      if (len > 0)
        {
          t = g_random_int_range (- (len / 3), len);
          if (t >= 0)
            {
              parent = g_slist_nth_data (evil_windows, t);

              if (parent != NULL)
                ctk_window_set_transient_for (CTK_WINDOW (w), CTK_WINDOW (parent));
            }
        }

      if (parent != NULL)
        c = ctk_button_new_with_label ("Evil Transient!");
      else
        c = ctk_button_new_with_label ("Evil Window!");
      ctk_container_add (CTK_CONTAINER (w), c);

      ctk_widget_show_all (w);

      evil_windows = g_slist_prepend (evil_windows, w);

      ++i;
    }

  /* Destroy some windows */
  if (len > destroy_count)
    {
      n_windows = g_random_int_range (0, destroy_count);
      i = 0;
      while (i < n_windows)
        {
          CtkWidget *w;

          w = g_slist_nth_data (evil_windows,
                                g_random_int_range (0, len));
          if (w)
            {
              --len;
              evil_windows = g_slist_remove (evil_windows, w);
              ctk_widget_destroy (w);
            }

          ++i;
        }
    }

  return TRUE;
}

static void
set_up_the_evil (void)
{
  g_timeout_add (400, evil_timeout, NULL);
}

static void
set_up_icon_windows (void)
{
  int i;
  int n_windows;

  /* Create some windows */
  n_windows = 9;

  i = 0;
  while (i < n_windows)
    {
      CtkWidget *w;
      CtkWidget *c;
      GList *icons;
      GdkPixbuf *pix;
      int size  = 0;

      w = ctk_window_new (CTK_WINDOW_TOPLEVEL);
      c = ctk_button_new_with_label ("Icon window");
      ctk_container_add (CTK_CONTAINER (w), c);

      icons = NULL;

      ctk_icon_size_lookup (CTK_ICON_SIZE_LARGE_TOOLBAR, NULL, &size);
      pix = ctk_icon_theme_load_icon (ctk_icon_theme_get_default (), "ctk-save", size, 0, NULL);
      icons = g_list_append (icons, pix);

      if (i % 2)
        {
          ctk_icon_size_lookup (CTK_ICON_SIZE_DIALOG, NULL, &size);
          pix = ctk_icon_theme_load_icon (ctk_icon_theme_get_default (), "ctk-save", size, 0, NULL);
          icons = g_list_append (icons, pix);
        }

      if (i % 3)
        {
          ctk_icon_size_lookup (CTK_ICON_SIZE_MENU, NULL, &size);
          pix = ctk_icon_theme_load_icon (ctk_icon_theme_get_default (), "ctk-save", size, 0, NULL);
          icons = g_list_append (icons, pix);
        }

      ctk_window_set_icon_list (CTK_WINDOW (w), icons);

      g_list_free_full (icons, g_object_unref);

      ctk_widget_show_all (w);

      ++i;
    }
}
