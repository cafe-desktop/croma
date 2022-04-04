/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Croma theme preview widget */

/*
 * Copyright (C) 2002 Havoc Pennington
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

#define _GNU_SOURCE
#define _XOPEN_SOURCE 600 /* for the maths routines over floats */

#include <math.h>
#include <ctk/ctk.h>
#include "preview-widget.h"

static void     meta_preview_class_init    (MetaPreviewClass *klass);
static void     meta_preview_init          (MetaPreview      *preview);

static void meta_preview_get_preferred_width(CtkWidget *widget,
                                             gint *minimal, gint *natural);
static void meta_preview_get_preferred_height(CtkWidget *widget,
                                              gint *minimal, gint *natural);
static void     meta_preview_size_allocate (CtkWidget        *widget,
                                            CtkAllocation    *allocation);
static gboolean meta_preview_draw           (CtkWidget          *widget,
                                             cairo_t            *cr);
static void     meta_preview_finalize      (GObject          *object);

#define NO_CHILD_WIDTH 80
#define NO_CHILD_HEIGHT 20

G_DEFINE_TYPE (MetaPreview, meta_preview, CTK_TYPE_BIN);

static void
meta_preview_class_init (MetaPreviewClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  CtkWidgetClass *widget_class;

  widget_class = (CtkWidgetClass*) class;
  meta_preview_parent_class = g_type_class_peek (CTK_TYPE_BIN);

  gobject_class->finalize = meta_preview_finalize;

  widget_class->draw = meta_preview_draw;
  widget_class->get_preferred_width = meta_preview_get_preferred_width;
  widget_class->get_preferred_height = meta_preview_get_preferred_height;

  ctk_container_class_handle_border_width (CTK_CONTAINER_CLASS (class));
  widget_class->size_allocate = meta_preview_size_allocate;
}

static void
meta_preview_init (MetaPreview *preview)
{
  int i;

  ctk_widget_set_has_window (CTK_WIDGET (preview), FALSE);

  i = 0;
  while (i < MAX_BUTTONS_PER_CORNER)
    {
      preview->button_layout.left_buttons[i] = META_BUTTON_FUNCTION_LAST;
      preview->button_layout.right_buttons[i] = META_BUTTON_FUNCTION_LAST;
      ++i;
    }

  preview->button_layout.left_buttons[0] = META_BUTTON_FUNCTION_MENU;

  preview->button_layout.right_buttons[0] = META_BUTTON_FUNCTION_MINIMIZE;
  preview->button_layout.right_buttons[1] = META_BUTTON_FUNCTION_MAXIMIZE;
  preview->button_layout.right_buttons[2] = META_BUTTON_FUNCTION_CLOSE;

  preview->type = META_FRAME_TYPE_NORMAL;
  preview->flags =
    META_FRAME_ALLOWS_DELETE |
    META_FRAME_ALLOWS_MENU |
    META_FRAME_ALLOWS_MINIMIZE |
    META_FRAME_ALLOWS_MAXIMIZE |
    META_FRAME_ALLOWS_VERTICAL_RESIZE |
    META_FRAME_ALLOWS_HORIZONTAL_RESIZE |
    META_FRAME_HAS_FOCUS |
    META_FRAME_ALLOWS_SHADE |
    META_FRAME_ALLOWS_MOVE;

  preview->borders_cached = FALSE;
}

CtkWidget*
meta_preview_new (void)
{
  MetaPreview *preview;

  preview = g_object_new (META_TYPE_PREVIEW, NULL);

  return CTK_WIDGET (preview);
}

static void
meta_preview_finalize (GObject *object)
{
  MetaPreview *preview;

  preview = META_PREVIEW (object);

  g_free (preview->title);
  preview->title = NULL;

  G_OBJECT_CLASS (meta_preview_parent_class)->finalize (object);
}

