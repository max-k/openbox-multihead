#include "kernel/debug.h"
#include "kernel/menu.h"
#include "kernel/menuframe.h"
#include "kernel/screen.h"
#include "kernel/client.h"
#include "kernel/openbox.h"
#include "kernel/frame.h"
#include "gettext.h"

#include <glib.h>

#define CLIENT_MENU_NAME  "client-menu"
#define SEND_TO_MENU_NAME "client-send-to-menu"
#define LAYER_MENU_NAME   "client-layer-menu"

enum {
    LAYER_TOP,
    LAYER_NORMAL,
    LAYER_BOTTOM
};

enum {
    CLIENT_SEND_TO,
    CLIENT_LAYER,
    CLIENT_ICONIFY,
    CLIENT_MAXIMIZE,
    CLIENT_RAISE,
    CLIENT_LOWER,
    CLIENT_SHADE,
    CLIENT_DECORATE,
    CLIENT_MOVE,
    CLIENT_RESIZE,
    CLIENT_CLOSE
};

static void client_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    frame->show_title = FALSE;

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = !!frame->client;
    }

    if (!frame->client)
        return;

    e = menu_find_entry_id(menu, CLIENT_ICONIFY);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_ICONIFY;

    e = menu_find_entry_id(menu, CLIENT_MAXIMIZE);
    e->data.normal.enabled =frame->client->functions & OB_CLIENT_FUNC_MAXIMIZE;

    e = menu_find_entry_id(menu, CLIENT_SHADE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_SHADE;

    e = menu_find_entry_id(menu, CLIENT_MOVE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_MOVE;

    e = menu_find_entry_id(menu, CLIENT_RESIZE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_RESIZE;

    e = menu_find_entry_id(menu, CLIENT_CLOSE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_CLOSE;
}

static void layer_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = !!frame->client;
    }

    if (!frame->client)
        return;

    e = menu_find_entry_id(menu, LAYER_TOP);
    e->data.normal.enabled = !frame->client->above;

    e = menu_find_entry_id(menu, LAYER_NORMAL);
    e->data.normal.enabled = (frame->client->above || frame->client->below);

    e = menu_find_entry_id(menu, LAYER_BOTTOM);
    e->data.normal.enabled = !frame->client->below;
}

static void send_to_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;
    GSList *acts;
    ObAction *act;

    menu_clear_entries(menu);

    if (!frame->client)
        return;

    for (i = 0; i <= screen_num_desktops; ++i) {
        gchar *name;
        guint desk;

        if (i >= screen_num_desktops) {
            menu_add_separator(menu, -1);

            desk = DESKTOP_ALL;
            name = _("All desktops");
        } else {
            desk = i;
            name = screen_desktop_names[i];
        }

        act = action_from_string("SendToDesktop");
        act->data.sendto.desk = desk;
        act->data.sendto.follow = FALSE;
        acts = g_slist_prepend(NULL, act);
        menu_add_normal(menu, desk, name, acts);

        if (frame->client->desktop == desk) {
            ObMenuEntry *e = menu_find_entry_id(menu, desk);
            g_assert(e);
            e->data.normal.enabled = FALSE;
        }
    }
}

void client_menu_startup()
{
    GSList *acts;
    ObMenu *menu;

    menu = menu_new(LAYER_MENU_NAME, _("Layer"), NULL);
    menu_set_update_func(menu, layer_update);

    acts = g_slist_prepend(NULL, action_from_string("SendToTopLayer"));
    menu_add_normal(menu, LAYER_TOP, _("Always on top"), acts);

    acts = g_slist_prepend(NULL, action_from_string("SendToNormalLayer"));
    menu_add_normal(menu, LAYER_NORMAL, _("Normal"), acts);

    acts = g_slist_prepend(NULL, action_from_string("SendToBottomLayer"));
    menu_add_normal(menu, LAYER_BOTTOM, _("Always on bottom"),acts);


    menu = menu_new(SEND_TO_MENU_NAME, _("Send to desktop"), NULL);
    menu_set_update_func(menu, send_to_update);


    menu = menu_new(CLIENT_MENU_NAME, _("Client menu"), NULL);
    menu_set_update_func(menu, client_update);

    menu_add_submenu(menu, CLIENT_SEND_TO, SEND_TO_MENU_NAME);

    menu_add_submenu(menu, CLIENT_LAYER, LAYER_MENU_NAME);

    acts = g_slist_prepend(NULL, action_from_string("Iconify"));
    menu_add_normal(menu, CLIENT_ICONIFY, _("Iconify"), acts);

    acts = g_slist_prepend(NULL, action_from_string("ToggleMaximizeFull"));
    menu_add_normal(menu, CLIENT_MAXIMIZE, _("Maximize"), acts);

    acts = g_slist_prepend(NULL, action_from_string("Raise"));
    menu_add_normal(menu, CLIENT_RAISE, _("Raise to top"), acts);

    acts = g_slist_prepend(NULL, action_from_string("Lower"));
    menu_add_normal(menu, CLIENT_LOWER, _("Lower to bottom"),acts);

    acts = g_slist_prepend(NULL, action_from_string("ToggleShade"));
    menu_add_normal(menu, CLIENT_SHADE, _("Roll up/down"), acts);

    acts = g_slist_prepend(NULL, action_from_string("ToggleDecorations"));
    menu_add_normal(menu, CLIENT_DECORATE, _("Decorate"), acts);

    menu_add_separator(menu, -1);

    acts = g_slist_prepend(NULL, action_from_string("KeyboardMove"));
    menu_add_normal(menu, CLIENT_MOVE, _("Move"), acts);

    acts = g_slist_prepend(NULL, action_from_string("KeyboardResize"));
    menu_add_normal(menu, CLIENT_RESIZE, _("Resize"), acts);

    menu_add_separator(menu, -1);

    acts = g_slist_prepend(NULL, action_from_string("Close"));
    menu_add_normal(menu, CLIENT_CLOSE, _("Close"), acts);
}