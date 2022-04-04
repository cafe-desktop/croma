/* Croma window types/properties demo app */

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

#include <ctk/ctk.h>
#include <cdk/cdkx.h>
#include <X11/Xatom.h>
#include <unistd.h>

static void
do_appwindow (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data);

static gboolean aspect_on;

static void
set_cdk_window_struts (CdkWindow *window,
                       int        left,
                       int        right,
                       int        top,
                       int        bottom)
{
  long vals[12];

  vals[0] = left;
  vals[1] = right;
  vals[2] = top;
  vals[3] = bottom;
  vals[4] = 000;
  vals[5] = 400;
  vals[6] = 200;
  vals[7] = 600;
  vals[8] = 76;
  vals[9] = 676;
  vals[10] = 200;
  vals[11] = 800;

  XChangeProperty (CDK_WINDOW_XDISPLAY (window),
                   CDK_WINDOW_XID (window),
                   XInternAtom (CDK_WINDOW_XDISPLAY (window),
                                "_NET_WM_STRUT_PARTIAL", False),
                   XA_CARDINAL, 32, PropModeReplace,
                   (guchar *)vals, 12);
}

static void
on_realize_set_struts (CtkWindow *window,
                       gpointer   data)
{
  CtkWidget *widget;
  int left;
  int right;
  int top;
  int bottom;

  widget = CTK_WIDGET (window);

  g_return_if_fail (ctk_widget_get_realized (widget));

  left = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "meta-strut-left"));
  right = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "meta-strut-right"));
  top = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "meta-strut-top"));
  bottom = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "meta-strut-bottom"));

  set_cdk_window_struts (ctk_widget_get_window (widget),
                         left, right, top, bottom);
}

static void
set_ctk_window_struts (CtkWidget  *window,
                       int         left,
                       int         right,
                       int         top,
                       int         bottom)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (window);

  g_object_set_data (G_OBJECT (window), "meta-strut-left",
                     GINT_TO_POINTER (left));
  g_object_set_data (G_OBJECT (window), "meta-strut-right",
                     GINT_TO_POINTER (right));
  g_object_set_data (G_OBJECT (window), "meta-strut-top",
                     GINT_TO_POINTER (top));
  g_object_set_data (G_OBJECT (window), "meta-strut-bottom",
                     GINT_TO_POINTER (bottom));

  g_signal_handlers_disconnect_by_func (G_OBJECT (window),
                                        on_realize_set_struts,
                                        NULL);

  g_signal_connect_after (G_OBJECT (window),
                          "realize",
                          G_CALLBACK (on_realize_set_struts),
                          NULL);

  if (ctk_widget_get_realized (widget))
    set_cdk_window_struts (ctk_widget_get_window (widget),
                           left, right, top, bottom);
}

static void
set_cdk_window_type (CdkWindow  *window,
                     const char *type)
{
  Atom atoms[2] = { None, None };

  atoms[0] = XInternAtom (CDK_WINDOW_XDISPLAY (window),
                          type, False);

  XChangeProperty (CDK_WINDOW_XDISPLAY (window),
                   CDK_WINDOW_XID (window),
                   XInternAtom (CDK_WINDOW_XDISPLAY (window), "_NET_WM_WINDOW_TYPE", False),
                   XA_ATOM, 32, PropModeReplace,
                   (guchar *)atoms,
                   1);
}

static void
on_realize_set_type (CtkWindow *window,
                     gpointer   data)
{
  const char *type;

  g_return_if_fail (ctk_widget_get_realized (CTK_WIDGET (window)));

  type = g_object_get_data (G_OBJECT (window), "meta-window-type");

  g_return_if_fail (type != NULL);

  set_cdk_window_type (ctk_widget_get_window (CTK_WIDGET (window)),
                       type);
}

static void
set_ctk_window_type (CtkWindow  *window,
                     const char *type)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (window);

  g_object_set_data (G_OBJECT (window), "meta-window-type", (char*) type);

  g_signal_handlers_disconnect_by_func (G_OBJECT (window),
                                        on_realize_set_type,
                                        NULL);

  g_signal_connect_after (G_OBJECT (window),
                          "realize",
                          G_CALLBACK (on_realize_set_type),
                          NULL);

  if (ctk_widget_get_realized (widget))
    set_cdk_window_type (ctk_widget_get_window (widget),
                         type);
}

