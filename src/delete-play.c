/*
 * delete-play.c
 * A Totem plugin that adds the ability to delete the currently playing file.
 *
 *   Copyright (C) 2013 Christopher A. Doyle
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "config.h"

#include <glib/gstdio.h>

#include <totem-plugin.h>
#include "totem-interface.h"

#define TOTEM_TYPE_DELETEPLAY_PLUGIN         (totem_deleteplay_plugin_get_type())
#define TOTEM_DELETEPLAY_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TOTEM_TYPE_DELETEPLAY_PLUGIN, TotemDeleteplayPlugin))
#define TOTEM_DELETEPLAY_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST    ((k), TOTEM_TYPE_DELETEPLAY_PLUGIN, TotemDeleteplayPluginClass))
#define TOTEM_IS_DELETEPLAY_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TOTEM_TYPE_DELETEPLAY_PLUGIN))
#define TOTEM_IS_DELETEPLAY_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE    ((k), TOTEM_TYPE_DELETEPLAY_PLUGIN))
#define TOTEM_DELETEPLAY_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS  ((o), TOTEM_TYPE_DELETEPLAY_PLUGIN, TotemDeleteplayPluginClass))

#define ACTION_GROUP "DeleteplayActions"
#define ACTION_NAME  "Deleteplay"

typedef struct {
  TotemObject    *totem;
  char           *mrl;          /* media resource locater for the currently playing file */
  GtkActionGroup *action_group;
  guint           ui_merge_id;
} TotemDeleteplayPluginPrivate;

TOTEM_PLUGIN_REGISTER(TOTEM_TYPE_DELETEPLAY_PLUGIN, TotemDeleteplayPlugin, totem_deleteplay_plugin)

static void totem_deleteplay_plugin_delete(GtkAction *action, TotemDeleteplayPlugin *pi);

static GtkActionEntry totem_deleteplay_plugin_actions [] = {
  { ACTION_NAME,
    "gtk-delete",
    NULL,
    "<Ctrl>D",
    "Delete the currently playing file from the filesystem.",
    G_CALLBACK(totem_deleteplay_plugin_delete)
  },
};

/* Delete the currently playing file. */
static void
totem_deleteplay_plugin_delete(GtkAction *action, TotemDeleteplayPlugin *pi) {
  GFile       *file;
  char        *filename;
  GtkWidget   *dialog;
  TotemObject *totem = pi->priv->totem;

  if (pi->priv->mrl == NULL) {
    return;
  }

  file = g_file_new_for_uri(pi->priv->mrl);
  filename = g_file_get_parse_name(file);
  g_object_unref(file);

  if (filename == NULL) {
    return;
  }

  dialog = gtk_message_dialog_new(totem_get_main_window(totem),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_WARNING,
                                  GTK_BUTTONS_OK_CANCEL,
                                  "Do you really want to delete this item?\r\n%s", filename);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

  if (GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(dialog))) {
    totem_file_closed(totem);
    g_remove(filename);       // delete the current file
    /* If there are other files to play, then play them, otherwise exit. */
    if (totem_get_playlist_length(totem) > 1) {
      totem_action_next(totem);
    } else {
      totem_action_exit(totem);
    }
  }
  gtk_widget_destroy(dialog);

  g_free(filename);
}

/* Called when a file is opened. */
static void
totem_deleteplay_file_opened(TotemObject           *totem,
                             const char            *mrl,
                             TotemDeleteplayPlugin *pi)
{
  TotemDeleteplayPluginPrivate *priv   = pi->priv;
  GtkAction                    *action = NULL;

  /* free the old saved mrl if any */
  if (pi->priv->mrl != NULL) {
    g_free(pi->priv->mrl);
    pi->priv->mrl = NULL;
  }

  if (mrl == NULL) {
    return;
  }

  /* Make menu item sensitive if mrl starts with "file:".  This is the only
     type of file that this plugin can delete. */
  if (g_str_has_prefix(mrl, "file:")) {
    action = gtk_action_group_get_action(priv->action_group, ACTION_NAME);
    gtk_action_set_sensitive(action, TRUE);
    pi->priv->mrl = g_strdup(mrl);  /* save the mrl of the newly opened file */
  }
}

