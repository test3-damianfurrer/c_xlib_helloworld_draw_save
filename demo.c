#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#define ALLMASKS 		(ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask) //recognized Masks
#define IGNOREMASKS 		(LockMask|Mod2Mask)				     		     //explicit Ignored Masks(Mod2Mask is NumLock)
#define CLEANMASK(mask)         (mask & ~IGNOREMASKS & ALLMASKS)				     //will always and only be a combination of regonized Masks
#define GEOCHANGED		((owx!=wx) || (owy!=wy) || (owidth!=width) || (oheight!=height))

enum { state_Paint,state_NoFocus, state_None };
enum { lfunc_Reload,lfunc_ReloadNext,lfunc_PutImg, lfunc_None };
uint isfullscreen=0;
void setfullscreen(uint *i, Display *dpy, Window root, Window win){
  XEvent ev; Atom atom;
  *i = !(*i);
  ev.type = ClientMessage; ev.xclient.window = win;
  ev.xclient.message_type = XInternAtom(dpy, "_NET_WM_STATE", False);
  ev.xclient.format = 32; ev.xclient.data.l[0] = *i;
  atom = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  ev.xclient.data.l[1] = atom; ev.xclient.data.l[2] = atom;
  XSendEvent(dpy, root, False, ClientMessage, &ev);
}
int main(void) {
   int state = state_None, loop = lfunc_None;
   XEvent e;
   XImage *imgin=NULL,*img;
   int imgx = 10, imgy = 10, wx = 10, wy = 10,owx = 10, owy=10;
   Display *d = XOpenDisplay(NULL);
   if (d == NULL)
      return 1;
   int s = DefaultScreen(d);
   Window wroot = RootWindow(d, s),w = XCreateSimpleWindow(d, wroot, wx, wy, 100, 100, 1,
                           BlackPixel(d, s), WhitePixel(d, s));
   uint width,height,owidth,oheight,rwidth,rheight,border_width_return,depth_return;
   XGetGeometry(d,wroot,&wroot,&wx,&wy,&width,&height,&border_width_return,&depth_return);
   rwidth=width; rheight=height;
   XImage * imginorig = XGetImage(d, wroot, 0, 0, width, height, 4294967295, XYPixmap);
   GC g = DefaultGC(d, s);
   XpmReadFileToImage(d, "cat.xpm", &img, NULL, NULL);

   XSelectInput(d, w, SubstructureRedirectMask | PropertyChangeMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask);
   XMapWindow(d, w);

int xoffset, yoffset, woffset ,hoffset;
   while (!XNextEvent(d, &e)) {
     xoffset = 0; yoffset = 0; woffset = 0; hoffset = 0;
     XGetGeometry(d,w,&wroot,&wx,&wy,&width,&height,&border_width_return,&depth_return);
     if (wx<0)
	xoffset=-1*(wx+border_width_return);
     if (wy<0)
	yoffset=-1*(wy+border_width_return);
     if(rwidth<wx+width+border_width_return)
	woffset=wx+width+border_width_return-rwidth;
     if(rheight<wy+height+border_width_return)
	hoffset=wy+height+border_width_return-rheight;

     if((e.type != Expose || (wx!=owx||wy!=owy)) && GEOCHANGED){
	owx=wx; owy=wy; owidth=width; oheight=height;
        XPutImage(d, w, g, imginorig, wx+border_width_return+xoffset, wy+border_width_return+yoffset, 0+xoffset, 0+yoffset, imginorig->width-wx, imginorig->height-wy);
     }
     if(e.type == Expose && (wx==owx && wy==owy)){
       int hdiff=height-oheight, wdiff=width-owidth;
       owx=wx; owy=wy; owidth=width; oheight=height;
       XPutImage(d, w, g, imginorig, wx+border_width_return+xoffset, wy+border_width_return+yoffset, 0+xoffset, 0+yoffset, imginorig->width-wx, imginorig->height-wy); //seems whole needs updating
     }
     if (e.type == KeyPress){
       KeySym keysym=XKeycodeToKeysym(d, (KeyCode)e.xkey.keycode, 0);
       if ((CLEANMASK(e.xkey.state) == 0) && keysym == XK_q)
         break;
       if ((CLEANMASK(e.xkey.state) == 0) && keysym == XK_f)
	setfullscreen(&isfullscreen,d,wroot,w);
       if ((CLEANMASK(e.xkey.state) == 0) && keysym == XK_c)
        XPutImage(d, w, g, imginorig, wx+border_width_return+xoffset, wy+border_width_return+yoffset, 0+xoffset, 0+yoffset, imginorig->width-wx, imginorig->height-wy);
     }
     if (e.type == ButtonPress){
       state=state_Paint;
       imgx = e.xbutton.x;
       imgy = e.xbutton.y;
     }
     if(e.type == MotionNotify && state == state_Paint){
       imgx = e.xmotion.x;
       imgy = e.xmotion.y;
     }
     if (state == state_Paint){
       XPutImage(d, w, g, img, 0, 0, imgx, imgy, img->width, img->height);
     }
     XFillRectangle(d, w, g, 20, 20, 10, 10);
     XDrawString(d, w, g, 50, 50, "Hello, World!", 13);
     if (e.type == ButtonRelease)
       state=state_None;
   }
   XImage * imgout = XGetImage(d, w, xoffset, yoffset, width-xoffset-woffset, height-yoffset-hoffset, 4294967295, XYPixmap);
   XpmWriteFileFromImage(d,"drawn.xpm",imgout,NULL,NULL);
   XCloseDisplay(d);
   return 0;
}
