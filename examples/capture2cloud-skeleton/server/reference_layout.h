/* Extracted from Capture2Cloud, commit 35e0d43; dashboard allocation guard removed for standalone use. */
static int wozt_dashboard_row_extra(int surplus, gint64 before,
                                    gint64 after, gint64 total)
{
    if (surplus <= 0 || total <= 0) return 0;
    return (int)((gint64)surplus * after / total -
                 (gint64)surplus * before / total);
}

static void wozt_dashboard_allocate_height(GtkWidget *dashboard,
                                            GtkAllocation *allocation,
                                            gpointer userdata)
{
    (void)userdata;
    GList *rows = gtk_container_get_children(GTK_CONTAINER(dashboard));
    GtkWidget *lines[8];
    GtkAllocation cells[8];
    int heights[8];
    guint count = 0;
    gint64 total = 0;
    for (GList *it = rows; it; it = it->next) {
        GtkWidget *line = GTK_WIDGET(it->data);
        if (!gtk_widget_get_visible(line)) continue;
        if (!GTK_IS_BOX(line) || count == G_N_ELEMENTS(lines)) {
            g_list_free(rows);
            return;
        }
        lines[count] = line;
        gtk_widget_get_allocation(line, &cells[count]);
        gint minimum = 0, natural = 0;
        gtk_widget_get_preferred_height_for_width(line,
            MAX(1, cells[count].width), &minimum, &natural);
        heights[count] = MAX(1, MAX(minimum, natural));
        total += heights[count++];
    }
    g_list_free(rows);
    if (!count) return;
    const int spacing = gtk_box_get_spacing(GTK_BOX(dashboard));
    const gint64 available = (gint64)allocation->height -
                            (gint64)spacing * (count - 1);
    if (available <= total) return; /* GTK handles tight/scrolled layouts. */
    const int surplus = (int)(available - total);
    gint64 before = 0;
    int y = cells[0].y;
    for (guint i = 0; i < count; ++i) {
        GtkAllocation cell = cells[i];
        const gint64 after = before + heights[i];
        cell.y = y;
        cell.height = heights[i] +
            wozt_dashboard_row_extra(surplus, before, after, total);
        gtk_widget_size_allocate(lines[i], &cell);
        y += cell.height + spacing;
        before = after;
    }
}

static gboolean wozt_resize_drag(GtkWidget *widget, GdkEventButton *event,
                                  gpointer user_data)
{
    (void)widget;
    if (event->button != 1 || event->type != GDK_BUTTON_PRESS) return FALSE;
    GtkWidget *win = g_object_get_data(G_OBJECT(widget), "wozt-window");
    if (!win || gtk_window_is_maximized(GTK_WINDOW(win))) return FALSE;
    gtk_window_begin_resize_drag(GTK_WINDOW(win), (GdkWindowEdge)GPOINTER_TO_INT(user_data),
                                 (gint)event->button,
                                 (gint)event->x_root, (gint)event->y_root,
                                 event->time);
    return TRUE;
}

static void wozt_resize_realized(GtkWidget *widget, gpointer user_data)
{
    static const char *const CURSORS[8] = {
        "nw-resize", "n-resize", "ne-resize", "w-resize",
        "e-resize", "sw-resize", "s-resize", "se-resize"
    };
    int index = GPOINTER_TO_INT(user_data);
    if (index < 0 || index >= 8) return;
    GdkDisplay *display = gtk_widget_get_display(widget);
    GdkCursor *cursor = gdk_cursor_new_from_name(display, CURSORS[index]);
    if (cursor) {
        GdkWindow *window = gtk_widget_get_window(widget);
        if (window) gdk_window_set_cursor(window, cursor);
        g_object_unref(cursor);
    }
}

static void wozt_add_resize_zones(GtkWidget *overlay, GtkWidget *window)
{
    static const GdkWindowEdge EDGES[8] = {
        GDK_WINDOW_EDGE_NORTH_WEST, GDK_WINDOW_EDGE_NORTH,
        GDK_WINDOW_EDGE_NORTH_EAST, GDK_WINDOW_EDGE_WEST,
        GDK_WINDOW_EDGE_EAST, GDK_WINDOW_EDGE_SOUTH_WEST,
        GDK_WINDOW_EDGE_SOUTH, GDK_WINDOW_EDGE_SOUTH_EAST
    };
    for (int i = 0; i < 8; ++i) {
        GtkWidget *zone = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(zone), TRUE);
        gtk_widget_add_events(zone, GDK_BUTTON_PRESS_MASK);
        gtk_style_context_add_class(gtk_widget_get_style_context(zone), "wozt-resize");
        const gboolean left = i == 0 || i == 3 || i == 5;
        const gboolean right = i == 2 || i == 4 || i == 7;
        const gboolean top = i <= 2;
        const gboolean bottom = i >= 5;
        const gboolean corner = i == 0 || i == 2 || i == 5 || i == 7;
        gtk_widget_set_halign(zone, left ? GTK_ALIGN_START :
                                   right ? GTK_ALIGN_END : GTK_ALIGN_FILL);
        gtk_widget_set_valign(zone, top ? GTK_ALIGN_START :
                                   bottom ? GTK_ALIGN_END : GTK_ALIGN_FILL);
        gtk_widget_set_size_request(zone, corner ? 12 : (left || right ? 5 : -1),
                                          corner ? 12 : (top || bottom ? 5 : -1));
        g_object_set_data(G_OBJECT(zone), "wozt-window", window);
        g_signal_connect(zone, "realize", G_CALLBACK(wozt_resize_realized),
                         GINT_TO_POINTER(i));
        g_signal_connect(zone, "button-press-event", G_CALLBACK(wozt_resize_drag),
                         GINT_TO_POINTER(EDGES[i]));
        gtk_overlay_add_overlay(GTK_OVERLAY(overlay), zone);
    }
}