static void
set_cdk_window_border_only (CdkWindow *window)
{
  cdk_window_set_decorations (window, CDK_DECOR_BORDER);
}

static void
on_realize_set_border_only (CtkWindow *window,
                            gpointer   data)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (window);

  g_return_if_fail (ctk_widget_get_realized (widget));

  set_cdk_window_border_only (ctk_widget_get_window (widget));
}

static void
set_ctk_window_border_only (CtkWindow  *window)
{
CtkWidget *widget;

  widget = CTK_WIDGET (window);

  g_signal_handlers_disconnect_by_func (G_OBJECT (window),
                                        on_realize_set_border_only,
                                        NULL);

  g_signal_connect_after (G_OBJECT (window),
                          "realize",
                          G_CALLBACK (on_realize_set_border_only),
                          NULL);

  if (ctk_widget_get_realized (widget))
    set_cdk_window_border_only (ctk_widget_get_window (widget));
}

int
main (int argc, char **argv)
{
  GList *list;
  CdkPixbuf *pixbuf;
  GError *err;

  ctk_init (&argc, &argv);

  err = NULL;
  pixbuf = cdk_pixbuf_new_from_file (CROMA_ICON_DIR"/croma-window-demo.png",
                                     &err);
  if (pixbuf)
    {
      list = g_list_prepend (NULL, pixbuf);

      ctk_window_set_default_icon_list (list);
      g_list_free (list);
      g_object_unref (G_OBJECT (pixbuf));
    }
  else
    {
      g_printerr ("Could not load icon: %s\n", err->message);
      g_error_free (err);
    }

  do_appwindow (NULL, NULL, NULL);

  ctk_main ();

  return 0;
}

static void
response_cb (CtkDialog *dialog,
             int        response_id,
             void      *data);

static void
make_dialog (CtkWidget *parent,
             int        depth)
{
  CtkWidget *dialog;
  char *str;

  dialog = ctk_message_dialog_new (parent ? CTK_WINDOW (parent) : NULL,
                                   CTK_DIALOG_DESTROY_WITH_PARENT,
                                   CTK_MESSAGE_INFO,
                                   CTK_BUTTONS_CLOSE,
                                   parent ? "Here is a dialog %d" :
                                   "Here is a dialog %d with no transient parent",
                                   depth);

  str = g_strdup_printf ("%d dialog", depth);
  ctk_window_set_title (CTK_WINDOW (dialog), str);
  g_free (str);

  ctk_dialog_add_button (CTK_DIALOG (dialog),
                         "Open child dialog",
                         CTK_RESPONSE_ACCEPT);

  /* Close dialog on user response */
  g_signal_connect (G_OBJECT (dialog),
                    "response",
                    G_CALLBACK (response_cb),
                    NULL);

  g_object_set_data (G_OBJECT (dialog), "depth",
                     GINT_TO_POINTER (depth));

  ctk_widget_show (dialog);
}

static void
response_cb (CtkDialog *dialog,
             int        response_id,
             void      *data)
{
  switch (response_id)
    {
    case CTK_RESPONSE_ACCEPT:
      make_dialog (CTK_WIDGET (dialog),
                   GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog),
                                                       "depth")) + 1);
      break;

    default:
      ctk_widget_destroy (CTK_WIDGET (dialog));
      break;
    }
}

static void
dialog_cb (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       callback_data)
{
  make_dialog (CTK_WIDGET (callback_data), 1);
}

static void
modal_dialog_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       callback_data)
{
  CtkWidget *dialog;

  dialog = ctk_message_dialog_new (CTK_WINDOW (callback_data),
                                   CTK_DIALOG_DESTROY_WITH_PARENT,
                                   CTK_MESSAGE_INFO,
                                   CTK_BUTTONS_CLOSE,
                                   "Here is a MODAL dialog");

  set_ctk_window_type (CTK_WINDOW (dialog), "_NET_WM_WINDOW_TYPE_MODAL_DIALOG");

  ctk_dialog_run (CTK_DIALOG (dialog));

  ctk_widget_destroy (dialog);
}

static void
no_parent_dialog_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       callback_data)
{
  make_dialog (NULL, 1);
}

