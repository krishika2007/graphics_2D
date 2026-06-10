#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ROWS 20
#define COLS 60
#define MAX_SHAPES 100

// Define shape types
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Shape data structures
typedef struct {
    int x1, y1;
    int x2, y2;
} LineData;

typedef struct {
    int x, y;
    int width, height;
} RectData;

typedef struct {
    int cx, cy;
    int radius;
} CircleData;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleData;

typedef struct {
    int id;
    ShapeType type;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriangleData triangle;
    } data;
} Shape;

// Global state
Shape shapes[MAX_SHAPES];
int num_shapes = 0;
int next_shape_id = 1;
char canvas[ROWS][COLS];

// Function declarations
void clear_canvas();
void draw_pixel(int x, int y);
void draw_line(int x1, int y1, int x2, int y2);
void draw_rectangle(int x, int y, int w, int h);
void draw_circle(int cx, int cy, int r);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
void render_all_shapes();
void display_canvas();

int read_int(const char* prompt, int min_val, int max_val);
void add_shape_menu();
void delete_shape_menu();
void modify_shape_menu();
void print_shape_details(const Shape* s);

int main() {
    printf("===========================================\n");
    printf("     Welcome to the 2D Graphics Editor     \n");
    printf("===========================================\n");

    while (1) {
        printf("\n--- Main Menu ---\n");
        printf("1. Display Canvas\n");
        printf("2. Add Shape\n");
        printf("3. Delete Shape\n");
        printf("4. Modify Shape\n");
        printf("5. List Active Shapes\n");
        printf("6. Exit\n");

        int choice = read_int("Enter your choice (1-6): ", 1, 6);

        switch (choice) {
            case 1:
                render_all_shapes();
                display_canvas();
                break;
            case 2:
                add_shape_menu();
                break;
            case 3:
                delete_shape_menu();
                break;
            case 4:
                modify_shape_menu();
                break;
            case 5:
                if (num_shapes == 0) {
                    printf("No active shapes on the canvas.\n");
                } else {
                    printf("\nActive Shapes:\n");
                    for (int i = 0; i < num_shapes; i++) {
                        print_shape_details(&shapes[i]);
                    }
                }
                break;
            case 6:
                printf("Exiting 2D Graphics Editor. Goodbye!\n");
                return 0;
        }
    }
}

// Helper to read integers robustly
int read_int(const char* prompt, int min_val, int max_val) {
    int val;
    char buffer[256];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Trim leading/trailing whitespace
        char* start = buffer;
        while (*start == ' ' || *start == '\t') {
            start++;
        }
        if (*start == '\0') {
            printf("Empty input. Please enter a valid number.\n");
            continue;
        }
        
        char* endptr;
        long parsed = strtol(start, &endptr, 10);
        if (endptr == start || (*endptr != '\0' && *endptr != '\r')) {
            printf("Invalid integer format. Please try again.\n");
            continue;
        }
        
        if (parsed < min_val || parsed > max_val) {
            printf("Value %ld is out of range [%d, %d]. Please try again.\n", parsed, min_val, max_val);
            continue;
        }
        
        val = (int)parsed;
        break;
    }
    return val;
}

// Canvas and rendering functions
void clear_canvas() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            canvas[r][c] = '_';
        }
    }
}

void draw_pixel(int x, int y) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        canvas[y][x] = '*';
    }
}

// Bresenham's Line Algorithm
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        draw_pixel(x1, y1);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw rectangle borders
void draw_rectangle(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    draw_line(x, y, x + w - 1, y);                   // Top border
    draw_line(x, y + h - 1, x + w - 1, y + h - 1);   // Bottom border
    draw_line(x, y, x, y + h - 1);                   // Left border
    draw_line(x + w - 1, y, x + w - 1, y + h - 1);   // Right border
}

// Midpoint Circle Algorithm
void draw_circle(int cx, int cy, int r) {
    if (r <= 0) return;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        // Draw the symmetric points in all 8 octants
        draw_pixel(cx + x, cy + y);
        draw_pixel(cx - x, cy + y);
        draw_pixel(cx + x, cy - y);
        draw_pixel(cx - x, cy - y);
        draw_pixel(cx + y, cy + x);
        draw_pixel(cx - y, cy + x);
        draw_pixel(cx + y, cy - x);
        draw_pixel(cx - y, cy - x);

        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        x++;
    }
}