static void
ensure_info (MetaPreview *preview)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (preview);

  if (preview->layout == NULL)
    {
      PangoFontDescription *font_desc;
      double scale;
      PangoAttrList *attrs;
      PangoAttribute *attr;

      if (preview->theme)
        scale = meta_theme_get_title_scale (preview->theme,
                                            preview->type,
                                            preview->flags);
      else
        scale = 1.0;

      preview->layout = ctk_widget_create_pango_layout (widget,
                                                        preview->title);

      font_desc = meta_ctk_widget_get_font_desc (widget, scale, NULL);

      preview->text_height =
        meta_pango_font_desc_get_text_height (font_desc,
                                              ctk_widget_get_pango_context (widget));

      attrs = pango_attr_list_new ();

      attr = pango_attr_size_new (pango_font_description_get_size (font_desc));
      attr->start_index = 0;
      attr->end_index = G_MAXINT;

      pango_attr_list_insert (attrs, attr);

      pango_layout_set_attributes (preview->layout, attrs);

      pango_attr_list_unref (attrs);

      pango_font_description_free (font_desc);
    }

  if (!preview->borders_cached)
    {
      if (preview->theme)
          meta_theme_get_frame_borders (preview->theme,
                                        preview->type,
                                        preview->text_height,
                                        preview->flags,
                                        &preview->borders);
      else
        meta_frame_borders_clear (&preview->borders);
      preview->borders_cached = TRUE;
    }
}

static gboolean
meta_preview_draw (CtkWidget *widget,
                   cairo_t   *cr)
{
  MetaPreview *preview;
  CtkAllocation allocation;
  int border_width;
  int client_width;
  int client_height;
  MetaButtonState button_states[META_BUTTON_TYPE_LAST] =
  {
    META_BUTTON_STATE_NORMAL,
    META_BUTTON_STATE_NORMAL,
    META_BUTTON_STATE_NORMAL,
    META_BUTTON_STATE_NORMAL
  };

  g_return_val_if_fail (META_IS_PREVIEW (widget), FALSE);

  preview = META_PREVIEW (widget);

  ensure_info (preview);

  cairo_save (cr);

  border_width = ctk_container_get_border_width (CTK_CONTAINER (widget));

  ctk_widget_get_allocation (widget, &allocation);
  client_width = allocation.width - preview->borders.total.left - preview->borders.total.right - border_width * 2;
  client_height = allocation.height - preview->borders.total.top - preview->borders.total.bottom - border_width * 2;

  if (client_width < 0)
    client_width = 1;
  if (client_height < 0)
    client_height = 1;

  if (preview->theme)
    {
      meta_theme_draw_frame (preview->theme,
                             ctk_widget_get_style_context (widget),
                             cr,
                             preview->type,
                             preview->flags,
                             client_width, client_height,
                             preview->layout,
                             preview->text_height,
                             &preview->button_layout,
                             button_states,
                             meta_preview_get_mini_icon (),
                             meta_preview_get_icon ());
    }

  cairo_restore (cr);

  /* draw child */
  return CTK_WIDGET_CLASS (meta_preview_parent_class)->draw (widget, cr);
}

static void
meta_preview_get_preferred_width (CtkWidget *widget,
                                  gint      *minimum,
                                  gint      *natural)
{
  MetaPreview *preview;
  int border_width;
  CtkWidget *child;

  preview = META_PREVIEW (widget);

  ensure_info (preview);

  *minimum = *natural = preview->borders.total.left + preview->borders.total.right;

  child = ctk_bin_get_child (CTK_BIN (preview));
  if (child && ctk_widget_get_visible (child))
    {
      gint child_min, child_nat;

      ctk_widget_get_preferred_width (child, &child_min, &child_nat);

      *minimum += child_min;
      *natural += child_nat;
    }
  else
    {
      *minimum += NO_CHILD_WIDTH;
      *natural += NO_CHILD_WIDTH;
    }

  border_width = ctk_container_get_border_width (CTK_CONTAINER (widget));
  *minimum += border_width * 2;
  *natural += border_width * 2;
}

static void
meta_preview_get_preferred_height (CtkWidget *widget,
                                   gint      *minimum,
                                   gint      *natural)
{
  MetaPreview *preview;
  int border_width;
  CtkWidget *child;

  preview = META_PREVIEW (widget);

  ensure_info (preview);

  *minimum = *natural = preview->borders.total.top + preview->borders.total.bottom;

  child = ctk_bin_get_child (CTK_BIN (preview));
  if (child && ctk_widget_get_visible (child))
    {
      gint child_min, child_nat;

      ctk_widget_get_preferred_height (child, &child_min, &child_nat);

      *minimum += child_min;
      *natural += child_nat;
    }
  else
    {
      *minimum += NO_CHILD_HEIGHT;
      *natural += NO_CHILD_HEIGHT;
    }

  border_width = ctk_container_get_border_width (CTK_CONTAINER (widget));
  *minimum += border_width * 2;
  *natural += border_width * 2;
}