static void
utility_cb (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *vbox;
  CtkWidget *button;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_UTILITY");
  ctk_window_set_title (CTK_WINDOW (window), "Utility");

  ctk_window_set_transient_for (CTK_WINDOW (window), CTK_WINDOW (callback_data));

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  button = ctk_button_new_with_mnemonic ("_A button");
  ctk_box_pack_start (CTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = ctk_button_new_with_mnemonic ("_B button");
  ctk_box_pack_start (CTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = ctk_button_new_with_mnemonic ("_C button");
  ctk_box_pack_start (CTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = ctk_button_new_with_mnemonic ("_D button");
  ctk_box_pack_start (CTK_BOX (vbox), button, FALSE, FALSE, 0);

  ctk_widget_show_all (window);
}

static void
toolbar_cb (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *vbox;
  CtkWidget *label;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_TOOLBAR");
  ctk_window_set_title (CTK_WINDOW (window), "Toolbar");

  ctk_window_set_transient_for (CTK_WINDOW (window), CTK_WINDOW (callback_data));

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  label = ctk_label_new ("FIXME this needs a resize grip, etc.");
  ctk_box_pack_start (CTK_BOX (vbox), label, FALSE, FALSE, 0);

  ctk_widget_show_all (window);
}

static void
menu_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *vbox;
  CtkWidget *label;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_MENU");
  ctk_window_set_title (CTK_WINDOW (window), "Menu");

  ctk_window_set_transient_for (CTK_WINDOW (window), CTK_WINDOW (callback_data));

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  label = ctk_label_new ("FIXME this isn't a menu.");
  ctk_box_pack_start (CTK_BOX (vbox), label, FALSE, FALSE, 0);

  ctk_widget_show_all (window);
}

static void
override_redirect_cb (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *vbox;
  CtkWidget *label;

  window = ctk_window_new (CTK_WINDOW_POPUP);
  ctk_window_set_title (CTK_WINDOW (window), "Override Redirect");

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  label = ctk_label_new ("This is an override\nredirect window\nand should not be managed");
  ctk_box_pack_start (CTK_BOX (vbox), label, FALSE, FALSE, 0);

  ctk_widget_show_all (window);
}

static void
border_only_cb (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *vbox;
  CtkWidget *label;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_border_only (CTK_WINDOW (window));
  ctk_window_set_title (CTK_WINDOW (window), "Border only");

  ctk_window_set_transient_for (CTK_WINDOW (window), CTK_WINDOW (callback_data));

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  label = ctk_label_new ("This window is supposed to have a border but no titlebar.");
  ctk_box_pack_start (CTK_BOX (vbox), label, FALSE, FALSE, 0);

  ctk_widget_show_all (window);
}

static gboolean
focus_in_event_cb (CtkWidget *window,
                   CdkEvent  *event,
                   gpointer   data)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (data);

  ctk_label_set_text (CTK_LABEL (widget), "Has focus");

  return TRUE;
}


static gboolean
focus_out_event_cb (CtkWidget *window,
                    CdkEvent  *event,
                    gpointer   data)
{
  CtkWidget *widget;

  widget = CTK_WIDGET (data);

  ctk_label_set_text (CTK_LABEL (widget), "Not focused");

  return TRUE;
}

static CtkWidget*
focus_label (CtkWidget *window)
{
  CtkWidget *label;

  label = ctk_label_new ("Not focused");

  g_signal_connect (G_OBJECT (window), "focus_in_event",
                    G_CALLBACK (focus_in_event_cb), label);

  g_signal_connect (G_OBJECT (window), "focus_out_event",
                    G_CALLBACK (focus_out_event_cb), label);

  return label;
}

static void
splashscreen_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *image;
  CtkWidget *vbox;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_SPLASH");
  ctk_window_set_title (CTK_WINDOW (window), "Splashscreen");

  vbox = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);

  image = ctk_image_new_from_icon_name ("dialog-information", CTK_ICON_SIZE_DIALOG);
  ctk_box_pack_start (CTK_BOX (vbox), image, FALSE, FALSE, 0);

  ctk_box_pack_start (CTK_BOX (vbox), focus_label (window), FALSE, FALSE, 0);

  ctk_container_add (CTK_CONTAINER (window), vbox);

  ctk_widget_show_all (window);
}

enum
{
  DOCK_TOP = 1,
  DOCK_BOTTOM = 2,
  DOCK_LEFT = 3,
  DOCK_RIGHT = 4,
  DOCK_ALL = 5
};

