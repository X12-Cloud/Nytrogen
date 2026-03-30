extern int gtk_init(int* argc, int* argv);
extern int* gtk_window_new(int type);
extern int gtk_window_set_title(int* window, string title);
extern int gtk_widget_show_all(int* window);
extern int gtk_main();
extern int* gtk_label_new(string text);
extern int gtk_container_add(int* container, int* widget);
extern int* gtk_button_new_with_label(string label);

int main() {
    int argc = 0;
    int argv = 0;

    gtk_init(&argc, &argv);

    int* win = gtk_window_new(0);

    int* lbl = gtk_label_new("Nytrogen Compiler v0.1");
    // int* btn = gtk_button_new_with_label("Click Me");

    // gtk_container_add(win, btn);
    gtk_container_add(win, lbl);

    gtk_window_set_title(win, "Nytrogen GTK");
    gtk_widget_show_all(win);

    gtk_main();

    return 0;
}