// Draw triangle connecting three points
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// Render canvas with all active shapes
void render_all_shapes() {
    clear_canvas();
    for (int i = 0; i < num_shapes; i++) {
        Shape* s = &shapes[i];
        switch (s->type) {
            case SHAPE_LINE:
                draw_line(s->data.line.x1, s->data.line.y1, s->data.line.x2, s->data.line.y2);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(s->data.rect.x, s->data.rect.y, s->data.rect.width, s->data.rect.height);
                break;
            case SHAPE_CIRCLE:
                draw_circle(s->data.circle.cx, s->data.circle.cy, s->data.circle.radius);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(s->data.triangle.x1, s->data.triangle.y1,
                              s->data.triangle.x2, s->data.triangle.y2,
                              s->data.triangle.x3, s->data.triangle.y3);
                break;
        }
    }
}

// Displays canvas with rulers
void display_canvas() {
    printf("\n");
    // Print decades ruler
    printf("    ");
    for (int c = 0; c < COLS; c++) {
        if (c % 10 == 0) {
            printf("%d", c / 10);
        } else {
            printf(" ");
        }
    }
    printf("\n");

    // Print units ruler
    printf("    ");
    for (int c = 0; c < COLS; c++) {
        printf("%d", c % 10);
    }
    printf("\n");

    // Print top border
    printf("  +");
    for (int c = 0; c < COLS; c++) {
        printf("-");
    }
    printf("+\n");

    // Print canvas rows with row indices
    for (int r = 0; r < ROWS; r++) {
        printf("%2d|", r);
        for (int c = 0; c < COLS; c++) {
            printf("%c", canvas[r][c]);
        }
        printf("|\n");
    }

    // Print bottom border
    printf("  +");
    for (int c = 0; c < COLS; c++) {
        printf("-");
    }
    printf("+\n");
}

void print_shape_details(const Shape* s) {
    switch (s->type) {
        case SHAPE_LINE:
            printf("ID %d: Line from (%d, %d) to (%d, %d)\n",
                   s->id, s->data.line.x1, s->data.line.y1, s->data.line.x2, s->data.line.y2);
            break;
        case SHAPE_RECTANGLE:
            printf("ID %d: Rectangle at (%d, %d), width = %d, height = %d\n",
                   s->id, s->data.rect.x, s->data.rect.y, s->data.rect.width, s->data.rect.height);
            break;
        case SHAPE_CIRCLE:
            printf("ID %d: Circle at (%d, %d), radius = %d\n",
                   s->id, s->data.circle.cx, s->data.circle.cy, s->data.circle.radius);
            break;
        case SHAPE_TRIANGLE:
            printf("ID %d: Triangle with vertices (%d, %d), (%d, %d), (%d, %d)\n",
                   s->id, s->data.triangle.x1, s->data.triangle.y1,
                   s->data.triangle.x2, s->data.triangle.y2,
                   s->data.triangle.x3, s->data.triangle.y3);
            break;
    }
}

// CLI Shape Add Menu
void add_shape_menu() {
    if (num_shapes >= MAX_SHAPES) {
        printf("Error: Maximum shape limit reached (%d). Delete some shapes first.\n", MAX_SHAPES);
        return;
    }

    printf("\n--- Add Shape ---\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. Back to Main Menu\n");

    int type_choice = read_int("Select shape type (1-5): ", 1, 5);
    if (type_choice == 5) return;

    Shape s;
    s.id = next_shape_id++;

    switch (type_choice) {
        case 1:
            s.type = SHAPE_LINE;
            printf("Enter coordinates for the start and end points:\n");
            s.data.line.x1 = read_int("Start X (col, -50 to 150): ", -50, 150);
            s.data.line.y1 = read_int("Start Y (row, -50 to 150): ", -50, 150);
            s.data.line.x2 = read_int("End X (col, -50 to 150): ", -50, 150);
            s.data.line.y2 = read_int("End Y (row, -50 to 150): ", -50, 150);
            break;
        case 2:
            s.type = SHAPE_RECTANGLE;
            printf("Enter parameters for the rectangle:\n");
            s.data.rect.x = read_int("Top-left X (col, -50 to 150): ", -50, 150);
            s.data.rect.y = read_int("Top-left Y (row, -50 to 150): ", -50, 150);
            s.data.rect.width = read_int("Width (1 to 100): ", 1, 100);
            s.data.rect.height = read_int("Height (1 to 100): ", 1, 100);
            break;
        case 3:
            s.type = SHAPE_CIRCLE;
            printf("Enter parameters for the circle:\n");
            s.data.circle.cx = read_int("Center X (col, -50 to 150): ", -50, 150);
            s.data.circle.cy = read_int("Center Y (row, -50 to 150): ", -50, 150);
            s.data.circle.radius = read_int("Radius (1 to 100): ", 1, 100);
            break;
        case 4:
            s.type = SHAPE_TRIANGLE;
            printf("Enter coordinates for the three vertices:\n");
            s.data.triangle.x1 = read_int("Vertex 1 X (col, -50 to 150): ", -50, 150);
            s.data.triangle.y1 = read_int("Vertex 1 Y (row, -50 to 150): ", -50, 150);
            s.data.triangle.x2 = read_int("Vertex 2 X (col, -50 to 150): ", -50, 150);
            s.data.triangle.y2 = read_int("Vertex 2 Y (row, -50 to 150): ", -50, 150);
            s.data.triangle.x3 = read_int("Vertex 3 X (col, -50 to 150): ", -50, 150);
            s.data.triangle.y3 = read_int("Vertex 3 Y (row, -50 to 150): ", -50, 150);
            break;
    }

    shapes[num_shapes++] = s;
    printf("Shape added successfully with ID %d!\n", s.id);
}