static void
make_dock (int type)
{
  CtkWidget *window;
  CtkWidget *image;
  CtkWidget *box;
  CtkWidget *button;

  g_return_if_fail (type != DOCK_ALL);

  box = NULL;
  switch (type)
    {
    case DOCK_LEFT:
    case DOCK_RIGHT:
      box = ctk_box_new (CTK_ORIENTATION_VERTICAL, 0);
      break;
    case DOCK_TOP:
    case DOCK_BOTTOM:
      box = ctk_box_new (CTK_ORIENTATION_HORIZONTAL, 0);
      break;
    case DOCK_ALL:
      break;
    }

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_DOCK");

  image = ctk_image_new_from_icon_name ("dialog-information", CTK_ICON_SIZE_DIALOG);
  ctk_box_pack_start (CTK_BOX (box), image, FALSE, FALSE, 0);

  ctk_box_pack_start (CTK_BOX (box), focus_label (window), FALSE, FALSE, 0);

  button = ctk_button_new_with_label ("Close");
  ctk_box_pack_start (CTK_BOX (box), button, FALSE, FALSE, 0);

  g_signal_connect_swapped (G_OBJECT (button), "clicked",
                            G_CALLBACK (ctk_widget_destroy), window);

  ctk_container_add (CTK_CONTAINER (window), box);

#define DOCK_SIZE 48
  switch (type)
    {
    case DOCK_LEFT:
      ctk_widget_set_size_request (window, DOCK_SIZE, 400);
      ctk_window_move (CTK_WINDOW (window), 0, 000);
      set_ctk_window_struts (window, DOCK_SIZE, 0, 0, 0);
      ctk_window_set_title (CTK_WINDOW (window), "LeftDock");
      break;
    case DOCK_RIGHT:
      ctk_widget_set_size_request (window, DOCK_SIZE, 400);
      ctk_window_move (CTK_WINDOW (window), WidthOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())) - DOCK_SIZE, 200);
      set_ctk_window_struts (window, 0, DOCK_SIZE, 0, 0);
      ctk_window_set_title (CTK_WINDOW (window), "RightDock");
      break;
    case DOCK_TOP:
      ctk_widget_set_size_request (window, 600, DOCK_SIZE);
      ctk_window_move (CTK_WINDOW (window), 76, 0);
      set_ctk_window_struts (window, 0, 0, DOCK_SIZE, 0);
      ctk_window_set_title (CTK_WINDOW (window), "TopDock");
      break;
    case DOCK_BOTTOM:
      ctk_widget_set_size_request (window, 600, DOCK_SIZE);
      ctk_window_move (CTK_WINDOW (window), 200, HeightOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())) - DOCK_SIZE);
      set_ctk_window_struts (window, 0, 0, 0, DOCK_SIZE);
      ctk_window_set_title (CTK_WINDOW (window), "BottomDock");
      break;
    case DOCK_ALL:
      break;
    }

  ctk_widget_show_all (window);
}

static void
dock_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       callback_data)
{
  guint callback_action;
  const gchar *name;

  g_object_get (G_OBJECT (action), "name", &name, NULL);

  if (!g_strcmp0 (name, "top-dock"))
    callback_action = DOCK_TOP;
  else if (!g_strcmp0 (name, "bottom-dock"))
    callback_action = DOCK_BOTTOM;
  else if (!g_strcmp0 (name, "left-dock"))
    callback_action = DOCK_LEFT;
  else if (!g_strcmp0 (name, "right-dock"))
    callback_action = DOCK_RIGHT;
  else if (!g_strcmp0 (name, "all-docks"))
    callback_action = DOCK_ALL;
  else
    return;

  if (callback_action == DOCK_ALL)
    {
      make_dock (DOCK_TOP);
      make_dock (DOCK_BOTTOM);
      make_dock (DOCK_LEFT);
      make_dock (DOCK_RIGHT);
    }
  else
    {
      make_dock (callback_action);
    }
}

static void
override_background_color (CtkWidget *widget,
                           CdkRGBA   *rgba)
{
  gchar          *css;
  CtkCssProvider *provider;

  provider = ctk_css_provider_new ();

  css = g_strdup_printf ("* { background-color: %s; }",
                         cdk_rgba_to_string (rgba));
  ctk_css_provider_load_from_data (provider, css, -1, NULL);
  g_free (css);

  ctk_style_context_add_provider (ctk_widget_get_style_context (widget),
                                  CTK_STYLE_PROVIDER (provider),
                                  CTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);
}

