/* Croma window menu */

/*
 * Copyright (C) 2001 Havoc Pennington
 * Copyright (C) 2004 Rob Adams
 * Copyright (C) 2005 Elijah Newren
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

#include <cdk/cdkx.h>
#include <config.h>
#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "main.h"
#include "util.h"
#include "core.h"
#include "metaaccellabel.h"
#include "ui.h"

typedef struct _MenuItem MenuItem;
typedef struct _MenuData MenuData;

typedef enum {
	MENU_ITEM_SEPARATOR = 0,
	MENU_ITEM_NORMAL,
	MENU_ITEM_IMAGE,
	MENU_ITEM_CHECKBOX,
	MENU_ITEM_RADIOBUTTON,
	MENU_ITEM_WORKSPACE_LIST,
} MetaMenuItemType;

struct _MenuItem {
	MetaMenuOp op;
	MetaMenuItemType type;
	const char* stock_id;
	const gboolean checked;
	const char* label;
};


struct _MenuData {
	MetaWindowMenu* menu;
	MetaMenuOp op;
};

static void activate_cb(CtkWidget* menuitem, gpointer data);

static MenuItem menuitems[] = {
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MINIMIZE, MENU_ITEM_IMAGE, CROMA_STOCK_MINIMIZE, FALSE, N_("Mi_nimize")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MAXIMIZE, MENU_ITEM_IMAGE, CROMA_STOCK_MAXIMIZE, FALSE, N_("Ma_ximize")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_UNMAXIMIZE, MENU_ITEM_IMAGE, CROMA_STOCK_RESTORE, FALSE, N_("Unma_ximize")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_SHADE, MENU_ITEM_NORMAL, NULL, FALSE, N_("Roll _Up")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_UNSHADE, MENU_ITEM_NORMAL, NULL, FALSE, N_("_Unroll")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MOVE, MENU_ITEM_NORMAL, NULL, FALSE, N_("_Move") },
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_RESIZE, MENU_ITEM_NORMAL, NULL, FALSE, N_("_Resize")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_RECOVER, MENU_ITEM_NORMAL, NULL, FALSE, N_("Move Titlebar On_screen")},
	{META_MENU_OP_WORKSPACES, MENU_ITEM_SEPARATOR, NULL, FALSE, NULL}, /* separator */
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_ABOVE, MENU_ITEM_CHECKBOX, NULL, FALSE, N_("Always on _Top")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_UNABOVE, MENU_ITEM_CHECKBOX, NULL, TRUE, N_("Always on _Top")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_STICK, MENU_ITEM_RADIOBUTTON, NULL, FALSE, N_("_Always on Visible Workspace")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_UNSTICK, MENU_ITEM_RADIOBUTTON, NULL, FALSE,  N_("_Only on This Workspace")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MOVE_LEFT, MENU_ITEM_NORMAL, NULL, FALSE, N_("Move to Workspace _Left")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MOVE_RIGHT, MENU_ITEM_NORMAL, NULL, FALSE, N_("Move to Workspace R_ight")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MOVE_UP, MENU_ITEM_NORMAL, NULL, FALSE, N_("Move to Workspace _Up")},
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_MOVE_DOWN, MENU_ITEM_NORMAL, NULL, FALSE, N_("Move to Workspace _Down")},
	{0, MENU_ITEM_WORKSPACE_LIST, NULL, FALSE, NULL},
	{0, MENU_ITEM_SEPARATOR, NULL, FALSE, NULL}, /* separator */
	/* Translators: Translate this string the same way as you do in libwnck! */
	{META_MENU_OP_DELETE, MENU_ITEM_IMAGE, CROMA_STOCK_DELETE, FALSE, N_("_Close")}
};

static void popup_position_func(CtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data)
{
	CtkRequisition req;
	CdkPoint* pos;

	pos = user_data;

	ctk_widget_get_preferred_size (CTK_WIDGET (menu), &req, NULL);

	*x = pos->x;
	*y = pos->y;

	if (meta_ui_get_direction() == META_UI_DIRECTION_RTL)
	{
		*x = MAX (0, *x - req.width);
	}

	/* Ensure onscreen */
	*x = CLAMP (*x, 0, MAX(0, WidthOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())) - req.width));
	*y = CLAMP (*y, 0, MAX(0, HeightOfScreen (cdk_x11_screen_get_xscreen (cdk_screen_get_default ())) - req.height));
}