static void
meta_preview_size_allocate (CtkWidget         *widget,
                            CtkAllocation     *allocation)
{
  MetaPreview *preview;
  int border_width;
  CtkAllocation widget_allocation, child_allocation;
  CtkWidget *child;

  preview = META_PREVIEW (widget);

  ensure_info (preview);

  ctk_widget_set_allocation (widget, allocation);

  border_width = ctk_container_get_border_width (CTK_CONTAINER (widget));

  child = ctk_bin_get_child (CTK_BIN (widget));
  if (child &&
      ctk_widget_get_visible (child))
    {
      ctk_widget_get_allocation (widget, &widget_allocation);
      child_allocation.x = widget_allocation.x + border_width + preview->borders.total.left;
      child_allocation.y = widget_allocation.y + border_width + preview->borders.total.top;

      child_allocation.width = MAX (1, widget_allocation.width - border_width * 2 - preview->borders.total.left - preview->borders.total.right);
      child_allocation.height = MAX (1, widget_allocation.height - border_width * 2 - preview->borders.total.top - preview->borders.total.bottom);

      ctk_widget_size_allocate (ctk_bin_get_child (CTK_BIN (widget)), &child_allocation);
    }
}

static void
clear_cache (MetaPreview *preview)
{
  if (preview->layout)
    {
      g_object_unref (G_OBJECT (preview->layout));
      preview->layout = NULL;
    }

  preview->borders_cached = FALSE;
}

void
meta_preview_set_theme (MetaPreview    *preview,
                        MetaTheme      *theme)
{
  g_return_if_fail (META_IS_PREVIEW (preview));

  preview->theme = theme;

  clear_cache (preview);

  ctk_widget_queue_resize (CTK_WIDGET (preview));
}

void
meta_preview_set_title (MetaPreview    *preview,
                        const char     *title)
{
  g_return_if_fail (META_IS_PREVIEW (preview));

  g_free (preview->title);
  preview->title = g_strdup (title);

  clear_cache (preview);

  ctk_widget_queue_resize (CTK_WIDGET (preview));
}

void
meta_preview_set_frame_type (MetaPreview    *preview,
                             MetaFrameType   type)
{
  g_return_if_fail (META_IS_PREVIEW (preview));

  preview->type = type;

  clear_cache (preview);

  ctk_widget_queue_resize (CTK_WIDGET (preview));
}

void
meta_preview_set_frame_flags (MetaPreview    *preview,
                              MetaFrameFlags  flags)
{
  g_return_if_fail (META_IS_PREVIEW (preview));

  preview->flags = flags;

  clear_cache (preview);

  ctk_widget_queue_resize (CTK_WIDGET (preview));
}

void
meta_preview_set_button_layout (MetaPreview            *preview,
                                const MetaButtonLayout *button_layout)
{
  g_return_if_fail (META_IS_PREVIEW (preview));

  preview->button_layout = *button_layout;

  ctk_widget_queue_draw (CTK_WIDGET (preview));
}

CdkPixbuf*
meta_preview_get_icon (void)
{
  static CdkPixbuf *default_icon = NULL;

  if (default_icon == NULL)
    {
      CtkIconTheme *theme;
      gboolean icon_exists;

      theme = ctk_icon_theme_get_default ();

      icon_exists = ctk_icon_theme_has_icon (theme, META_DEFAULT_ICON_NAME);

      if (icon_exists)
          default_icon = ctk_icon_theme_load_icon (theme,
                                                   META_DEFAULT_ICON_NAME,
                                                   META_DEFAULT_ICON_SIZE,
                                                   0,
                                                   NULL);
      else
          default_icon = ctk_icon_theme_load_icon (theme,
                                                   "image-missing",
                                                   META_DEFAULT_ICON_SIZE,
                                                   0,
                                                   NULL);

      g_assert (default_icon);
    }

  return default_icon;
}

