// cd "C:\Users\user\Desktop\123"

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
    int adjacent_mines;
} Cell;

Cell grid[ROWS][COLS];

void reset_game(GtkWidget *widget, gpointer data);

void reveal_cell(int row, int col);

void end_game(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
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

void reveal_cell(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS || grid[row][col].revealed) {
        return;
    }

    grid[row][col].revealed = 1;
    gtk_widget_set_sensitive(grid[row][col].button, FALSE);

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

    int unrevealed_cells = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (!grid[i][j].revealed && !grid[i][j].is_mine) {
                unrevealed_cells++;
            }
        }
    }

    if (unrevealed_cells == 0) {
        end_game(gtk_widget_get_toplevel(grid[row][col].button), "Congratulations! You won!");
    }
}

void cell_clicked(GtkWidget *widget, gpointer data) {
    int *coords = (int *)data;
    int row = coords[0];
    int col = coords[1];
    reveal_cell(row, col);
}

void reset_game(GtkWidget *widget, gpointer data) {
    GtkWidget *parent = (GtkWidget *)data;
    srand(time(NULL));

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j].is_mine = 0;
            grid[i][j].revealed = 0;
            grid[i][j].adjacent_mines = 0;
            gtk_button_set_label(GTK_BUTTON(grid[i][j].button), "");
            gtk_widget_set_sensitive(grid[i][j].button, TRUE);
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
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *grid_layout = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid_layout);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            grid[i][j].button = gtk_button_new_with_label("");
            int *coords = malloc(2 * sizeof(int));
            coords[0] = i;
            coords[1] = j;
            g_signal_connect(grid[i][j].button, "clicked", G_CALLBACK(cell_clicked), coords);
            gtk_grid_attach(GTK_GRID(grid_layout), grid[i][j].button, j, i, 1, 1);
        }
    }

    GtkWidget *reset_button = gtk_button_new_with_label("Reset");
    g_signal_connect(reset_button, "clicked", G_CALLBACK(reset_game), window);
    gtk_grid_attach(GTK_GRID(grid_layout), reset_button, 0, ROWS, COLS, 1);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    reset_game(NULL, window);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