static void menu_closed(CtkMenu* widget, gpointer data)
{
	MetaWindowMenu *menu;

	menu = data;

	meta_frames_notify_menu_hide (menu->frames);

	(*menu->func)(
		menu,
		CDK_DISPLAY_XDISPLAY(cdk_display_get_default()),
		menu->client_xwindow,
		ctk_get_current_event_time (),
		0, 0,
		menu->data);

	/* menu may now be freed */
}

static void activate_cb(CtkWidget* menuitem, gpointer data)
{
  MenuData* md;

  g_return_if_fail (CTK_IS_WIDGET (menuitem));

  md = data;

  meta_frames_notify_menu_hide (md->menu->frames);

	(*md->menu->func)(
		md->menu,
		CDK_DISPLAY_XDISPLAY (cdk_display_get_default()),
		md->menu->client_xwindow,
		ctk_get_current_event_time(),
		md->op,
		GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menuitem), "workspace")),
		md->menu->data);

  /* menu may now be freed */
}

/*
 * Given a Display and an index, get the workspace name and add any
 * accelerators. At the moment this means adding a _ if the name is of
 * the form "Workspace n" where n is less than 10, and escaping any
 * other '_'s so they do not create inadvertant accelerators.
 *
 * The calling code owns the string, and is reponsible to free the
 * memory after use.
 *
 * See also http://mail.gnome.org/archives/cafe-i18n/2008-March/msg00380.html
 * which discusses possible i18n concerns.
 */
static char*
get_workspace_name_with_accel (Display *display,
                               Window   xroot,
                               int      index)
{
  const char *name;
  int number;
  int charcount=0;

  name = meta_core_get_workspace_name_with_index (display, xroot, index);

  g_assert (name != NULL);

  /*
   * If the name is of the form "Workspace x" where x is an unsigned
   * integer, insert a '_' before the number if it is less than 10 and
   * return it
   */
  number = 0;
  if (sscanf (name, _("Workspace %d%n"), &number, &charcount) != 0 &&
      *(name + charcount)=='\0')
    {
      char *new_name;

      /*
       * Above name is a pointer into the Workspace struct. Here we make
       * a copy copy so we can have our wicked way with it.
       */
      if (number == 10)
        new_name = g_strdup_printf (_("Workspace 1_0"));
      else
        new_name = g_strdup_printf (_("Workspace %s%d"),
                                    number < 10 ? "_" : "",
                                    number);
      return new_name;
    }
  else
    {
      /*
       * Otherwise this is just a normal name. Escape any _ characters so that
       * the user's workspace names do not get mangled.  If the number is less
       * than 10 we provide an accelerator.
       */
      char *new_name;
      const char *source;
      char *dest;

      /*
       * Assume the worst case, that every character is a _.  We also
       * provide memory for " (_#)"
       */
      new_name = g_malloc0 (strlen (name) * 2 + 6 + 1);

      /*
       * Now iterate down the strings, adding '_' to escape as we go
       */
      dest = new_name;
      source = name;
      while (*source != '\0')
        {
          if (*source == '_')
            *dest++ = '_';
          *dest++ = *source++;
        }

      /* People don't start at workspace 0, but workspace 1 */
      if (index < 9)
        {
          g_snprintf (dest, 6, " (_%d)", index + 1);
        }
      else if (index == 9)
        {
          g_snprintf (dest, 6, " (_0)");
        }

      return new_name;
    }
}