static void
desktop_cb (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       callback_data)
{
  CtkWidget *window;
  CtkWidget *label;
  CdkRGBA    desktop_color;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  set_ctk_window_type (CTK_WINDOW (window), "_NET_WM_WINDOW_TYPE_DESKTOP");
  ctk_window_set_title (CTK_WINDOW (window), "Desktop");
  ctk_widget_set_size_request (window,
                               WidthOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())),
                               HeightOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())));
  ctk_window_move (CTK_WINDOW (window), 0, 0);

  desktop_color.red = 0.32;
  desktop_color.green = 0.46;
  desktop_color.blue = 0.65;
  desktop_color.alpha = 1.0;

  override_background_color (window, &desktop_color);

  label = focus_label (window);

  ctk_container_add (CTK_CONTAINER (window), label);

  ctk_widget_show_all (window);
}

static void
sleep_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       data)
{
  sleep (1000);
}

static void
toggle_aspect_ratio (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       data)
{
  CtkWidget *window;
  CdkGeometry geom;
  CtkWidget *widget = CTK_WIDGET (data);

  if (aspect_on)
    {
      geom.min_aspect = 0;
      geom.max_aspect = 65535;
    }
  else
    {
      geom.min_aspect = 1.777778;
      geom.max_aspect = 1.777778;
    }

  aspect_on = !aspect_on;

  window = ctk_widget_get_ancestor (widget, CTK_TYPE_WINDOW);
  if (window)
    ctk_window_set_geometry_hints (CTK_WINDOW (window),
				   CTK_WIDGET (data),
				   &geom,
				   CDK_HINT_ASPECT);

}

static void
toggle_decorated_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       data)
{
  CtkWidget *window;
  window = ctk_widget_get_ancestor (data, CTK_TYPE_WINDOW);
  if (window)
    ctk_window_set_decorated (CTK_WINDOW (window),
                              !ctk_window_get_decorated (CTK_WINDOW (window)));
}

static void
clicked_toolbar_cb (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       data)
{
  CtkWidget *dialog;

  dialog = ctk_message_dialog_new (CTK_WINDOW (data),
                                   CTK_DIALOG_DESTROY_WITH_PARENT,
                                   CTK_MESSAGE_INFO,
                                   CTK_BUTTONS_CLOSE,
                                   "Clicking the toolbar buttons doesn't do anything");

  /* Close dialog on user response */
  g_signal_connect (G_OBJECT (dialog),
                    "response",
                    G_CALLBACK (ctk_widget_destroy),
                    NULL);

  ctk_widget_show (dialog);
}

static void
update_statusbar (CtkTextBuffer *buffer,
                  CtkStatusbar  *statusbar)
{
  gchar *msg;
  gint row, col;
  gint count;
  CtkTextIter iter;

  ctk_statusbar_pop (statusbar, 0); /* clear any previous message, underflow is allowed */

  count = ctk_text_buffer_get_char_count (buffer);

  ctk_text_buffer_get_iter_at_mark (buffer,
                                    &iter,
                                    ctk_text_buffer_get_insert (buffer));

  row = ctk_text_iter_get_line (&iter);
  col = ctk_text_iter_get_line_offset (&iter);

  msg = g_strdup_printf ("Cursor at row %d column %d - %d chars in document",
                         row, col, count);

  ctk_statusbar_push (statusbar, 0, msg);

  g_free (msg);
}

static void
mark_set_callback (CtkTextBuffer     *buffer,
                   const CtkTextIter *new_location,
                   CtkTextMark       *mark,
                   gpointer           data)
{
  update_statusbar (buffer, CTK_STATUSBAR (data));
}

static int window_count = 0;

static void
destroy_cb (CtkWidget *w, gpointer data)
{
  --window_count;
  if (window_count == 0)
    ctk_main_quit ();
}