// CLI Shape Delete Menu
void delete_shape_menu() {
    if (num_shapes == 0) {
        printf("No active shapes to delete.\n");
        return;
    }

    printf("\nActive Shapes:\n");
    for (int i = 0; i < num_shapes; i++) {
        print_shape_details(&shapes[i]);
    }

    int id_to_delete = read_int("Enter the ID of the shape to delete (or 0 to cancel): ", 0, next_shape_id);
    if (id_to_delete == 0) return;

    int found_idx = -1;
    for (int i = 0; i < num_shapes; i++) {
        if (shapes[i].id == id_to_delete) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Error: Shape with ID %d not found.\n", id_to_delete);
        return;
    }

    // Shift remaining shapes
    for (int i = found_idx; i < num_shapes - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    num_shapes--;
    printf("Shape with ID %d has been successfully deleted.\n", id_to_delete);
}

// CLI Shape Modify Menu
void modify_shape_menu() {
    if (num_shapes == 0) {
        printf("No active shapes to modify.\n");
        return;
    }

    printf("\nActive Shapes:\n");
    for (int i = 0; i < num_shapes; i++) {
        print_shape_details(&shapes[i]);
    }

    int id_to_modify = read_int("Enter the ID of the shape to modify (or 0 to cancel): ", 0, next_shape_id);
    if (id_to_modify == 0) return;

    int found_idx = -1;
    for (int i = 0; i < num_shapes; i++) {
        if (shapes[i].id == id_to_modify) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Error: Shape with ID %d not found.\n", id_to_modify);
        return;
    }

    Shape* s = &shapes[found_idx];
    printf("\nModifying shape ID %d: ", s->id);
    print_shape_details(s);

    switch (s->type) {
        case SHAPE_LINE:
            printf("Enter new coordinates for the start and end points:\n");
            s->data.line.x1 = read_int("Start X (col, -50 to 150): ", -50, 150);
            s->data.line.y1 = read_int("Start Y (row, -50 to 150): ", -50, 150);
            s->data.line.x2 = read_int("End X (col, -50 to 150): ", -50, 150);
            s->data.line.y2 = read_int("End Y (row, -50 to 150): ", -50, 150);
            break;
        case SHAPE_RECTANGLE:
            printf("Enter new parameters for the rectangle:\n");
            s->data.rect.x = read_int("Top-left X (col, -50 to 150): ", -50, 150);
            s->data.rect.y = read_int("Top-left Y (row, -50 to 150): ", -50, 150);
            s->data.rect.width = read_int("Width (1 to 100): ", 1, 100);
            s->data.rect.height = read_int("Height (1 to 100): ", 1, 100);
            break;
        case SHAPE_CIRCLE:
            printf("Enter new parameters for the circle:\n");
            s->data.circle.cx = read_int("Center X (col, -50 to 150): ", -50, 150);
            s->data.circle.cy = read_int("Center Y (row, -50 to 150): ", -50, 150);
            s->data.circle.radius = read_int("Radius (1 to 100): ", 1, 100);
            break;
        case SHAPE_TRIANGLE:
            printf("Enter new coordinates for the three vertices:\n");
            s->data.triangle.x1 = read_int("Vertex 1 X (col, -50 to 150): ", -50, 150);
            s->data.triangle.y1 = read_int("Vertex 1 Y (row, -50 to 150): ", -50, 150);
            s->data.triangle.x2 = read_int("Vertex 2 X (col, -50 to 150): ", -50, 150);
            s->data.triangle.y2 = read_int("Vertex 2 Y (row, -50 to 150): ", -50, 150);
            s->data.triangle.x3 = read_int("Vertex 3 X (col, -50 to 150): ", -50, 150);
            s->data.triangle.y3 = read_int("Vertex 3 Y (row, -50 to 150): ", -50, 150);
            break;
    }

    printf("Shape with ID %d has been successfully modified.\n", id_to_modify);
}
