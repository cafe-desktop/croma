/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Draw a workspace */

/* This file should not be modified to depend on other files in
 * libwnck or croma, since it's used in both of them
 */

/*
 * Copyright (C) 2002 Red Hat Inc.
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

#include "draw-workspace.h"
#include "theme.h"

static void
get_window_rect (const WnckWindowDisplayInfo *win,
                 int                    screen_width,
                 int                    screen_height,
                 const CdkRectangle    *workspace_rect,
                 CdkRectangle          *rect)
{
  double width_ratio, height_ratio;
  int x, y, width, height;

  width_ratio = (double) workspace_rect->width / (double) screen_width;
  height_ratio = (double) workspace_rect->height / (double) screen_height;

  x = win->x;
  y = win->y;
  width = win->width;
  height = win->height;

  x *= width_ratio;
  y *= height_ratio;
  width *= width_ratio;
  height *= height_ratio;

  x += workspace_rect->x;
  y += workspace_rect->y;

  if (width < 3)
    width = 3;
  if (height < 3)
    height = 3;

  rect->x = x;
  rect->y = y;
  rect->width = width;
  rect->height = height;
}

static void
get_background_color (CtkStyleContext *context,
                      CtkStateFlags    state,
                      CdkRGBA         *color)
{
  CdkRGBA *c;

  g_return_if_fail (color != NULL);
  g_return_if_fail (CTK_IS_STYLE_CONTEXT (context));

  ctk_style_context_get (context,
                         state,
                         "background-color", &c,
                         NULL);

  *color = *c;
  cdk_rgba_free (c);
}

static void
draw_window (CtkWidget                   *widget,
             cairo_t                     *cr,
             const WnckWindowDisplayInfo *win,
             const CdkRectangle          *winrect,
             CtkStateType                state)
{
  cairo_surface_t *icon;
  int icon_w, icon_h, scale;
  gboolean is_active;
  CdkRGBA color;
  CtkStyleContext *style;

  is_active = win->is_active;

  cairo_save (cr);

  cairo_rectangle (cr, winrect->x, winrect->y, winrect->width, winrect->height);
  cairo_clip (cr);

  style = ctk_widget_get_style_context (widget);
  if (is_active)
    meta_ctk_style_get_light_color (style, state, &color);
  else
    get_background_color (style, state, &color);

  cdk_cairo_set_source_rgba (cr, &color);

  cairo_rectangle (cr,
                   winrect->x + 1, winrect->y + 1,
                   MAX (0, winrect->width - 2), MAX (0, winrect->height - 2));
  cairo_fill (cr);

  scale = ctk_widget_get_scale_factor (widget);

  icon = cdk_cairo_surface_create_from_pixbuf (win->icon, scale, NULL);

  icon_w = icon_h = 0;

  if (icon)
    {
      icon_w = cairo_image_surface_get_width (icon) / scale;
      icon_h = cairo_image_surface_get_height (icon) / scale;

      /* If the icon is too big, fall back to mini icon. */
      if (icon_w > (winrect->width - 2) || icon_h > (winrect->height - 2))
        {
          cairo_surface_destroy (icon);
          icon = cdk_cairo_surface_create_from_pixbuf (win->mini_icon, scale, NULL);
          if (icon)
            {
              icon_w = cairo_image_surface_get_width (icon) / scale;
              icon_h = cairo_image_surface_get_height (icon) / scale;

              /* Give up. */
              if (icon_w > (winrect->width - 2) || icon_h > (winrect->height - 2))
                {
                  cairo_surface_destroy (icon);
                  icon = NULL;
                }
            }
        }
    }

  if (icon)
    {
      int icon_x, icon_y;

      icon_x = winrect->x + (winrect->width - icon_w) / 2;
      icon_y = winrect->y + (winrect->height - icon_h) / 2;

      cairo_save (cr);
      cairo_set_source_surface (cr, icon, icon_x, icon_y);
      cairo_rectangle (cr, icon_x, icon_y, icon_w, icon_h);
      cairo_clip (cr);
      cairo_paint (cr);
      cairo_restore (cr);

      cairo_surface_destroy (icon);
    }

  ctk_style_context_get_color (style, state, &color);
  cdk_cairo_set_source_rgba (cr, &color);

  cairo_set_line_width (cr, 1.0);
  cairo_rectangle (cr,
                   winrect->x + 0.5, winrect->y + 0.5,
                   MAX (0, winrect->width - 1), MAX (0, winrect->height - 1));
  cairo_stroke (cr);

  cairo_restore(cr);
}

void
wnck_draw_workspace (CtkWidget                   *widget,
                     cairo_t                     *cr,
                     int                          x,
                     int                          y,
                     int                          width,
                     int                          height,
                     int                          screen_width,
                     int                          screen_height,
                     GdkPixbuf                   *workspace_background,
                     gboolean                     is_active,
                     const WnckWindowDisplayInfo *windows,
                     int                          n_windows)
{
  int i;
  CdkRectangle workspace_rect;
  CtkStateFlags state;
  CtkStyleContext *style;

  workspace_rect.x = x;
  workspace_rect.y = y;
  workspace_rect.width = width;
  workspace_rect.height = height;

  if (is_active)
    state = CTK_STATE_FLAG_SELECTED;
  else if (workspace_background)
    state = CTK_STATE_FLAG_PRELIGHT;
  else
    state = CTK_STATE_FLAG_NORMAL;

  style = ctk_widget_get_style_context (widget);
  cairo_save (cr);

  if (workspace_background)
    {
      cdk_cairo_set_source_pixbuf (cr, workspace_background, x, y);
      cairo_paint (cr);
    }
  else
    {
      CdkRGBA color;

      meta_ctk_style_get_dark_color (style, state, &color);
      color.alpha = 0.25;

      cdk_cairo_set_source_rgba (cr, &color);
      cairo_rectangle (cr, x, y, width, height);
      cairo_fill (cr);
    }

  i = 0;
  while (i < n_windows)
    {
      const WnckWindowDisplayInfo *win = &windows[i];
      CdkRectangle winrect;

      get_window_rect (win, screen_width,
                       screen_height, &workspace_rect, &winrect);

      draw_window (widget,
                   cr,
                   win,
                   &winrect,
                   state);

      ++i;
    }

  cairo_restore(cr);
}