static const gchar *xml =
  "<interface>"
    "<menu id='menubar'>"
      "<submenu>"
        "<attribute name='label'>Windows</attribute>"
        "<section>"
          "<item>"
            "<attribute name='label'>Dialog</attribute>"
            "<attribute name='action'>demo.dialog1</attribute>"
            "<attribute name='accel'>&lt;control&gt;d</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Modal dialog</attribute>"
            "<attribute name='action'>demo.dialog2</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Parentless dialog</attribute>"
            "<attribute name='action'>demo.dialog3</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Utility</attribute>"
            "<attribute name='action'>demo.utility</attribute>"
            "<attribute name='accel'>&lt;control&gt;u</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Splashscreen</attribute>"
            "<attribute name='action'>demo.splashscreen</attribute>"
            "<attribute name='accel'>&lt;control&gt;s</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Top dock</attribute>"
            "<attribute name='action'>demo.top-dock</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Bottom dock</attribute>"
            "<attribute name='action'>demo.bottom-dock</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Left dock</attribute>"
            "<attribute name='action'>demo.left-dock</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Right dock</attribute>"
            "<attribute name='action'>demo.right-dock</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>All docks</attribute>"
            "<attribute name='action'>demo.all-docks</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Desktop</attribute>"
            "<attribute name='action'>demo.desktop</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Menu</attribute>"
            "<attribute name='action'>demo.menu</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Toolbar</attribute>"
            "<attribute name='action'>demo.toolbar</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Override Redirect</attribute>"
            "<attribute name='action'>demo.override-redirect</attribute>"
          "</item>"
          "<item>"
            "<attribute name='label'>Border Only</attribute>"
            "<attribute name='action'>demo.border-only</attribute>"
          "</item>"
        "</section>"
      "</submenu>"
    "</menu>"
  "</interface>";

static GActionEntry demo_entries[] =
{
  /* menubar */
  { "dialog1",           dialog_cb,            NULL, NULL, NULL, {} },
  { "dialog2",           modal_dialog_cb,      NULL, NULL, NULL, {} },
  { "dialog3",           no_parent_dialog_cb,  NULL, NULL, NULL, {} },
  { "utility",           utility_cb,           NULL, NULL, NULL, {} },
  { "splashscreen",      splashscreen_cb,      NULL, NULL, NULL, {} },
  { "top-dock",          dock_cb,              NULL, NULL, NULL, {} },
  { "bottom-dock",       dock_cb,              NULL, NULL, NULL, {} },
  { "left-dock",         dock_cb,              NULL, NULL, NULL, {} },
  { "right-dock",        dock_cb,              NULL, NULL, NULL, {} },
  { "all-docks",         dock_cb,              NULL, NULL, NULL, {} },
  { "desktop",           desktop_cb,           NULL, NULL, NULL, {} },
  { "menu",              menu_cb,              NULL, NULL, NULL, {} },
  { "toolbar",           toolbar_cb,           NULL, NULL, NULL, {} },
  { "override-redirect", override_redirect_cb, NULL, NULL, NULL, {} },
  { "border-only",       border_only_cb,       NULL, NULL, NULL, {} },
  /* toolbar */
  { "new",               do_appwindow,         NULL, NULL, NULL, {} },
  { "lock",              sleep_cb,             NULL, NULL, NULL, {} },
  { "decorations",       toggle_decorated_cb,  NULL, NULL, NULL, {} },
  { "quit",              clicked_toolbar_cb,   NULL, NULL, NULL, {} },
  { "ratio",             toggle_aspect_ratio,  NULL, NULL, NULL, {} },
};

