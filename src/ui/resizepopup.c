/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Croma resizing-terminal-window feedback */

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

#include <config.h>
#include "resizepopup.h"
#include "util.h"
#include <ctk/ctk.h>
#include <cdk/cdkx.h>

struct _MetaResizePopup
{
  CtkWidget *size_window;
  CtkWidget *size_label;
  Display *display;
  int screen_number;

  int vertical_size;
  int horizontal_size;

  gboolean showing;

  MetaRectangle rect;
};

MetaResizePopup*
meta_ui_resize_popup_new (Display *display,
                          int      screen_number)
{
  MetaResizePopup *popup;

  popup = g_new0 (MetaResizePopup, 1);

  popup->display = display;
  popup->screen_number = screen_number;

  return popup;
}

void
meta_ui_resize_popup_free (MetaResizePopup *popup)
{
  g_return_if_fail (popup != NULL);

  if (popup->size_window)
    ctk_widget_destroy (popup->size_window);

  g_free (popup);
}

static void
ensure_size_window (MetaResizePopup *popup)
{
  CtkWidget *frame;

  if (popup->size_window)
    return;

  popup->size_window = ctk_window_new (CTK_WINDOW_POPUP);

  ctk_window_set_screen (CTK_WINDOW (popup->size_window),
			 cdk_display_get_default_screen (cdk_x11_lookup_xdisplay (popup->display)));

  /* never shrink the size window */
  ctk_window_set_resizable (CTK_WINDOW (popup->size_window),
                            TRUE);

  frame = ctk_frame_new (NULL);
  ctk_frame_set_shadow_type (CTK_FRAME (frame), CTK_SHADOW_OUT);

  ctk_container_add (CTK_CONTAINER (popup->size_window), frame);

  popup->size_label = ctk_label_new ("");
  ctk_widget_set_margin_start (popup->size_label, 3);
  ctk_widget_set_margin_end (popup->size_label, 3);
  ctk_widget_set_margin_top (popup->size_label, 3);
  ctk_widget_set_margin_bottom (popup->size_label, 3);

  ctk_container_add (CTK_CONTAINER (frame), popup->size_label);

  ctk_widget_show_all (frame);
}

static void
update_size_window (MetaResizePopup *popup)
{
  char *str;
  int x, y;
  int width, height;
  int scale;

  g_return_if_fail (popup->size_window != NULL);

  scale = ctk_widget_get_scale_factor (CTK_WIDGET (popup->size_window));
  /* Translators: This represents the size of a window.  The first number is
   * the width of the window and the second is the height.
   */
  str = g_strdup_printf (_("%d x %d"),
                         popup->horizontal_size,
                         popup->vertical_size);

  ctk_label_set_text (CTK_LABEL (popup->size_label), str);

  g_free (str);

  ctk_window_get_size (CTK_WINDOW (popup->size_window), &width, &height);

  x = popup->rect.x + (popup->rect.width - width) / 2;
  y = popup->rect.y + (popup->rect.height - height) / 2;

  if (scale)
    {
      x = x / scale;
      y = y / scale;
    }

  if (ctk_widget_get_realized (popup->size_window))
    {
      /* using move_resize to avoid jumpiness */
      cdk_window_move_resize (ctk_widget_get_window (popup->size_window),
                              x, y,
                              width, height);
    }
  else
    {
      ctk_window_move   (CTK_WINDOW (popup->size_window),
                         x, y);
    }
}

static void
sync_showing (MetaResizePopup *popup)
{
  if (popup->showing)
    {
      if (popup->size_window)
        ctk_widget_show (popup->size_window);

      if (popup->size_window && ctk_widget_get_realized (popup->size_window))
        cdk_window_raise (ctk_widget_get_window(CTK_WIDGET(popup->size_window)));
    }
  else
    {
      if (popup->size_window)
        ctk_widget_hide (popup->size_window);
    }
}

void
meta_ui_resize_popup_set (MetaResizePopup *popup,
                          MetaRectangle    rect,
                          int              base_width,
                          int              base_height,
                          int              width_inc,
                          int              height_inc)
{
  gboolean need_update_size;
  int display_w, display_h;

  g_return_if_fail (popup != NULL);

  need_update_size = FALSE;

  display_w = rect.width - base_width;
  if (width_inc > 0)
    display_w /= width_inc;

  display_h = rect.height - base_height;
  if (height_inc > 0)
    display_h /= height_inc;

  if (!meta_rectangle_equal(&popup->rect, &rect) ||
      display_w != popup->horizontal_size ||
      display_h != popup->vertical_size)
    need_update_size = TRUE;

  popup->rect = rect;
  popup->vertical_size = display_h;
  popup->horizontal_size = display_w;

  if (need_update_size)
    {
      ensure_size_window (popup);
      update_size_window (popup);
    }

  sync_showing (popup);
}

void
meta_ui_resize_popup_set_showing  (MetaResizePopup *popup,
                                   gboolean         showing)
{
  g_return_if_fail (popup != NULL);

  if (showing == popup->showing)
    return;

  popup->showing = !!showing;

  if (popup->showing)
    {
      ensure_size_window (popup);
      update_size_window (popup);
    }

  sync_showing (popup);
}