static CtkWidget* menu_item_new(MenuItem* menuitem, int workspace_id)
{
	unsigned int key;
	MetaVirtualModifier mods;
	const char* i18n_label;
	CtkWidget* mi;
	CtkWidget* accel_label;

	if (menuitem->type == MENU_ITEM_NORMAL)
	{
		mi = ctk_menu_item_new ();
	}
	else if (menuitem->type == MENU_ITEM_IMAGE)
	{
		CtkWidget* image = ctk_image_new_from_icon_name(menuitem->stock_id, CTK_ICON_SIZE_MENU);

		mi = ctk_image_menu_item_new();

		ctk_image_menu_item_set_image(CTK_IMAGE_MENU_ITEM(mi), image);
		ctk_widget_show(image);
	}
	else if (menuitem->type == MENU_ITEM_CHECKBOX)
	{
		mi = ctk_check_menu_item_new ();

		ctk_check_menu_item_set_active(CTK_CHECK_MENU_ITEM(mi), menuitem->checked);
    }
	else if (menuitem->type == MENU_ITEM_RADIOBUTTON)
	{
		mi = ctk_check_menu_item_new ();

		ctk_check_menu_item_set_draw_as_radio (CTK_CHECK_MENU_ITEM (mi), TRUE);
		ctk_check_menu_item_set_active (CTK_CHECK_MENU_ITEM (mi), menuitem->checked);
	}
	else if (menuitem->type == MENU_ITEM_WORKSPACE_LIST)
	{
		return NULL;
	}
	else
	{
		return ctk_separator_menu_item_new();
	}

	i18n_label = _(menuitem->label);
	meta_core_get_menu_accelerator (menuitem->op, workspace_id, &key, &mods);

	accel_label = meta_accel_label_new_with_mnemonic (i18n_label);
	ctk_widget_set_halign (accel_label, CTK_ALIGN_START);

	ctk_container_add (CTK_CONTAINER (mi), accel_label);
	ctk_widget_show (accel_label);

	meta_accel_label_set_accelerator (META_ACCEL_LABEL (accel_label), key, mods);

	g_object_set (ctk_settings_get_default (), "ctk-menu-images", TRUE, NULL);

	return mi;
}