static CtkWidget *
create_toolbar (void)
{
  CtkWidget *toolbar;
  CtkToolItem *item;

  toolbar = ctk_toolbar_new ();

  item = ctk_tool_button_new (ctk_image_new_from_icon_name ("document-new", CTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  ctk_tool_item_set_tooltip_markup (item, "Open another one of these windows");
  ctk_actionable_set_action_name (CTK_ACTIONABLE (item), "demo.new");
  ctk_toolbar_insert (CTK_TOOLBAR (toolbar), item, -1);

  item = ctk_tool_button_new (ctk_image_new_from_icon_name ("document-open", CTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  ctk_tool_item_set_tooltip_markup (item, "This is a demo button that locks up the demo");
  ctk_actionable_set_action_name (CTK_ACTIONABLE (item), "demo.lock");
  ctk_toolbar_insert (CTK_TOOLBAR (toolbar), item, -1);

  item = ctk_tool_button_new (ctk_image_new_from_icon_name ("document-open", CTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  ctk_tool_item_set_tooltip_markup (item, "This is a demo button that toggles window decorations");
  ctk_actionable_set_action_name (CTK_ACTIONABLE (item), "demo.decorations");
  ctk_toolbar_insert (CTK_TOOLBAR (toolbar), item, -1);

  item = ctk_tool_button_new (ctk_image_new_from_icon_name ("document-open", CTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  ctk_tool_item_set_tooltip_markup (item, "This is a demo button that locks the aspect ratio using a hint");
  ctk_actionable_set_action_name (CTK_ACTIONABLE (item), "demo.ratio");
  ctk_toolbar_insert (CTK_TOOLBAR (toolbar), item, -1);

  item = ctk_tool_button_new (ctk_image_new_from_icon_name ("ctk-quit", CTK_ICON_SIZE_SMALL_TOOLBAR), NULL);
  ctk_tool_item_set_tooltip_markup (item, "This is a demo button with a 'quit' icon");
  ctk_actionable_set_action_name (CTK_ACTIONABLE (item), "demo.quit");
  ctk_toolbar_insert (CTK_TOOLBAR (toolbar), item, -1);

  return toolbar;
}

static void
do_appwindow (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  CtkWidget *window;
  CtkWidget *grid;
  CtkWidget *toolbar;
  GSimpleActionGroup *action_group;
  CtkBuilder *builder;
  CtkWidget *statusbar;
  CtkWidget *contents;
  CtkWidget *sw;
  CtkTextBuffer *buffer;

  /* Create the toplevel window
   */

  ++window_count;

  aspect_on = FALSE;

  window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
  ctk_window_set_title (CTK_WINDOW (window), "Application Window");

  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (destroy_cb), NULL);

  grid = ctk_grid_new ();

  ctk_widget_set_vexpand (grid, TRUE);
  ctk_widget_set_hexpand (grid, TRUE);

  ctk_container_add (CTK_CONTAINER (window), grid);

  action_group = g_simple_action_group_new ();
  builder = ctk_builder_new_from_string (xml, -1);

  g_action_map_add_action_entries (G_ACTION_MAP (action_group),
                                   demo_entries,
                                   G_N_ELEMENTS (demo_entries),
                                   window);
  ctk_widget_insert_action_group (window, "demo", G_ACTION_GROUP (action_group));

  /* Create the menubar
   */

  GMenuModel *model = G_MENU_MODEL (ctk_builder_get_object (builder, "menubar"));
  CtkWidget *menubar = ctk_menu_bar_new_from_model (model);
  ctk_grid_attach (CTK_GRID (grid), menubar, 0, 0, 1, 1);
  ctk_widget_set_hexpand (menubar, TRUE);

  /* Create the toolbar
   */

  toolbar = create_toolbar ();
  ctk_grid_attach (CTK_GRID (grid), toolbar, 0, 1, 1, 1);
  ctk_widget_set_hexpand (toolbar, TRUE);

  /* Create document
   */

  sw = ctk_scrolled_window_new (NULL, NULL);

  ctk_scrolled_window_set_policy (CTK_SCROLLED_WINDOW (sw),
                                  CTK_POLICY_AUTOMATIC,
                                  CTK_POLICY_AUTOMATIC);

  ctk_scrolled_window_set_shadow_type (CTK_SCROLLED_WINDOW (sw),
                                       CTK_SHADOW_IN);

  ctk_grid_attach (CTK_GRID (grid), sw, 0, 2, 1, 1);

  ctk_widget_set_hexpand (sw, TRUE);
  ctk_widget_set_vexpand (sw, TRUE);

  ctk_window_set_default_size (CTK_WINDOW (window),
                               200, 200);

  contents = ctk_text_view_new ();
  ctk_text_view_set_wrap_mode (CTK_TEXT_VIEW (contents),
                               PANGO_WRAP_WORD);

  ctk_container_add (CTK_CONTAINER (sw),
                     contents);

  /* Create statusbar */

  statusbar = ctk_statusbar_new ();
  ctk_grid_attach (CTK_GRID (grid), statusbar, 0, 3, 1, 1);
  ctk_widget_set_hexpand (statusbar, TRUE);

  /* Show text widget info in the statusbar */
  buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (contents));

  ctk_text_buffer_set_text (buffer,
                            "This demo demonstrates various kinds of windows that "
                            "window managers and window manager themes should handle. "
                            "Be sure to tear off the menu and toolbar, those are also "
                            "a special kind of window.",
                            -1);

  g_signal_connect_object (buffer,
                           "changed",
                           G_CALLBACK (update_statusbar),
                           statusbar,
                           0);

  g_signal_connect_object (buffer,
                           "mark_set", /* cursor moved */
                           G_CALLBACK (mark_set_callback),
                           statusbar,
                           0);

  update_statusbar (buffer, CTK_STATUSBAR (statusbar));

  ctk_widget_show_all (window);

  g_object_unref (action_group);
  g_object_unref (builder);
}
