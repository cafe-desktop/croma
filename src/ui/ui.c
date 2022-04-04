/* Croma interface for talking to CTK+ UI module */

/*
 * Copyright (C) 2002 Havoc Pennington
 * stock icon code Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org>
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

#include "prefs.h"
#include "ui.h"
#include "frames.h"
#include "util.h"
#include "menu.h"
#include "core.h"
#include "theme.h"

#include <string.h>
#include <stdlib.h>

#include <cairo-xlib.h>

static void meta_ui_accelerator_parse(const char* accel, guint* keysym, guint* keycode, CdkModifierType* keymask);

struct _MetaUI {
	Display* xdisplay;
	Screen* xscreen;
	MetaFrames* frames;

	/* For double-click tracking */
	guint button_click_number;
	Window button_click_window;
	int button_click_x;
	int button_click_y;
	guint32 button_click_time;
};

void meta_ui_init(int* argc, char*** argv)
{
  /* As of 2.91.7, Cdk uses XI2 by default, which conflicts with the
   * direct X calls we use - in particular, events caused by calls to
   * XGrabPointer/XGrabKeyboard are no longer understood by CDK, while
   * CDK will no longer generate the core XEvents we process.
   * So at least for now, enforce the previous behavior.
   */
  cdk_disable_multidevice ();

	if (!ctk_init_check (argc, argv))
	{
		meta_fatal ("Unable to open X display %s\n", XDisplayName (NULL));
	}
}

Display* meta_ui_get_display(void)
{
	return CDK_DISPLAY_XDISPLAY(cdk_display_get_default());
}

/* We do some of our event handling in frames.c, which expects
 * CDK events delivered by CTK+.  However, since the transition to
 * client side windows, we can't let CDK see button events, since the
 * client-side tracking of implicit and explicit grabs it does will
 * get confused by our direct use of X grabs in the core code.
 *
 * So we do a very minimal CDK => CTK event conversion here and send on the
 * events we care about, and then filter them out so they don't go
 * through the normal CDK event handling.
 *
 * To reduce the amount of code, the only events fields filled out
 * below are the ones that frames.c uses. If frames.c is modified to
 * use more fields, more fields need to be filled out below.
 */

