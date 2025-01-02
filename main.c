#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 10
#define COLS 10
#define MINES 10

typedef struct {
    GtkWidget *button;
    int is_mine;
    int revealed;
    int flagged;
    int adjacent_mines;
} Cell;

Cell grid[ROWS][COLS];
time_t start_time;
GtkWidget *timer_label;

void reset_game(GtkWidget *widget, gpointer data);

void reveal_cell(int row, int col);

void check_victory(GtkWidget *parent);

void end_game(GtkWidget *parent, const char *message) {
    time_t end_time = time(NULL);
    int elapsed_time = (int)difftime(end_time, start_time);

    char full_message[256];
    snprintf(full_message, sizeof(full_message), "%s\nTime taken: %d seconds", message, elapsed_time);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", full_message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    reset_game(NULL, parent);
}

int count_adjacent_mines(int row, int col) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int r = row + i;
            int c = col + j;
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS && grid[r][c].is_mine) {
                count++;
            }
        }
    }
    return count;
}

void update_timer_label() {
    time_t current_time = time(NULL);
    int elapsed = (int)difftime(current_time, start_time);

    char time_text[20];
    snprintf(time_text, sizeof(time_text), "Time: %d sec", elapsed);
    gtk_label_set_text(GTK_LABEL(timer_label), time_text);
}

gboolean timer_callback(gpointer data) {
    update_timer_label();
    return TRUE; // Continue calling this function
}

void reveal_cell(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS || grid[row][col].revealed || grid[row][col].flagged) {
        return;
    }

    grid[row][col].revealed = 1;
    gtk_widget_set_sensitive(grid[row][col].button, FALSE);
    gtk_widget_override_background_color(grid[row][col].button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1.0, 1.0, 1.0, 1.0}); // Set background to white

    if (grid[row][col].is_mine) {
        gtk_button_set_label(GTK_BUTTON(grid[row][col].button), "M");
        end_game(gtk_widget_get_toplevel(grid[row][col].button), "You hit a mine! Game Over!");
        return;
    }

    int adjacent = count_adjacent_mines(row, col);
    grid[row][col].adjacent_mines = adjacent;

    if (adjacent > 0) {
        char label[2];
        snprintf(label, sizeof(label), "%d", adjacent);
        gtk_button_set_label(GTK_BUTTON(grid[row][col].button), label);
    } else {
        gtk_button_set_label(GTK_BUTTON(grid[row][col].button), " ");
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                reveal_cell(row + i, col + j);
            }
        }
    }

    check_victory(gtk_widget_get_toplevel(grid[row][col].button));
}

void toggle_flag(GtkWidget *widget, gpointer data) {
    int *coords = (int *)data;
    int row = coords[0];
    int col = coords[1];

    if (grid[row][col].revealed) {
        return;
    }

    grid[row][col].flagged = !grid[row][col].flagged;

    if (grid[row][col].flagged) {
        gtk_button_set_label(GTK_BUTTON(grid[row][col].button), "F");
    } else {
        gtk_button_set_label(GTK_BUTTON(grid[row][col].button), "");
    }

    check_victory(gtk_widget_get_toplevel(grid[row][col].button));
}

void check_victory(GtkWidget *parent) {
    int correctly_flagged = 0;
    int total_flags = 0;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (grid[i][j].flagged) {
                total_flags++;
                if (grid[i][j].is_mine) {
                    correctly_flagged++;
                }
            }
        }
    }

    if (correctly_flagged == MINES && total_flags == MINES) {
        end_game(parent, "Congratulations! You flagged all the mines correctly and won!");
    }
}

void cell_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    int *coords = (int *)data;
    int row = coords[0];
    int col = coords[1];

    if (event->button == 1) { // Left click
        reveal_cell(row, col);
    } else if (event->button == 3) { // Right click
        toggle_flag(widget, data);
    }
}

void reset_game(GtkWidget *widget, gpointer data) {
    GtkWidget *parent = (GtkWidget *)data;
    srand(time(NULL));
    start_time = time(NULL);
    update_timer_label();

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j].is_mine = 0;
            grid[i][j].revealed = 0;
            grid[i][j].flagged = 0;
            grid[i][j].adjacent_mines = 0;
            gtk_button_set_label(GTK_BUTTON(grid[i][j].button), "");
            gtk_widget_set_sensitive(grid[i][j].button, TRUE);
            gtk_widget_override_background_color(grid[i][j].button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0.0, 0.0, 0.0, 1.0}); // Set background to black
        }
    }

    int placed = 0;
    while (placed < MINES) {
        int row = rand() % ROWS;
        int col = rand() % COLS;
        if (!grid[row][col].is_mine) {
            grid[row][col].is_mine = 1;
            placed++;
        }
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minesweeper");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    timer_label = gtk_label_new("Time: 0 sec");
    gtk_box_pack_start(GTK_BOX(vbox), timer_label, FALSE, FALSE, 0);

    GtkWidget *grid_layout = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid_layout, TRUE, TRUE, 0);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j].button = gtk_button_new_with_label("");
            int *coords = malloc(2 * sizeof(int));
            coords[0] = i;
            coords[1] = j;
            g_signal_connect(grid[i][j].button, "button-press-event", G_CALLBACK(cell_clicked), coords);
            gtk_grid_attach(GTK_GRID(grid_layout), grid[i][j].button, j, i, 1, 1);
        }
    }

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_game), window);
    gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    reset_game(NULL, window);

    g_timeout_add(1000, timer_callback, NULL); // Update timer every second

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