MetaWindowMenu*
meta_window_menu_new   (MetaFrames         *frames,
                        MetaMenuOp          ops,
                        MetaMenuOp          insensitive,
                        Window              client_xwindow,
                        unsigned long       active_workspace,
                        int                 n_workspaces,
                        MetaWindowMenuFunc  func,
                        gpointer            data)
{
  int i;
  MetaWindowMenu *menu;

  /* FIXME: Modifications to 'ops' should happen in meta_window_show_menu */
  if (n_workspaces < 2)
    ops &= ~(META_MENU_OP_STICK | META_MENU_OP_UNSTICK | META_MENU_OP_WORKSPACES);
  else if (n_workspaces == 2)
    /* #151183: If we only have two workspaces, disable the menu listing them. */
    ops &= ~(META_MENU_OP_WORKSPACES);

  menu = g_new (MetaWindowMenu, 1);
  menu->frames = frames;
  menu->client_xwindow = client_xwindow;
  menu->func = func;
  menu->data = data;
  menu->ops = ops;
  menu->insensitive = insensitive;

  menu->menu = ctk_menu_new ();

  ctk_menu_set_screen (CTK_MENU (menu->menu),
                       ctk_widget_get_screen (CTK_WIDGET (frames)));

  for (i = 0; i < (int) G_N_ELEMENTS (menuitems); i++)
    {
      MenuItem menuitem = menuitems[i];
      if (ops & menuitem.op || menuitem.op == 0)
        {
          CtkWidget *mi;
          MenuData *md;
          unsigned int key;
          MetaVirtualModifier mods;

          mi = menu_item_new (&menuitem, -1);

          /* Set the activeness of radiobuttons. */
          switch (menuitem.op)
            {
            case META_MENU_OP_STICK:
              ctk_check_menu_item_set_active (CTK_CHECK_MENU_ITEM (mi),
                                              active_workspace == 0xFFFFFFFF);
              break;
            case META_MENU_OP_UNSTICK:
              ctk_check_menu_item_set_active (CTK_CHECK_MENU_ITEM (mi),
                                              active_workspace != 0xFFFFFFFF);
              break;
            default:
              break;
            }

          if (menuitem.type == MENU_ITEM_WORKSPACE_LIST)
            {
              if (ops & META_MENU_OP_WORKSPACES)
                {
                  Display *display;
                  Window xroot;
                  CdkScreen *screen;
                  CdkWindow *window;
                  CtkWidget *submenu;
                  int j;

                  MenuItem to_another_workspace = {
                    0, MENU_ITEM_NORMAL,
                    NULL, FALSE,
                    N_("Move to Another _Workspace")
                  };

                  meta_verbose ("Creating %d-workspace menu current space %lu\n",
                      n_workspaces, active_workspace);

                  window = ctk_widget_get_window (CTK_WIDGET (frames));

                  display = CDK_WINDOW_XDISPLAY (window);

                  screen = cdk_window_get_screen (window);
                  xroot = CDK_WINDOW_XID (cdk_screen_get_root_window (screen));

                  submenu = ctk_menu_new ();

                  g_assert (mi==NULL);
                  mi = menu_item_new (&to_another_workspace, -1);
                  ctk_menu_item_set_submenu (CTK_MENU_ITEM (mi), submenu);

                  for (j = 0; j < n_workspaces; j++)
                    {
                      char *label;
                      MenuData *md;
                      unsigned int key;
                      MetaVirtualModifier mods;
                      MenuItem moveitem;
                      CtkWidget *submi;

                      meta_core_get_menu_accelerator (META_MENU_OP_WORKSPACES,
                          j + 1,
                          &key, &mods);

                      label = get_workspace_name_with_accel (display, xroot, j);

                      moveitem.type = MENU_ITEM_NORMAL;
                      moveitem.op = META_MENU_OP_WORKSPACES;
                      moveitem.label = label;
                      submi = menu_item_new (&moveitem, j + 1);

                      g_free (label);

                      if ((active_workspace == (unsigned)j) && (ops & META_MENU_OP_UNSTICK))
                        ctk_widget_set_sensitive (submi, FALSE);

                      md = g_new (MenuData, 1);

                      md->menu = menu;
                      md->op = META_MENU_OP_WORKSPACES;

                      g_object_set_data (G_OBJECT (submi),
                          "workspace",
                          GINT_TO_POINTER (j));

                      g_signal_connect_data (G_OBJECT (submi),
                          "activate",
                          G_CALLBACK (activate_cb),
                          md,
                          (GClosureNotify) g_free, 0);

                      ctk_menu_shell_append (CTK_MENU_SHELL (submenu), submi);

                      ctk_widget_show (submi);
                    }
                  }
                else
                  meta_verbose ("not creating workspace menu\n");
            }
          else if (menuitem.type != MENU_ITEM_SEPARATOR)
            {
              meta_core_get_menu_accelerator (menuitems[i].op, -1,
                                              &key, &mods);

              if (insensitive & menuitem.op)
                ctk_widget_set_sensitive (mi, FALSE);

              md = g_new (MenuData, 1);

              md->menu = menu;
              md->op = menuitem.op;

              g_signal_connect_data (G_OBJECT (mi),
                                     "activate",
                                     G_CALLBACK (activate_cb),
                                     md,
                                     (GClosureNotify) g_free, 0);
            }

          if (mi)
            {
              ctk_menu_shell_append (CTK_MENU_SHELL (menu->menu), mi);

              ctk_widget_show (mi);
            }
        }
    }

  g_signal_connect (menu->menu, "selection_done",
                    G_CALLBACK (menu_closed), menu);

  return menu;
}

void meta_window_menu_popup(MetaWindowMenu* menu, int root_x, int root_y, int button, guint32 timestamp)
{
	CdkPoint* pt = g_new(CdkPoint, 1);
	gint scale;

	g_object_set_data_full(G_OBJECT(menu->menu), "destroy-point", pt, g_free);

	scale = ctk_widget_get_scale_factor (menu->menu);
	pt->x = root_x / scale;
	pt->y = root_y / scale;

	ctk_menu_popup(CTK_MENU (menu->menu), NULL, NULL, popup_position_func, pt, button, timestamp);

    if (!ctk_widget_get_visible (menu->menu))
      meta_warning("CtkMenu failed to grab the pointer\n");
}

void meta_window_menu_free(MetaWindowMenu* menu)
{
	ctk_widget_destroy(menu->menu);
	g_free(menu);
}
