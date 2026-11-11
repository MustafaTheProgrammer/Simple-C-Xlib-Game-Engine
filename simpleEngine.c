#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define STATUS_ERROR 0

typedef struct{
    uint8_t *Memory;
    uint64_t Size;
    uint32_t Width;
    uint32_t Height;
    uint32_t Pitch;
} buffer;

void setpixel(buffer *Buffer, int32_t x, int32_t y, uint32_t color){
    uint32_t *pixel = (uint32_t *)((uint32_t *)Buffer->Memory + y * Buffer->Width + x);
    *pixel = 0xffffff00;
}

void rect(buffer *Buffer, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color){
    for (int i = x; i < x + w; i++){
        for (int j = y; j < y + h; j++){
	    setpixel(Buffer, i, j, color);
	}
    }
}

int main(){
    Display *display = XOpenDisplay(0);
    Window rootwindow = XDefaultRootWindow(display);

    int screen = DefaultScreen(display);
    GC context = XDefaultGC(display, screen);

    int screendepth = 24;
    XVisualInfo visualinfo = {};
    if (STATUS_ERROR == XMatchVisualInfo(display, screen, screendepth, TrueColor, &visualinfo))
        printf("Error: no matching visual info\n");

    int windowx = 0;
    int windowy = 0;
    int windoww = 800;
    int windowh = 600;
    int borderw = 0;
    int windowdepth = visualinfo.depth;
    int windowclass = InputOutput;
    Visual* windowvisual = visualinfo.visual;

    int attributevaluemask = CWBackPixel | CWEventMask;
    XSetWindowAttributes windowattributes = {};
    windowattributes.background_pixel = 0xffffccaa;
    windowattributes.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ExposureMask;

    Window mainwindow = XCreateWindow(display, rootwindow, windowx, windowy, windoww, windowh, borderw, windowdepth, windowclass, windowvisual, attributevaluemask, &windowattributes);
    XMapWindow(display, mainwindow);
    XStoreName(display, mainwindow, "something...");

    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    if (!XSetWMProtocols(display, mainwindow, &WM_DELETE_WINDOW, 1))
        printf("Couldn't register WM_DELETE_WINDOW property\n");

    int bitsperpixel = 32;
    int bytesperpixel = bitsperpixel/8;
    int windowbuffersize = windoww * windowh * bytesperpixel;

    buffer Buffer = {};
    Buffer.Width = windoww;
    Buffer.Height = windowh;
    Buffer.Pitch = Buffer.Width * bytesperpixel;
    Buffer.Size = Buffer.Pitch * Buffer.Height;
    Buffer.Memory = (uint8_t *)malloc(Buffer.Size);

    int offset = 0;
    int bytesbetweenscanlines = 0;
    XImage *windowbuffer = XCreateImage(display, visualinfo.visual, visualinfo.depth, ZPixmap, offset, (char *)Buffer.Memory, windoww, windowh, bitsperpixel, bytesbetweenscanlines);

    int x = 0;
    int y = 50;
    int rkey;

    int running = 1;
    while (running){
        while (XPending(display) > 0){
	    XEvent generalevent = {};
	    XNextEvent(display, &generalevent);
	    switch (generalevent.type){
		case KeyPress:
		{
		    XKeyPressedEvent *event = (XKeyPressedEvent *)&generalevent;
	            if (event->keycode == XKeysymToKeycode(display, XK_Right))
			rkey = 1;
		} break;
		case KeyRelease:
		{
		    XKeyPressedEvent *event = (XKeyPressedEvent *)&generalevent;
	            if (event->keycode == XKeysymToKeycode(display, XK_Right))
			rkey = 0;
		} break;
		case ClientMessage:
		{
		    XClientMessageEvent *event = (XClientMessageEvent *)&generalevent;
		    if ((Atom)event->data.l[0] == WM_DELETE_WINDOW){
		        XDestroyWindow(display, mainwindow);
			running = 0;
		    }
		} break;
		    
		case ConfigureNotify:
	        {
		    XConfigureEvent *event = (XConfigureEvent *)&generalevent;
		    windoww = event->width;
		    windowh = event->height;

		    XDestroyImage(windowbuffer);

                    Buffer.Width = windoww;
                    Buffer.Height = windowh;
                    Buffer.Pitch = Buffer.Width * bytesperpixel;
                    Buffer.Size = Buffer.Pitch * Buffer.Height;
                    Buffer.Memory = (uint8_t *)malloc(Buffer.Size);
                
                    windowbuffer = XCreateImage(display, visualinfo.visual, visualinfo.depth, ZPixmap, offset, (char *)Buffer.Memory, windoww, windowh, bitsperpixel, bytesbetweenscanlines);
		 } break;
	    }
	}
	//update
	if (rkey == 1)
	    x++;
	
	//rendering to be done here
	rect(&Buffer, x, y, 50, 50, 0xffffff00);

	int sourcex = 0;
	int sourcey = 0;
	int destx = 0;
	int desty = 0;
	XPutImage(display, mainwindow, context, windowbuffer, sourcex, sourcey, destx, desty, windoww, windowh);
    }
}
