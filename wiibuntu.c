#include <gtk/gtk.h>

void on_link_clicked(GtkWidget *widget, gpointer data) {
    const char *url = (const char *)data;
    gtk_show_uri(NULL, url, GDK_CURRENT_TIME, NULL);
}

void on_deb_download(GtkWidget *widget, gpointer data) {
    const char *deb_url = (const char *)data;
    char command[512];
    snprintf(command, sizeof(command), "wget %s -O /tmp/package.deb && sudo dpkg -i /tmp/package.deb", deb_url);
    system(command);
}

void create_page(GtkWidget *notebook, const char *text, const char *image_path, const char *page_name) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label = gtk_label_new(text);
    GtkWidget *image = gtk_image_new_from_file(image_path);

    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 5);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, gtk_label_new(page_name));
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Wiibuntu Splash");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    create_page(notebook, "Welcome to Wiibuntu", "146908244.png", "Welcome");
    create_page(notebook, "Feature Overview", "what-is.jpg", "Overview");
    create_page(notebook, "More Distros", "221247.png", "Distros");

    GtkWidget *final_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *final_label = gtk_label_new("Thank you for installing Wiibuntu. Choose an option below:");
    GtkWidget *final_image = gtk_image_new_from_file("placeholder4.png");
    GtkWidget *web_button = gtk_button_new_with_label("Visit Website");
    GtkWidget *deb_button = gtk_button_new_with_label("Download .deb Installer");

    g_signal_connect(web_button, "clicked", G_CALLBACK(on_link_clicked), "https://example.com");
    g_signal_connect(deb_button, "clicked", G_CALLBACK(on_deb_download), "https://example.com/download/package.deb");

    gtk_box_pack_start(GTK_BOX(final_page), final_image, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(final_page), final_label, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(final_page), web_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(final_page), deb_button, TRUE, TRUE, 5);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), final_page, gtk_label_new("Final Page"));

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