/* Called when a file is closed. */
static void
totem_deleteplay_file_closed(TotemObject           *totem,
                             TotemDeleteplayPlugin *pi) {
  GtkAction *action;

  /* free the saved mrl */
  g_free(pi->priv->mrl);
  pi->priv->mrl = NULL;

  /* make menu item inactive */
  action = gtk_action_group_get_action(pi->priv->action_group, ACTION_NAME);
  gtk_action_set_sensitive(action, FALSE);
}


/* Called when the plugin is activated.
   Totem calls this when either the user activates the plugin,
   or when totem starts up with the plugin already configured as active. */
static void
impl_activate(PeasActivatable *plugin) {
  TotemDeleteplayPlugin        *pi         = TOTEM_DELETEPLAY_PLUGIN(plugin);
  TotemDeleteplayPluginPrivate *priv       = pi->priv;
  GtkUIManager                 *ui_manager = NULL;
  GtkAction                    *action     = NULL;
  char                         *mrl        = NULL;

  priv->totem = g_object_get_data(G_OBJECT(plugin), "object");

  /* Connect the callbacks. */
  g_signal_connect(priv->totem,
                   "file-opened",
                   G_CALLBACK(totem_deleteplay_file_opened),
                   plugin);
  g_signal_connect(priv->totem,
                   "file-closed",
                   G_CALLBACK(totem_deleteplay_file_closed),
                   plugin);

  /* Create the GUI */
  priv->action_group = gtk_action_group_new(ACTION_GROUP);
  gtk_action_group_add_actions(priv->action_group,
                               totem_deleteplay_plugin_actions,
                               G_N_ELEMENTS(totem_deleteplay_plugin_actions),
                               pi);

  ui_manager = totem_get_ui_manager(priv->totem);
  gtk_ui_manager_insert_action_group(ui_manager, priv->action_group, -1);
  g_object_unref(priv->action_group);

  priv->ui_merge_id = gtk_ui_manager_new_merge_id(ui_manager);

  /* Create Menu->Delete */
  gtk_ui_manager_add_ui(ui_manager,
                        priv->ui_merge_id,
                        "/ui/tmw-menubar/movie/save-placeholder",
                        ACTION_NAME,
                        ACTION_NAME,
                        GTK_UI_MANAGER_MENUITEM,
                        TRUE);

  /* Add Delete to pop-up window */
  gtk_ui_manager_add_ui(ui_manager,
                        priv->ui_merge_id,
                        "/ui/totem-main-popup/save-placeholder",
                        ACTION_NAME,
                        ACTION_NAME,
                        GTK_UI_MANAGER_MENUITEM,
                        TRUE);

  /* Default the menu item to being insensitive. */
  action = gtk_action_group_get_action(priv->action_group, ACTION_NAME);
  gtk_action_set_sensitive(action, FALSE);

  /* Save the mrl for the currently playing file (if any) */
  mrl = totem_get_current_mrl(priv->totem);
  totem_deleteplay_file_opened(priv->totem, mrl, pi);
  g_free(mrl);
}

/* Called when the plugin is activated.
   Totem calls this when either the user deactivates the plugin,
   or when totem exits with the plugin configured as active. */
static void
impl_deactivate(PeasActivatable *plugin) {
  TotemDeleteplayPlugin        *pi         = TOTEM_DELETEPLAY_PLUGIN(plugin);
  TotemDeleteplayPluginPrivate *priv       = pi->priv;
  GtkUIManager                 *ui_manager = NULL;

  g_signal_handlers_disconnect_by_func(priv->totem, totem_deleteplay_file_opened, plugin);
  g_signal_handlers_disconnect_by_func(priv->totem, totem_deleteplay_file_closed, plugin);

  ui_manager = totem_get_ui_manager(priv->totem);
  gtk_ui_manager_remove_ui(ui_manager, priv->ui_merge_id);
  gtk_ui_manager_remove_action_group(ui_manager, priv->action_group);

  priv->totem = NULL;

  g_free(priv->mrl);
  priv->mrl = NULL;
}