CdkPixbuf*
meta_preview_get_mini_icon (void)
{
  static CdkPixbuf *default_icon = NULL;

  if (default_icon == NULL)
    {
      CtkIconTheme *theme;
      gboolean icon_exists;

      theme = ctk_icon_theme_get_default ();

      icon_exists = ctk_icon_theme_has_icon (theme, META_DEFAULT_ICON_NAME);

      if (icon_exists)
          default_icon = ctk_icon_theme_load_icon (theme,
                                                   META_DEFAULT_ICON_NAME,
                                                   META_MINI_ICON_WIDTH,
                                                   0,
                                                   NULL);
      else
          default_icon = ctk_icon_theme_load_icon (theme,
                                                   "image-missing",
                                                   META_MINI_ICON_WIDTH,
                                                   0,
                                                   NULL);

      g_assert (default_icon);
    }

  return default_icon;
}

cairo_region_t *
meta_preview_get_clip_region (MetaPreview *preview, gint new_window_width, gint new_window_height)
{
  CdkRectangle xrect;
  cairo_region_t *corners_xregion, *window_xregion;
  gint flags;
  MetaFrameLayout *fgeom;
  MetaFrameStyle *frame_style;

  g_return_val_if_fail (META_IS_PREVIEW (preview), NULL);

  flags = (META_PREVIEW (preview)->flags);

  window_xregion = cairo_region_create ();

  xrect.x = 0;
  xrect.y = 0;
  xrect.width = new_window_width;
  xrect.height = new_window_height;

  cairo_region_union_rectangle (window_xregion, &xrect);

  if (preview->theme == NULL)
    return window_xregion;

  /* Otherwise, we do have a theme, so calculate the corners */
  frame_style = meta_theme_get_frame_style (preview->theme,
      META_FRAME_TYPE_NORMAL, flags);

  fgeom = frame_style->layout;

  corners_xregion = cairo_region_create ();

  if (fgeom->top_left_corner_rounded_radius != 0)
    {
      const int corner = fgeom->top_left_corner_rounded_radius;
      const float radius = sqrt(corner) + corner;
      int i;

      for (i=0; i<corner; i++)
        {

          const int width = floor(0.5 + radius - sqrt(radius*radius - (radius-(i+0.5))*(radius-(i+0.5))));
          xrect.x = 0;
          xrect.y = i;
          xrect.width = width;
          xrect.height = 1;

          cairo_region_union_rectangle (corners_xregion, &xrect);
        }
    }

  if (fgeom->top_right_corner_rounded_radius != 0)
    {
      const int corner = fgeom->top_right_corner_rounded_radius;
      const float radius = sqrt(corner) + corner;
      int i;

      for (i=0; i<corner; i++)
        {
          const int width = floor(0.5 + radius - sqrt(radius*radius - (radius-(i+0.5))*(radius-(i+0.5))));
          xrect.x = new_window_width - width;
          xrect.y = i;
          xrect.width = width;
          xrect.height = 1;

          cairo_region_union_rectangle (corners_xregion, &xrect);
        }
    }

  if (fgeom->bottom_left_corner_rounded_radius != 0)
    {
      const int corner = fgeom->bottom_left_corner_rounded_radius;
      const float radius = sqrt(corner) + corner;
      int i;

      for (i=0; i<corner; i++)
        {
          const int width = floor(0.5 + radius - sqrt(radius*radius - (radius-(i+0.5))*(radius-(i+0.5))));
          xrect.x = 0;
          xrect.y = new_window_height - i - 1;
          xrect.width = width;
          xrect.height = 1;

          cairo_region_union_rectangle (corners_xregion, &xrect);
        }
    }

  if (fgeom->bottom_right_corner_rounded_radius != 0)
    {
      const int corner = fgeom->bottom_right_corner_rounded_radius;
      const float radius = sqrt(corner) + corner;
      int i;

      for (i=0; i<corner; i++)
        {
          const int width = floor(0.5 + radius - sqrt(radius*radius - (radius-(i+0.5))*(radius-(i+0.5))));
          xrect.x = new_window_width - width;
          xrect.y = new_window_height - i - 1;
          xrect.width = width;
          xrect.height = 1;

          cairo_region_union_rectangle (corners_xregion, &xrect);
        }
    }

  cairo_region_subtract (window_xregion, corners_xregion);
  cairo_region_destroy (corners_xregion);

  return window_xregion;
}
