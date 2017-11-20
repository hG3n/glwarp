//
// Created by ephtron on 20.11.17.
//

#include <stdlib.h>
#include <stdio.h>

#include <GL/glu.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>

int main(int argc, char **argv) {
    Display *display;
    display = XOpenDisplay(":1.0");

    if (display == NULL) {
        printf("Cannot connect to X server ':1.0'\n");
        exit(1);
    }

    Window target_win;
    target_win = 0x400016; // In example...


    XWindowAttributes win_info;

    int absx, absy, x, y;
    XImage *image;
    unsigned width, height;
    Window dummywin;

    XGetWindowAttributes(display, target_win, &win_info);
    XTranslateCoordinates(display, target_win, RootWindow(display, 0), 0, 0, &absx, &absy, &dummywin);

    win_info.x = absx;
    win_info.y = absy;
    width = win_info.width;
    height = win_info.height;
    x = absx - win_info.x;
    y = absy - win_info.y;

    XMapRaised(display, target_win);

    // flushes the output buffer and then waits until all requests have been received and processed by the X server
    XSync(display, 0);

//    XSetInputFocus(display, target_win, RevertToPointerRoot, CurrentTime);

    image = XGetImage(display, target_win, x, y, width, height, AllPlanes, ZPixmap);
    if (!image) printf("Unable to create image...\n");

    printf("%d %d", image->width, image->height);


    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
                      image->width, image->height,
                      GL_RGB, GL_UNSIGNED_BYTE,
                      image->data); // Exit with segmentation fault...

    XDestroyImage(image);
    XCloseDisplay(display);
    exit(0);
}