static gboolean
maybe_redirect_mouse_event (XEvent *xevent)
{
  CdkDisplay *gdisplay;
  MetaUI *ui;
  CdkSeat *seat;
  CdkDevice *gdevice;
  CdkEvent *gevent;
  CdkWindow *cdk_window;
  Window window;

  switch (xevent->type)
    {
    case ButtonPress:
    case ButtonRelease:
      window = xevent->xbutton.window;
      break;
    case MotionNotify:
      window = xevent->xmotion.window;
      break;
    case EnterNotify:
    case LeaveNotify:
      window = xevent->xcrossing.window;
      break;
    default:
      return FALSE;
    }

  gdisplay = cdk_x11_lookup_xdisplay (xevent->xany.display);
  ui = g_object_get_data (G_OBJECT (gdisplay), "meta-ui");
  if (!ui)
    return FALSE;

  cdk_window = cdk_x11_window_lookup_for_display (gdisplay, window);
  if (cdk_window == NULL)
    return FALSE;

  seat = cdk_display_get_default_seat (gdisplay);
  gdevice = cdk_seat_get_pointer (seat);

  /* If CDK already thinks it has a grab, we better let it see events; this
   * is the menu-navigation case and events need to get sent to the appropriate
   * (client-side) subwindow for individual menu items.
   */
  if (cdk_display_device_is_grabbed (gdisplay, gdevice))
    return FALSE;

  switch (xevent->type)
    {
    case ButtonPress:
    case ButtonRelease:
      if (xevent->type == ButtonPress)
        {
          CtkSettings *settings = ctk_settings_get_default ();
          int double_click_time;
          int double_click_distance;

          g_object_get (settings,
                        "ctk-double-click-time", &double_click_time,
                        "ctk-double-click-distance", &double_click_distance,
                        NULL);

          if (xevent->xbutton.button == ui->button_click_number &&
              xevent->xbutton.window == ui->button_click_window &&
              xevent->xbutton.time < ui->button_click_time + double_click_time &&
              ABS (xevent->xbutton.x - ui->button_click_x) <= double_click_distance &&
              ABS (xevent->xbutton.y - ui->button_click_y) <= double_click_distance)
            {
              gevent = cdk_event_new (CDK_2BUTTON_PRESS);
              ui->button_click_number = 0;
            }
          else
            {
              gevent = cdk_event_new (CDK_BUTTON_PRESS);
              ui->button_click_number = xevent->xbutton.button;
              ui->button_click_window = xevent->xbutton.window;
              ui->button_click_time = xevent->xbutton.time;
              ui->button_click_x = xevent->xbutton.x;
              ui->button_click_y = xevent->xbutton.y;
            }
        }
      else
        {
          gevent = cdk_event_new (CDK_BUTTON_RELEASE);
        }

      gevent->button.window = g_object_ref (cdk_window);
      gevent->button.button = xevent->xbutton.button;
      gevent->button.time = xevent->xbutton.time;
      gevent->button.x = xevent->xbutton.x;
      gevent->button.y = xevent->xbutton.y;
      gevent->button.x_root = xevent->xbutton.x_root;
      gevent->button.y_root = xevent->xbutton.y_root;
      break;
    case MotionNotify:
      gevent = cdk_event_new (CDK_MOTION_NOTIFY);
      gevent->motion.type = CDK_MOTION_NOTIFY;
      gevent->motion.window = g_object_ref (cdk_window);
      break;
    case EnterNotify:
    case LeaveNotify:
      gevent = cdk_event_new (xevent->type == EnterNotify ? CDK_ENTER_NOTIFY : CDK_LEAVE_NOTIFY);
      gevent->crossing.window = g_object_ref (cdk_window);
      gevent->crossing.x = xevent->xcrossing.x;
      gevent->crossing.y = xevent->xcrossing.y;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

  /* If we've gotten here, we've filled in the cdk_event and should send it on */
  cdk_event_set_device (gevent, gdevice);
  ctk_main_do_event (gevent);
  cdk_event_free (gevent);

  return TRUE;
}

typedef struct _EventFunc EventFunc;

struct _EventFunc
{
  MetaEventFunc func;
  gpointer data;
};

static EventFunc *ef = NULL;

static CdkFilterReturn
filter_func (CdkXEvent *xevent,
             CdkEvent *event,
             gpointer data)
{
  g_return_val_if_fail (ef != NULL, CDK_FILTER_CONTINUE);

  if ((* ef->func) (xevent, ef->data) ||
      maybe_redirect_mouse_event (xevent))
    return CDK_FILTER_REMOVE;
  else
    return CDK_FILTER_CONTINUE;
}

void
meta_ui_add_event_func (Display       *xdisplay,
                        MetaEventFunc  func,
                        gpointer       data)
{
  g_return_if_fail (ef == NULL);

  ef = g_new (EventFunc, 1);
  ef->func = func;
  ef->data = data;

  cdk_window_add_filter (NULL, filter_func, ef);
}

/* removal is by data due to proxy function */
void
meta_ui_remove_event_func (Display       *xdisplay,
                           MetaEventFunc  func,
                           gpointer       data)
{
  g_return_if_fail (ef != NULL);

  cdk_window_remove_filter (NULL, filter_func, ef);

  g_free (ef);
  ef = NULL;
}

MetaUI*
meta_ui_new (Display *xdisplay,
             Screen  *screen)
{
  CdkDisplay *gdisplay;
  MetaUI *ui;

  ui = g_new0 (MetaUI, 1);
  ui->xdisplay = xdisplay;
  ui->xscreen = screen;

  gdisplay = cdk_x11_lookup_xdisplay (xdisplay);
  g_assert (gdisplay == cdk_display_get_default ());

  g_assert (xdisplay == CDK_DISPLAY_XDISPLAY (cdk_display_get_default ()));
  ui->frames = meta_frames_new ();
  /* CTK+ needs the frame-sync protocol to work in order to properly
   * handle style changes. This means that the dummy widget we create
   * to get the style for title bars actually needs to be mapped
   * and fully tracked as a MetaWindow. Horrible, but mostly harmless -
   * the window is a 1x1 overide redirect window positioned offscreen.
   */
  ctk_widget_show (CTK_WIDGET (ui->frames));

  g_object_set_data (G_OBJECT (gdisplay), "meta-ui", ui);

  return ui;
}

void
meta_ui_free (MetaUI *ui)
{
  CdkDisplay *gdisplay;

  ctk_widget_destroy (CTK_WIDGET (ui->frames));

  gdisplay = cdk_x11_lookup_xdisplay (ui->xdisplay);
  g_object_set_data (G_OBJECT (gdisplay), "meta-ui", NULL);

  g_free (ui);
}

void
meta_ui_get_frame_borders (MetaUI *ui,
                           Window frame_xwindow,
                           MetaFrameBorders *borders)
{
  meta_frames_get_borders (ui->frames, frame_xwindow, borders);
}

void
meta_ui_get_corner_radiuses (MetaUI *ui,
                             Window  xwindow,
                             float  *top_left,
                             float  *top_right,
                             float  *bottom_left,
                             float  *bottom_right)
{
  meta_frames_get_corner_radiuses (ui->frames, xwindow,
                                   top_left, top_right,
                                   bottom_left, bottom_right);
}

static void
set_background_none (Display *xdisplay,
                     Window   xwindow)
{
  XSetWindowAttributes attrs;

  attrs.background_pixmap = None;
  XChangeWindowAttributes (xdisplay, xwindow,
                           CWBackPixmap, &attrs);
}

Window
meta_ui_create_frame_window (MetaUI *ui,
                             Display *xdisplay,
                             Visual *xvisual,
			     gint x,
			     gint y,
			     gint width,
			     gint height)
{
  CdkDisplay *display = cdk_x11_lookup_xdisplay (xdisplay);
  CdkScreen *screen = cdk_display_get_default_screen (display);
  CdkWindowAttr attrs;
  gint attributes_mask;
  CdkWindow *window;
  CdkVisual *visual;

  /* Default depth/visual handles clients with weird visuals; they can
   * always be children of the root depth/visual obviously, but
   * e.g. DRI games can't be children of a parent that has the same
   * visual as the client.
   */
  if (!xvisual)
    visual = cdk_screen_get_system_visual (screen);
  else
    {
      visual = cdk_x11_screen_lookup_visual (screen,
                                             XVisualIDFromVisual (xvisual));
    }

  attrs.title = NULL;

  /* frame.c is going to replace the event mask immediately, but
   * we still have to set it here to let CDK know what it is.
   */
  attrs.event_mask =
    CDK_EXPOSURE_MASK | CDK_BUTTON_PRESS_MASK | CDK_BUTTON_RELEASE_MASK |
    CDK_POINTER_MOTION_MASK | CDK_POINTER_MOTION_HINT_MASK |
    CDK_ENTER_NOTIFY_MASK | CDK_LEAVE_NOTIFY_MASK | CDK_FOCUS_CHANGE_MASK;
  attrs.x = x;
  attrs.y = y;
  attrs.wclass = CDK_INPUT_OUTPUT;
  attrs.visual = visual;
  attrs.window_type = CDK_WINDOW_CHILD;
  attrs.cursor = NULL;
  attrs.wmclass_name = NULL;
  attrs.wmclass_class = NULL;
  attrs.override_redirect = FALSE;

  attrs.width  = width;
  attrs.height = height;

  attributes_mask = CDK_WA_X | CDK_WA_Y | CDK_WA_VISUAL;

  window =
    cdk_window_new (cdk_screen_get_root_window(screen),
		    &attrs, attributes_mask);

  cdk_window_resize (window, width, height);
  set_background_none (xdisplay, CDK_WINDOW_XID (window));

  meta_frames_manage_window (ui->frames, CDK_WINDOW_XID (window), window);

  return CDK_WINDOW_XID (window);
}

void
meta_ui_destroy_frame_window (MetaUI *ui,
			      Window  xwindow)
{
  meta_frames_unmanage_window (ui->frames, xwindow);
}

void
meta_ui_move_resize_frame (MetaUI *ui,
			   Window frame,
			   int x,
			   int y,
			   int width,
			   int height)
{
  meta_frames_move_resize_frame (ui->frames, frame, x, y, width, height);
}

void
meta_ui_map_frame   (MetaUI *ui,
                     Window  xwindow)
{
  CdkWindow *window;

  CdkDisplay *display;

  display = cdk_x11_lookup_xdisplay (ui->xdisplay);
  window = cdk_x11_window_lookup_for_display (display, xwindow);

  if (window)
    cdk_window_show_unraised (window);
}

void
meta_ui_unmap_frame (MetaUI *ui,
                     Window  xwindow)
{
  CdkWindow *window;

  CdkDisplay *display;

  display = cdk_x11_lookup_xdisplay (ui->xdisplay);
  window = cdk_x11_window_lookup_for_display (display, xwindow);

  if (window)
    cdk_window_hide (window);
}

void
meta_ui_update_frame_style (MetaUI *ui,
                            Window  xwindow)
{
  meta_frames_update_frame_style (ui->frames, xwindow);
}

void
meta_ui_repaint_frame (MetaUI *ui,
                       Window xwindow)
{
  meta_frames_repaint_frame (ui->frames, xwindow);
}

void
meta_ui_apply_frame_shape  (MetaUI  *ui,
                            Window   xwindow,
                            int      new_window_width,
                            int      new_window_height,
                            gboolean window_has_shape)
{
  meta_frames_apply_shapes (ui->frames, xwindow,
                            new_window_width, new_window_height,
                            window_has_shape);
}

cairo_region_t *
meta_ui_get_frame_bounds (MetaUI *ui,
                          Window  xwindow,
                          int     window_width,
                          int     window_height)
{
  return meta_frames_get_frame_bounds (ui->frames,
                                       xwindow,
                                       window_width,
                                       window_height);
}

void
meta_ui_queue_frame_draw (MetaUI *ui,
                          Window xwindow)
{
  meta_frames_queue_draw (ui->frames, xwindow);
}


void
meta_ui_set_frame_title (MetaUI     *ui,
                         Window      xwindow,
                         const char *title)
{
  meta_frames_set_title (ui->frames, xwindow, title);
}

MetaWindowMenu*
meta_ui_window_menu_new  (MetaUI             *ui,
                          Window              client_xwindow,
                          MetaMenuOp          ops,
                          MetaMenuOp          insensitive,
                          unsigned long       active_workspace,
                          int                 n_workspaces,
                          MetaWindowMenuFunc  func,
                          gpointer            data)
{
  return meta_window_menu_new (ui->frames,
                               ops, insensitive,
                               client_xwindow,
                               active_workspace,
                               n_workspaces,
                               func, data);
}

void
meta_ui_window_menu_popup (MetaWindowMenu     *menu,
                           int                 root_x,
                           int                 root_y,
                           int                 button,
                           guint32             timestamp)
{
  meta_window_menu_popup (menu, root_x, root_y, button, timestamp);
}

void
meta_ui_window_menu_free (MetaWindowMenu *menu)
{
  meta_window_menu_free (menu);
}

CdkPixbuf*
meta_gdk_pixbuf_get_from_pixmap (CdkPixbuf   *dest,
                                 Pixmap       xpixmap,
                                 int          src_x,
                                 int          src_y,
                                 int          dest_x,
                                 int          dest_y,
                                 int          width,
                                 int          height)
{
  cairo_surface_t *surface;
  Display *display;
  Window root_return;
  int x_ret, y_ret;
  unsigned int w_ret, h_ret, bw_ret, depth_ret;
  XWindowAttributes attrs;
  CdkPixbuf *retval;

  display = CDK_DISPLAY_XDISPLAY (cdk_display_get_default ());

  if (!XGetGeometry (display, xpixmap, &root_return,
                     &x_ret, &y_ret, &w_ret, &h_ret, &bw_ret, &depth_ret))
    return NULL;

  if (depth_ret == 1)
    {
      surface = cairo_xlib_surface_create_for_bitmap (display,
                                                      xpixmap,
                                                      CDK_SCREEN_XSCREEN (cdk_screen_get_default ()),
                                                      w_ret,
                                                      h_ret);
    }
  else
    {
      if (!XGetWindowAttributes (display, root_return, &attrs))
        return NULL;

      surface = cairo_xlib_surface_create (display,
                                           xpixmap,
                                           attrs.visual,
                                           w_ret, h_ret);
    }

  retval = gdk_pixbuf_get_from_surface (surface,
                                        src_x,
                                        src_y,
                                        width,
                                        height);
  cairo_surface_destroy (surface);

  return retval;
}

void
meta_ui_push_delay_exposes (MetaUI *ui)
{
  meta_frames_push_delay_exposes (ui->frames);
}

void
meta_ui_pop_delay_exposes  (MetaUI *ui)
{
  meta_frames_pop_delay_exposes (ui->frames);
}

static CdkPixbuf *
load_default_window_icon (int size, int scale)
{
  CtkIconTheme *theme = ctk_icon_theme_get_default ();
  const char *icon_name;

  if (ctk_icon_theme_has_icon (theme, META_DEFAULT_ICON_NAME))
    icon_name = META_DEFAULT_ICON_NAME;
  else
    icon_name = "image-missing";

  return ctk_icon_theme_load_icon_for_scale (theme, icon_name, size, scale, 0, NULL);
}

CdkPixbuf*
meta_ui_get_default_window_icon (MetaUI *ui)
{
  static CdkPixbuf *default_icon = NULL;
  static int icon_size = 0;
  int current_icon_size = meta_prefs_get_icon_size();

  int scale;
  if (default_icon == NULL || current_icon_size != icon_size)
    {
      scale = ctk_widget_get_scale_factor (CTK_WIDGET (ui->frames));
      default_icon = load_default_window_icon (current_icon_size, scale);
      g_assert (default_icon);
      icon_size = current_icon_size;
    }

  g_object_ref (G_OBJECT (default_icon));

  return default_icon;
}

CdkPixbuf*
meta_ui_get_default_mini_icon (MetaUI *ui)
{
  static CdkPixbuf *default_icon = NULL;
  int scale;

  if (default_icon == NULL)
    {
      scale = ctk_widget_get_scale_factor (CTK_WIDGET (ui->frames));
      default_icon = load_default_window_icon (META_MINI_ICON_WIDTH, scale);
      g_assert (default_icon);
    }

  g_object_ref (G_OBJECT (default_icon));

  return default_icon;
}

gboolean
meta_ui_window_should_not_cause_focus (Display *xdisplay,
                                       Window   xwindow)
{
  CdkWindow *window;

  CdkDisplay *display;

  display = cdk_x11_lookup_xdisplay (xdisplay);
  window = cdk_x11_window_lookup_for_display (display, xwindow);

  /* we shouldn't cause focus if we're an override redirect
   * toplevel which is not foreign
   */
  if (window && cdk_window_get_window_type (window) == CDK_WINDOW_TEMP)
    return TRUE;
  else
    return FALSE;
}

void
meta_ui_theme_get_frame_borders (MetaUI           *ui,
                                 MetaFrameType     type,
                                 MetaFrameFlags    flags,
                                 MetaFrameBorders *borders)
{
  int text_height;
  CtkStyleContext *style = NULL;
  PangoFontDescription *free_font_desc = NULL;
  PangoContext *context;
  const PangoFontDescription *font_desc;

  if (meta_ui_have_a_theme ())
    {
      context = ctk_widget_get_pango_context (CTK_WIDGET (ui->frames));
      font_desc = meta_prefs_get_titlebar_font ();

      if (!font_desc)
        {
          CdkDisplay *display = cdk_x11_lookup_xdisplay (ui->xdisplay);
          CdkScreen *screen = cdk_display_get_default_screen (display);
          CtkWidgetPath *widget_path;

          style = ctk_style_context_new ();
          ctk_style_context_set_screen (style, screen);
          widget_path = ctk_widget_path_new ();
          ctk_widget_path_append_type (widget_path, CTK_TYPE_WINDOW);
          ctk_style_context_set_path (style, widget_path);
          ctk_widget_path_free (widget_path);

          ctk_style_context_save (style);
          ctk_style_context_set_state (style, CTK_STATE_FLAG_NORMAL);
          ctk_style_context_get (style,
                                 ctk_style_context_get_state (style),
                                 "font",
                                 &free_font_desc,
                                 NULL);
          ctk_style_context_restore (style);
          font_desc = (const PangoFontDescription *) free_font_desc;
        }

      text_height = meta_pango_font_desc_get_text_height (font_desc, context);

      meta_theme_get_frame_borders (meta_theme_get_current (),
                                    type, text_height, flags,
                                    borders);

      if (free_font_desc)
        pango_font_description_free (free_font_desc);
    }
  else
    {
      meta_frame_borders_clear (borders);
    }

  if (style != NULL)
    g_object_unref (style);
}

void
meta_ui_set_current_theme (const char *name,
                           gboolean    force_reload)
{
  meta_theme_set_current (name, force_reload);
  meta_invalidate_default_icons ();
}

gboolean
meta_ui_have_a_theme (void)
{
  return meta_theme_get_current () != NULL;
}

static void
meta_ui_accelerator_parse (const char      *accel,
                           guint           *keysym,
                           guint           *keycode,
                           CdkModifierType *keymask)
{
  if (accel[0] == '0' && accel[1] == 'x')
    {
      *keysym = 0;
      *keycode = (guint) strtoul (accel, NULL, 16);
      *keymask = 0;
    }
  else
    ctk_accelerator_parse (accel, keysym, keymask);
}

gboolean
meta_ui_parse_accelerator (const char          *accel,
                           unsigned int        *keysym,
                           unsigned int        *keycode,
                           MetaVirtualModifier *mask)
{
  CdkModifierType cdk_mask = 0;
  guint cdk_sym = 0;
  guint cdk_code = 0;

  *keysym = 0;
  *keycode = 0;
  *mask = 0;

  if (!accel[0] || strcmp (accel, "disabled") == 0)
    return TRUE;

  meta_ui_accelerator_parse (accel, &cdk_sym, &cdk_code, &cdk_mask);
  if (cdk_mask == 0 && cdk_sym == 0 && cdk_code == 0)
    return FALSE;

  if (cdk_sym == None && cdk_code == 0)
    return FALSE;

  if (cdk_mask & CDK_RELEASE_MASK) /* we don't allow this */
    return FALSE;

  *keysym = cdk_sym;
  *keycode = cdk_code;

  if (cdk_mask & CDK_SHIFT_MASK)
    *mask |= META_VIRTUAL_SHIFT_MASK;
  if (cdk_mask & CDK_CONTROL_MASK)
    *mask |= META_VIRTUAL_CONTROL_MASK;
  if (cdk_mask & CDK_MOD1_MASK)
    *mask |= META_VIRTUAL_ALT_MASK;
  if (cdk_mask & CDK_MOD2_MASK)
    *mask |= META_VIRTUAL_MOD2_MASK;
  if (cdk_mask & CDK_MOD3_MASK)
    *mask |= META_VIRTUAL_MOD3_MASK;
  if (cdk_mask & CDK_MOD4_MASK)
    *mask |= META_VIRTUAL_MOD4_MASK;
  if (cdk_mask & CDK_MOD5_MASK)
    *mask |= META_VIRTUAL_MOD5_MASK;
  if (cdk_mask & CDK_SUPER_MASK)
    *mask |= META_VIRTUAL_SUPER_MASK;
  if (cdk_mask & CDK_HYPER_MASK)
    *mask |= META_VIRTUAL_HYPER_MASK;
  if (cdk_mask & CDK_META_MASK)
    *mask |= META_VIRTUAL_META_MASK;

  return TRUE;
}

/* Caller responsible for freeing return string of meta_ui_accelerator_name! */
gchar*
meta_ui_accelerator_name  (unsigned int        keysym,
                           MetaVirtualModifier mask)
{
  CdkModifierType mods = 0;

  if (keysym == 0 && mask == 0)
    {
      return g_strdup ("disabled");
    }

  if (mask & META_VIRTUAL_SHIFT_MASK)
    mods |= CDK_SHIFT_MASK;
  if (mask & META_VIRTUAL_CONTROL_MASK)
    mods |= CDK_CONTROL_MASK;
  if (mask & META_VIRTUAL_ALT_MASK)
    mods |= CDK_MOD1_MASK;
  if (mask & META_VIRTUAL_MOD2_MASK)
    mods |= CDK_MOD2_MASK;
  if (mask & META_VIRTUAL_MOD3_MASK)
    mods |= CDK_MOD3_MASK;
  if (mask & META_VIRTUAL_MOD4_MASK)
    mods |= CDK_MOD4_MASK;
  if (mask & META_VIRTUAL_MOD5_MASK)
    mods |= CDK_MOD5_MASK;
  if (mask & META_VIRTUAL_SUPER_MASK)
    mods |= CDK_SUPER_MASK;
  if (mask & META_VIRTUAL_HYPER_MASK)
    mods |= CDK_HYPER_MASK;
  if (mask & META_VIRTUAL_META_MASK)
    mods |= CDK_META_MASK;

  return ctk_accelerator_name (keysym, mods);

}

gboolean
meta_ui_parse_modifier (const char          *accel,
                        MetaVirtualModifier *mask)
{
  CdkModifierType cdk_mask = 0;
  guint cdk_sym = 0;
  guint cdk_code = 0;

  *mask = 0;

  if (accel == NULL || !accel[0] || strcmp (accel, "disabled") == 0)
    return TRUE;

  meta_ui_accelerator_parse (accel, &cdk_sym, &cdk_code, &cdk_mask);
  if (cdk_mask == 0 && cdk_sym == 0 && cdk_code == 0)
    return FALSE;

  if (cdk_sym != None || cdk_code != 0)
    return FALSE;

  if (cdk_mask & CDK_RELEASE_MASK) /* we don't allow this */
    return FALSE;

  if (cdk_mask & CDK_SHIFT_MASK)
    *mask |= META_VIRTUAL_SHIFT_MASK;
  if (cdk_mask & CDK_CONTROL_MASK)
    *mask |= META_VIRTUAL_CONTROL_MASK;
  if (cdk_mask & CDK_MOD1_MASK)
    *mask |= META_VIRTUAL_ALT_MASK;
  if (cdk_mask & CDK_MOD2_MASK)
    *mask |= META_VIRTUAL_MOD2_MASK;
  if (cdk_mask & CDK_MOD3_MASK)
    *mask |= META_VIRTUAL_MOD3_MASK;
  if (cdk_mask & CDK_MOD4_MASK)
    *mask |= META_VIRTUAL_MOD4_MASK;
  if (cdk_mask & CDK_MOD5_MASK)
    *mask |= META_VIRTUAL_MOD5_MASK;
  if (cdk_mask & CDK_SUPER_MASK)
    *mask |= META_VIRTUAL_SUPER_MASK;
  if (cdk_mask & CDK_HYPER_MASK)
    *mask |= META_VIRTUAL_HYPER_MASK;
  if (cdk_mask & CDK_META_MASK)
    *mask |= META_VIRTUAL_META_MASK;

  return TRUE;
}

gboolean
meta_ui_window_is_widget (MetaUI *ui,
                          Window  xwindow)
{
  CdkWindow *window;

  CdkDisplay *display;
  display = cdk_x11_lookup_xdisplay (ui->xdisplay);
  window = cdk_x11_window_lookup_for_display (display, xwindow);

  if (window)
    {
      void *user_data = NULL;
      cdk_window_get_user_data (window, &user_data);
      return user_data != NULL && user_data != ui->frames;
    }
  else
    return FALSE;
}

/* stock icon code Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org> */

typedef struct {
	char* stock_id;
	const guint8* icon_data;
} MetaStockIcon;

int meta_ui_get_drag_threshold(MetaUI* ui)
{
	int threshold = 8;
	CtkSettings* settings = ctk_widget_get_settings(CTK_WIDGET(ui->frames));

	g_object_get(G_OBJECT(settings), "ctk-dnd-drag-threshold", &threshold, NULL);

	return threshold;
}

MetaUIDirection meta_ui_get_direction(void)
{
	if (ctk_widget_get_default_direction() == CTK_TEXT_DIR_RTL)
	{
		return META_UI_DIRECTION_RTL;
	}

	return META_UI_DIRECTION_LTR;
}

CdkPixbuf *meta_ui_get_pixbuf_from_surface (cairo_surface_t *surface)
{
	gint width;
	gint height;

	width = cairo_xlib_surface_get_width (surface);
	height = cairo_xlib_surface_get_height (surface);

	return gdk_pixbuf_get_from_surface (surface, 0, 0, width, height);
}

