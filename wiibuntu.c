#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <jpeglib.h> // Include libjpeg for JPEG support
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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

// Function to load and display a JPEG image
void display_jpeg(Display *display, Window window, GC gc, const char *filename, int x, int y) {
    FILE *jpeg_file = fopen(filename, "rb");
    if (!jpeg_file) {
        fprintf(stderr, "Failed to open JPEG file: %s\n", filename);
        return;
    }

    // Decompress JPEG
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, jpeg_file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int pixel_size = cinfo.output_components;

    unsigned char *image_data = malloc(width * height * pixel_size);
    unsigned char *row_pointer = image_data;

    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
        row_pointer += width * pixel_size;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(jpeg_file);

    // Convert to XImage
    XImage *ximage = XCreateImage(display, DefaultVisual(display, 0), DefaultDepth(display, 0),
                                  ZPixmap, 0, (char *)image_data, width, height, 32, 0);

    // Display the image
    XPutImage(display, window, gc, ximage, 0, 0, x, y, width, height);

    // Free resources
    ximage->data = NULL; // Prevent XDestroyImage from freeing our data
    XDestroyImage(ximage);
    free(image_data);
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
            // Draw the JPEG logo
            display_jpeg(display, window, gc, "146908244.jpg", window_width / 2 - 100, 20);

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

