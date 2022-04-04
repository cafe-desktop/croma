/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Croma fixed tooltip routine */

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
#include <ctk/ctk.h>
#include "fixedtip.h"
#include "ui.h"

/**
 * The floating rectangle.  This is a CtkWindow, and it contains
 * the "label" widget, below.
 */
static CtkWidget *tip = NULL;

/**
 * The actual text that gets displayed.
 */
static CtkWidget *label = NULL;

static GdkScreen *screen = NULL;

static gboolean
draw_handler (CtkWidget *widget,
              cairo_t   *cr)
{
  CtkStyleContext *context;
  gint width;
  gint height;

  if (widget == NULL)
    return FALSE;

  context = ctk_widget_get_style_context (widget);
  ctk_style_context_add_class (context, "tooltip");
  width = ctk_widget_get_allocated_width (widget);
  height = ctk_widget_get_allocated_height (widget);

  ctk_render_background (context, cr, 0, 0, width, height);
  ctk_render_frame (context, cr, 0, 0, width, height);

  return FALSE;
}

void
meta_fixed_tip_show (int root_x, int root_y,
                     const char *markup_text)
{
  gint w;
  gint h;
  GdkMonitor *mon_num;
  GdkRectangle monitor;
  gint screen_right_edge;

  if (tip == NULL)
    {
      GdkVisual *visual;

      tip = ctk_window_new (GTK_WINDOW_POPUP);

      ctk_window_set_type_hint (GTK_WINDOW(tip), GDK_WINDOW_TYPE_HINT_TOOLTIP);
      ctk_style_context_add_class (ctk_widget_get_style_context (tip),
                                   GTK_STYLE_CLASS_TOOLTIP);

      screen = gdk_display_get_default_screen (gdk_display_get_default ());
      visual = gdk_screen_get_rgba_visual (screen);

      ctk_window_set_screen (GTK_WINDOW (tip), screen);

      if (visual != NULL)
        ctk_widget_set_visual (tip, visual);

      ctk_widget_set_app_paintable (tip, TRUE);
      ctk_window_set_resizable (GTK_WINDOW (tip), FALSE);
      g_signal_connect (tip, "draw", G_CALLBACK (draw_handler), NULL);

      label = ctk_label_new (NULL);
      ctk_label_set_line_wrap (GTK_LABEL (label), TRUE);
      ctk_label_set_xalign (GTK_LABEL (label), 0.5);
      ctk_label_set_yalign (GTK_LABEL (label), 0.5);
      ctk_widget_show (label);

      ctk_container_set_border_width (GTK_CONTAINER (tip), 4);
      ctk_container_add (GTK_CONTAINER (tip), label);

      g_signal_connect (tip, "destroy",
			G_CALLBACK (ctk_widget_destroyed), &tip);
    }

  mon_num = gdk_display_get_monitor_at_point (gdk_screen_get_display (screen), root_x, root_y);
  gdk_monitor_get_geometry (mon_num, &monitor);
  screen_right_edge = monitor.x + monitor.width;

  ctk_label_set_markup (GTK_LABEL (label), markup_text);

  ctk_window_get_size (GTK_WINDOW (tip), &w, &h);

  if (meta_ui_get_direction() == META_UI_DIRECTION_RTL)
      root_x = MAX(0, root_x - w);

  if ((root_x + w) > screen_right_edge)
    root_x -= (root_x + w) - screen_right_edge;

  ctk_window_move (GTK_WINDOW (tip), root_x, root_y);

  ctk_widget_show (tip);
}

void
meta_fixed_tip_hide (void)
{
  if (tip)
    {
      ctk_widget_destroy (tip);
      tip = NULL;
    }
}
