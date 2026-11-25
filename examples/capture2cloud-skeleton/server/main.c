#include <gtk/gtk.h>
#include <string.h>
#include "wozt_ui_standard.h"
#include "reference_layout.h"

static GtkWidget *win, *dashboard, *frames[8], *status;
static WoztUiStandard *ui;
static int signature=-1;
static gboolean loading=FALSE;
static gboolean self_test=FALSE;
static GtkWidget *zones[8];
static gchar *config_path;
static int exit_status=0;
static GtkWidget *frame(const char *name,const char *icon,GtkWidget **grid) {
    GtkWidget *f=gtk_frame_new(NULL), *h=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4);
    gtk_style_context_add_class(gtk_widget_get_style_context(f),"wozt-section");
    gtk_box_pack_start(GTK_BOX(h),gtk_image_new_from_icon_name(icon,GTK_ICON_SIZE_MENU),FALSE,FALSE,0);
    GtkWidget *t=gtk_label_new(name);
    gtk_style_context_add_class(gtk_widget_get_style_context(t),"wozt-section-title");
    gtk_box_pack_start(GTK_BOX(h),t,FALSE,FALSE,0); gtk_frame_set_label_widget(GTK_FRAME(f),h);
    GtkWidget *outer=gtk_box_new(GTK_ORIENTATION_VERTICAL,3);
    gtk_widget_set_margin_start(outer,5);gtk_widget_set_margin_end(outer,5);
    gtk_widget_set_margin_top(outer,2);gtk_widget_set_margin_bottom(outer,3);
    *grid=gtk_grid_new();gtk_grid_set_row_spacing(GTK_GRID(*grid),2);gtk_grid_set_column_spacing(GTK_GRID(*grid),5);
    gtk_container_add(GTK_CONTAINER(f),outer);gtk_box_pack_start(GTK_BOX(outer),*grid,FALSE,FALSE,0);
    return f;
}
static void add_at(GtkWidget *g,int c,int r,const char *label,GtkWidget *w) {
    GtkWidget *l=gtk_label_new(label);gtk_widget_set_halign(l,GTK_ALIGN_START);
    gtk_label_set_single_line_mode(GTK_LABEL(l),TRUE);gtk_label_set_ellipsize(GTK_LABEL(l),PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(l),18);gtk_widget_set_tooltip_text(l,label);
    gtk_grid_attach(GTK_GRID(g),l,c,r,1,1);gtk_widget_set_halign(w,GTK_ALIGN_START);gtk_grid_attach(GTK_GRID(g),w,c+1,r,1,1);
}
static void add(GtkWidget *g,int r,const char *label,GtkWidget *w){add_at(g,0,r,label,w);}
static GtkWidget *scale(void) {
    GtkWidget *s=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,100,1);
    gtk_scale_set_value_pos(GTK_SCALE(s),GTK_POS_RIGHT);gtk_range_set_value(GTK_RANGE(s),50);
    gtk_widget_set_size_request(s,116,15);gtk_widget_set_margin_top(s,0);gtk_widget_set_margin_bottom(s,0);
    gtk_style_context_add_class(gtk_widget_get_style_context(s),"wozt-calibration-scale");return s;
}
static void combo_tip(GtkComboBox *c,gpointer p){(void)p;gchar *v=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(c));gtk_widget_set_tooltip_text(GTK_WIDGET(c),v?v:"");g_free(v);}
static GtkWidget *combo(const char *a,const char *b,int width) {
    GtkWidget *c=gtk_combo_box_text_new();gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c),a);gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c),b);
    gtk_combo_box_set_active(GTK_COMBO_BOX(c),0);gtk_widget_set_size_request(c,width,-1);
    GList *cells=gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(c));for(GList *i=cells;i;i=i->next) if(GTK_IS_CELL_RENDERER_TEXT(i->data)) {
        gtk_cell_renderer_set_fixed_size(GTK_CELL_RENDERER(i->data),width-24,-1);g_object_set(i->data,"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    }g_list_free(cells);g_signal_connect(c,"changed",G_CALLBACK(combo_tip),NULL);combo_tip(GTK_COMBO_BOX(c),NULL);return c;
}
static int width(GtkWidget *w) {gint m,n;gtk_widget_get_preferred_width(w,&m,&n);return MAX(m,n);}
static GtkWidget *line(void) {
    GtkWidget *l=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);gtk_widget_set_hexpand(l,TRUE);gtk_widget_set_vexpand(l,FALSE);
    gtk_widget_set_valign(l,GTK_ALIGN_FILL);gtk_box_pack_start(GTK_BOX(dashboard),l,FALSE,FALSE,0);return l;
}
static void pack(GtkWidget *l,GtkWidget *f,gboolean grow) {
    gtk_widget_set_halign(f,grow?GTK_ALIGN_FILL:GTK_ALIGN_START);gtk_widget_set_valign(f,GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(f,grow);gtk_widget_set_vexpand(f,FALSE);gtk_box_pack_start(GTK_BOX(l),f,grow,grow,0);
}
static void reflow(int w) {
    int available=MAX(1,w-30),a=width(frames[0]),b=width(frames[1]),c=MAX(width(frames[4]),width(frames[5]));
    int top=available>=a+b+c+12?3:available>=a+b+6?2:1;
    int mid=available>=width(frames[2])+width(frames[3])+6;
    int bottom=available>=3*MAX((width(frames[6])+1)/2,width(frames[7]))+12;
    int sig=top+10*mid+100*bottom;if(sig==signature)return;signature=sig;
    for(int i=0;i<8;i++){g_object_ref(frames[i]);GtkWidget *p=gtk_widget_get_parent(frames[i]);if(p)gtk_container_remove(GTK_CONTAINER(p),frames[i]);}
    GList *old=gtk_container_get_children(GTK_CONTAINER(dashboard));for(GList *i=old;i;i=i->next)gtk_container_remove(GTK_CONTAINER(dashboard),i->data);g_list_free(old);
    GtkWidget *l=line();pack(l,frames[0],top==1);if(top==1)l=line();pack(l,frames[1],TRUE);
    if(top<3)l=line();
    GtkWidget *col=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);gtk_box_pack_start(GTK_BOX(l),col,top<3,top<3,0);
    gtk_widget_set_valign(frames[5],GTK_ALIGN_FILL);gtk_widget_set_valign(frames[4],GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(col),frames[5],TRUE,TRUE,0);gtk_box_pack_start(GTK_BOX(col),frames[4],FALSE,TRUE,0);
    l=line();pack(l,frames[2],!mid);if(!mid)l=line();pack(l,frames[3],TRUE);
    l=line();if(bottom){GtkWidget *g=gtk_grid_new();gtk_grid_set_column_homogeneous(GTK_GRID(g),TRUE);gtk_grid_set_column_spacing(GTK_GRID(g),6);
        gtk_widget_set_halign(frames[6],GTK_ALIGN_FILL);gtk_widget_set_halign(frames[7],GTK_ALIGN_FILL);gtk_widget_set_valign(frames[6],GTK_ALIGN_FILL);gtk_widget_set_valign(frames[7],GTK_ALIGN_FILL);
        gtk_grid_attach(GTK_GRID(g),frames[6],0,0,2,1);gtk_grid_attach(GTK_GRID(g),frames[7],2,0,1,1);gtk_box_pack_start(GTK_BOX(l),g,TRUE,TRUE,0);
    }else{pack(l,frames[6],TRUE);l=line();pack(l,frames[7],TRUE);}
    for(int i=0;i<8;i++)g_object_unref(frames[i]);
    gtk_widget_show_all(dashboard);
}
static gboolean configure(GtkWidget *w,GdkEventConfigure *e,gpointer p){(void)w;(void)p;reflow(e->width);return FALSE;}
static void maximize(void){if(gtk_window_is_maximized(GTK_WINDOW(win)))gtk_window_unmaximize(GTK_WINDOW(win));else gtk_window_maximize(GTK_WINDOW(win));}
static gboolean drag(GtkWidget *w,GdkEventButton *e,gpointer p){(void)w;(void)p;if(e->button!=1)return FALSE;if(e->type==GDK_2BUTTON_PRESS)maximize();else if(e->type==GDK_BUTTON_PRESS)gtk_window_begin_move_drag(GTK_WINDOW(win),1,e->x_root,e->y_root,e->time);return TRUE;}
static void action(GtkButton *b,gpointer p){(void)b;switch(GPOINTER_TO_INT(p)){case 0:gtk_window_iconify(GTK_WINDOW(win));break;case 1:maximize();break;default:gtk_window_close(GTK_WINDOW(win));}}
static void save(void){GKeyFile *k=g_key_file_new();g_key_file_set_string(k,"UI","theme",wozt_ui_standard_theme_id(ui));g_key_file_set_string(k,"UI","language",wozt_ui_standard_language_id(ui));gchar *dir=g_path_get_dirname(config_path);g_mkdir_with_parents(dir,0700);g_free(dir);g_key_file_save_to_file(k,config_path,NULL);g_key_file_unref(k);}
static void theme(GtkComboBox *c,gpointer p){(void)p;if(loading)return;const char *id=gtk_combo_box_get_active_id(c);if(id){wozt_ui_standard_apply_theme(ui,id);if(!self_test)save();}}
static void language(GtkComboBox *c,gpointer p){(void)p;if(loading)return;const char *id=gtk_combo_box_get_active_id(c);if(id){loading=TRUE;wozt_ui_standard_apply_language(ui,id,win);loading=FALSE;int w,h;gtk_window_get_size(GTK_WINDOW(win),&w,&h);reflow(w);if(!self_test)save();}}
static void reload(GtkButton *b,gpointer p){(void)b;(void)p;loading=TRUE;wozt_ui_standard_reload(ui,win);loading=FALSE;}
static void toggle(GtkToggleButton *b,gpointer p){(void)p;wozt_ui_label_set_text(GTK_LABEL(status),gtk_toggle_button_get_active(b)?"on":"Off");}
static gboolean window_state(GtkWidget *w,GdkEventWindowState *e,gpointer p){(void)w;(void)p;for(int i=0;i<8;i++)if(zones[i])gtk_widget_set_visible(zones[i],!(e->new_window_state&GDK_WINDOW_STATE_MAXIMIZED));return FALSE;}
static gboolean smoke(gpointer p){(void)p;int sum=0;for(int i=0;i<3;i++)sum+=wozt_dashboard_row_extra(137,i,i+1,3);if(sum!=137||signature<0)exit_status=1;reflow(600);reflow(1400);reflow(880);const char *langs[]={"fr","de","it","es","pt","ru","ja","ko","zh-cn","en"};for(guint i=0;i<G_N_ELEMENTS(langs);i++)wozt_ui_standard_apply_language(ui,langs[i],win);gtk_main_quit();return G_SOURCE_REMOVE;}
int main(int argc,char **argv){
    self_test=argc>1&&!strcmp(argv[1],"--self-test");if(self_test)argc=1;gtk_init(&argc,&argv);
    config_path=g_build_filename(g_get_user_config_dir(),"wozt-gtk-skeleton","ui.ini",NULL);GKeyFile *k=g_key_file_new();if(!self_test)g_key_file_load_from_file(k,config_path,G_KEY_FILE_NONE,NULL);
    gchar *t=g_key_file_get_string(k,"UI","theme",NULL),*lang=g_key_file_get_string(k,"UI","language",NULL);g_key_file_unref(k);
    const char *data=g_getenv("WOZT_DATA_DIR");ui=wozt_ui_standard_new(data?data:"data",t,lang);g_free(t);g_free(lang);
    win=gtk_window_new(GTK_WINDOW_TOPLEVEL);gtk_window_set_title(GTK_WINDOW(win),"WOZT GTK Skeleton");gtk_window_set_decorated(GTK_WINDOW(win),FALSE);gtk_window_set_resizable(GTK_WINDOW(win),TRUE);gtk_window_set_default_size(GTK_WINDOW(win),880,806);
    GdkGeometry geom={0};geom.min_width=600;geom.min_height=420;gtk_window_set_geometry_hints(GTK_WINDOW(win),win,&geom,GDK_HINT_MIN_SIZE);
    gtk_style_context_add_class(gtk_widget_get_style_context(win),"wozt-window");gtk_window_set_icon_name(GTK_WINDOW(win),"applications-system");
    GtkWidget *overlay=gtk_overlay_new(),*root=gtk_box_new(GTK_ORIENTATION_VERTICAL,0),*header=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
    gtk_container_add(GTK_CONTAINER(win),overlay);gtk_container_add(GTK_CONTAINER(overlay),root);gtk_widget_set_margin_start(header,7);gtk_widget_set_margin_end(header,7);gtk_widget_set_margin_top(header,6);
    gtk_style_context_add_class(gtk_widget_get_style_context(header),"wozt-header");gtk_box_pack_start(GTK_BOX(root),header,FALSE,FALSE,0);
    GtkWidget *dragbox=gtk_event_box_new();gtk_widget_add_events(dragbox,GDK_BUTTON_PRESS_MASK);GtkWidget *brand=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4);gtk_box_pack_start(GTK_BOX(brand),gtk_image_new_from_icon_name("applications-system",GTK_ICON_SIZE_MENU),FALSE,FALSE,0);gtk_box_pack_start(GTK_BOX(brand),gtk_label_new("WOZT GTK Skeleton"),FALSE,FALSE,0);gtk_container_add(GTK_CONTAINER(dragbox),brand);gtk_box_pack_start(GTK_BOX(header),dragbox,TRUE,TRUE,0);g_signal_connect(dragbox,"button-press-event",G_CALLBACK(drag),NULL);
    GtkWidget *tc=wozt_ui_standard_theme_combo(ui),*lc=wozt_ui_standard_language_combo(ui);gtk_box_pack_start(GTK_BOX(header),tc,FALSE,FALSE,0);gtk_box_pack_start(GTK_BOX(header),lc,FALSE,FALSE,0);g_signal_connect(tc,"changed",G_CALLBACK(theme),NULL);g_signal_connect(lc,"changed",G_CALLBACK(language),NULL);
    GtkWidget *rb=gtk_button_new_with_label("↻");gtk_widget_set_tooltip_text(rb,"Reload themes and languages");gtk_box_pack_start(GTK_BOX(header),rb,FALSE,FALSE,0);g_signal_connect(rb,"clicked",G_CALLBACK(reload),NULL);
    const char *glyphs[]={"−","□","×"};const char *tips[]={"Minimize","Maximize / restore","Close"};for(int i=0;i<3;i++){GtkWidget *b=gtk_button_new_with_label(glyphs[i]);gtk_widget_set_tooltip_text(b,tips[i]);gtk_style_context_add_class(gtk_widget_get_style_context(b),"wozt-window-button");gtk_box_pack_start(GTK_BOX(header),b,FALSE,FALSE,0);g_signal_connect(b,"clicked",G_CALLBACK(action),GINT_TO_POINTER(i));}
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);gtk_widget_set_size_request(scroll,1,1);gtk_box_pack_start(GTK_BOX(root),scroll,TRUE,TRUE,0);
    dashboard=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);gtk_widget_set_hexpand(dashboard,TRUE);gtk_widget_set_margin_start(dashboard,6);gtk_widget_set_margin_end(dashboard,6);gtk_widget_set_margin_top(dashboard,5);gtk_widget_set_margin_bottom(dashboard,5);gtk_container_add(GTK_CONTAINER(scroll),dashboard);
    g_signal_connect_after(dashboard,"size-allocate",G_CALLBACK(wozt_dashboard_allocate_height),NULL);
    GtkWidget *g[8];const char *names[]={"Overview","Clients","Controller output","Local controller input","Capture","Monitor","Reset method","Maintenance"};const char *icons[]={"view-dashboard-symbolic","computer-symbolic","input-gaming-symbolic","input-gaming-symbolic","camera-video-symbolic","video-display-symbolic","system-reboot-symbolic","emblem-system-symbolic"};
    for(int i=0;i<8;i++){frames[i]=frame(names[i],icons[i],&g[i]);gtk_box_pack_start(GTK_BOX(dashboard),frames[i],FALSE,FALSE,0);}
    status=gtk_label_new("Off");GtkWidget *check=gtk_check_button_new_with_label("on");add(g[0],0,"Status",check);g_signal_connect(check,"toggled",G_CALLBACK(toggle),NULL);
    GtkWidget *nb=gtk_notebook_new();for(int i=0;i<2;i++){GtkWidget *p=gtk_grid_new();GtkWidget *sw=gtk_scrolled_window_new(NULL,NULL);gtk_container_add(GTK_CONTAINER(sw),p);gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);gtk_widget_set_size_request(sw,200,100);add(p,0,"bitrate (Mbps)",scale());gtk_notebook_append_page(GTK_NOTEBOOK(nb),sw,gtk_label_new(i?"Android":"Switch"));}gtk_grid_attach(GTK_GRID(g[1]),nb,0,0,2,1);
    for(int j=0;j<8;j++)add_at(g[2],2*(j/4),j%4,j%2?"Deadzone:":"Input %:",scale());
    for(int j=0;j<4;j++)add(g[3],j,j%2?"Deadzone:":"Input %:",scale());
    add(g[4],0,"Quality",combo("720p60","480p60",100));add(g[5],0,"Audio",scale());
    GtkWidget *flow=gtk_flow_box_new();gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow),GTK_SELECTION_NONE);gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow),1);gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow),2);gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow),8);
    GtkWidget *left=gtk_grid_new(),*right=gtk_grid_new();gtk_grid_set_row_spacing(GTK_GRID(left),1);gtk_grid_set_row_spacing(GTK_GRID(right),1);gtk_widget_set_valign(left,GTK_ALIGN_START);gtk_widget_set_valign(right,GTK_ALIGN_START);
    add(left,0,"Wake method",combo("Bash script","Bluetooth beacon",100));add(left,1,"target console",combo("Nintendo Switch 2","Nintendo Switch Pro Controller",100));
    add(right,0,"Bluetooth adapter",combo("none detected","none",200));add(right,1,"Status",gtk_label_new("Off"));gtk_container_add(GTK_CONTAINER(flow),left);gtk_container_add(GTK_CONTAINER(flow),right);gtk_grid_attach(GTK_GRID(g[6]),flow,0,0,2,1);
    add(g[7],0,"Status",gtk_label_new("Off"));
    GtkWidget *footer=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);gtk_widget_set_margin_start(footer,7);gtk_widget_set_margin_end(footer,7);gtk_widget_set_margin_bottom(footer,6);gtk_box_pack_start(GTK_BOX(footer),status,TRUE,TRUE,0);gtk_box_pack_end(GTK_BOX(footer),gtk_label_new("v1.3"),FALSE,FALSE,0);gtk_box_pack_end(GTK_BOX(root),footer,FALSE,FALSE,0);
    wozt_add_resize_zones(overlay,win);GList *children=gtk_container_get_children(GTK_CONTAINER(overlay));int z=0;for(GList *i=children;i;i=i->next)if(i->data!=root&&z<8){zones[z++]=i->data;gtk_widget_set_no_show_all(i->data,TRUE);gtk_widget_show(i->data);}g_list_free(children);
    g_signal_connect(win,"window-state-event",G_CALLBACK(window_state),NULL);g_signal_connect(win,"configure-event",G_CALLBACK(configure),NULL);g_signal_connect(win,"destroy",G_CALLBACK(gtk_main_quit),NULL);
    gtk_widget_show_all(win);reflow(880);loading=TRUE;wozt_ui_standard_apply_language(ui,wozt_ui_standard_language_id(ui),win);loading=FALSE;
    if(self_test)g_timeout_add(300,smoke,NULL);
    gtk_main();wozt_ui_standard_free(ui);g_free(config_path);return exit_status;
}
