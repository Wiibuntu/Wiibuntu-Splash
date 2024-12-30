#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <png.h> // Include libpng

#define BOX_COUNT 3

// Box structure for interactive elements
typedef struct {
    int x, y, width, height;
    char text[50];
} Box;

void draw_text(Display *display, Window window, GC gc, int x, int y, const char *text) {
    XDrawString(display, window, gc, x, y, text, strlen(text));
}

void draw_box(Display *display, Window window, GC gc, Box box) {
    XDrawRectangle(display, window, gc, box.x, box.y, box.width, box.height);
    draw_text(display, window, gc, box.x + 10, box.y + box.height / 2, box.text);
}

void draw_text_field(Display *display, Window window, GC gc, int x, int y, int width, int height, const char *placeholder) {
    XDrawRectangle(display, window, gc, x, y, width, height);
    draw_text(display, window, gc, x + 10, y + height / 2, placeholder);
}

// Function to load and display a PNG file
void draw_png(Display *display, Window window, GC gc, int x, int y, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Unable to open PNG file: %s\n", filename);
        return;
    }

    // Read PNG header
    unsigned char header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Error: File is not recognized as a PNG file.\n");
        fclose(fp);
        return;
    }

    // Initialize PNG structures
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fprintf(stderr, "Error: png_create_read_struct failed.\n");
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fprintf(stderr, "Error: png_create_info_struct failed.\n");
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error: setjmp failed.\n");
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Update PNG info
    png_read_update_info(png, info);

    // Allocate memory for the image
    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        row_pointers[i] = (png_byte *)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    // Create an XImage and copy the PNG data to it
    XImage *image = XCreateImage(display, DefaultVisual(display, 0), DefaultDepth(display, 0),
                                 ZPixmap, 0, malloc(width * height * 4), width, height, 32, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            png_bytep px = &(row_pointers[i][j * 4]); // Assume RGBA
            unsigned long pixel = (px[0] << 16) | (px[1] << 8) | px[2];
            XPutPixel(image, j, i, pixel);
        }
    }

    // Draw the image on the window
    GC image_gc = XCreateGC(display, window, 0, NULL);
    XPutImage(display, window, image_gc, image, 0, 0, x, y, width, height);
    XFreeGC(display, image_gc);
    XDestroyImage(image);

    // Free memory
    for (int i = 0; i < height; i++) {
        free(row_pointers[i]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
}

int main() {
    Display *display;
    Window window;
    XEvent event;
    int screen;

    // Open connection to X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Window dimensions
    int window_width = screen_width / 2;
    int window_height = screen_height / 2;

    // Create the window
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 screen_width / 4, screen_height / 4,
                                 window_width, window_height, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));

    // Select input events
    XSelectInput(display, window, ExposureMask | ButtonPressMask);

    // Create graphics context
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));

    // Show the window
    XMapWindow(display, window);

    // Define boxes
    Box boxes[BOX_COUNT] = {
        {window_width / 4, window_height / 2, 150, 50, "Option 1"},
        {window_width / 2 - 75, window_height / 2, 150, 50, "Option 2"},
        {3 * window_width / 4 - 150, window_height / 2, 150, 50, "Option 3"}
    };

    // Text field dimensions
    int text_field_width = 200;
    int text_field_height = 30;
    int text_field_x = (window_width - text_field_width) / 2;
    int text_field_y = window_height / 3;
    const char *placeholder = "Enter text here...";

    // Event loop
    while (1) {
        XNextEvent(display, &event);

        if (event.type == Expose) {
            // Draw the PNG logo
            draw_png(display, window, gc, window_width / 2 - 50, 20, "146908244.png");

            // Draw the main text
            draw_text(display, window, gc, window_width / 2 - 75, 100, "Welcome To Wiibuntu");

            // Draw the text field
            draw_text_field(display, window, gc, text_field_x, text_field_y, text_field_width, text_field_height, placeholder);

            // Draw boxes
            for (int i = 0; i < BOX_COUNT; i++) {
                draw_box(display, window, gc, boxes[i]);
            }
        } else if (event.type == ButtonPress) {
            // Check if a box was clicked
            int x = event.xbutton.x;
            int y = event.xbutton.y;
            for (int i = 0; i < BOX_COUNT; i++) {
                if (x >= boxes[i].x && x <= boxes[i].x + boxes[i].width &&
                    y >= boxes[i].y && y <= boxes[i].y + boxes[i].height) {
                    printf("Box %d clicked: %s\n", i + 1, boxes[i].text);
                    if (i == 0) {
                        // Run the shell script when Box 1 is clicked
                        if (fork() == 0) {
                            execlp("x-terminal-emulator", "x-terminal-emulator", "-e", "./script.sh", NULL);
                            perror("execlp failed");
                            exit(1);
                        }
                    } else if (i == 1) {
                        // Open a program when Box 2 is clicked
                        if (fork() == 0) {
                            execlp("gedit", "gedit", NULL);
                            perror("execlp failed");
                            exit(1);
                        }
                    } else if (i == 2) {
                        // Open a terminal and ping Google when Box 3 is clicked
                        if (fork() == 0) {
                            execlp("x-terminal-emulator", "x-terminal-emulator", "-e", "ping -c 4 google.com", NULL);
                            perror("execlp failed");
                            exit(1);
                        }
                    }
                }
            }
        }
    }

    // Clean up
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}

