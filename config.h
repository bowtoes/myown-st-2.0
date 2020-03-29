/* modifier 0 means no modifier */
static  int  surfuseragent = 1;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent = ""; /* Or override the whole user agent string */
static char    *scriptfile = "~/.surf/script.js";
static char      *styledir = "~/.surf/styles/";
static char      *cachedir = "~/.surf/cache/";
static char   *historyfile = "~/.surf/history"; /* History */
static char    *cookiefile = "/dev/null"; // No cookies
static char  *searchengine = "https://duckduckgo.com/?q="; /* Spacesearch */

static SearchEngine searchengines[] =
{
    {"ggl", "https://www.google.com.search?q=%s"},
    {"ddg", "https://www.duckduckgo.com/?q=%s"},
};

#define HOMEPAGE "https://duckduckgo.com/" /* Homepage */

/* Webkit default features */
static Parameter defconfig[ParameterLast] = {
    SETB(AcceleratedCanvas,  1),
    SETB(CaretBrowsing,      0),
    SETV(CookiePolicies,     "@Aa"),
    SETB(DiskCache,          1),
    SETB(DNSPrefetch,        0),
    SETI(FontSize,           14),
    SETB(FrameFlattening,    0),
    SETB(Geolocation,        0),
    SETB(HideBackground,     0),
    SETB(Inspector,          0),
    SETB(JavaScript,         1),
    SETB(KioskMode,          0),
    SETB(LoadImages,         1),
    SETB(MediaManualPlay,    0),
    SETB(Plugins,            1),
    SETV(PreferredLanguages, ((char *[]){NULL })),
    SETB(RunInFullscreen,    0),
    SETB(ScrollBars,         0),
    SETI(ShowIndicators,     1),
    SETB(SiteQuirks,         1),
    SETB(SpellChecking,      0),
    SETV(SpellLanguages,     ((char *[]){"en_US", NULL })),
    SETB(StrictSSL,          0),
    SETB(Style,              1),
    SETF(ZoomLevel,          1.0),
};

static UriParameters uriparams[] = {
    {"(://|\\.)suckless\\.org(/|$)", {
      FSETB(JavaScript, 0),
      FSETB(Plugins,    0),
    }, },
};

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

static char *linkselect_curwin [] =
{
    "/bin/sh", "-c",
    "~/.surf/linkselect.sh $0 'Link' | xargs -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO",
    winid,
    NULL
};
static char *linkselect_newwin [] =
{
    "/bin/sh", "-c",
    "~/.surf/linkselect.sh $0 'Link (new window)' | xargs -r surf",
    winid,
    NULL
};
static char *editscreen[] =
{
    "/bin/sh", "-c",
    "~/.surf/edit_screen.sh",
    NULL
};

/* History */
#define SETURI(p) { \
    .v = (char *[]){ "/bin/sh", "-c", \
        "prop=\"`history_dmenu.sh`\" &&" \
        "xprop -id $1 -f $0 8s -set $0 \"$prop\"", \
        p, winid, NULL \
    } \
}

/* ? */
#define SETPROP(p, q) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "prop=\"`xprop -id $2 $0 " \
             "| sed \"s/^$0(STRING) = \\(\\\\\"\\?\\)\\(.*\\)\\1$/\\2/\" " \
             "| xargs -0 printf %b | dmenu`\" &&" \
             "xprop -id $2 -f $1 8s -set $1 \"$prop\"", \
             p, q, winid, NULL \
        } \
}

/* Bookmarks */
// This is what bookmarks would modify SETPROPS
// #define SETPROP(r, s, p) { \
//         .v = (const char *[]){ "/bin/sh", "-c", \
//              "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
//-             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
//-             "| dmenu -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
//+             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\" && cat ~/.surf/bookmarks)\" " \
//+             "| dmenu -l 10 -p \"$4\" -w $1)\" && " \
//+             "xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
//              "surf-setprop", winid, r, s, p, NULL \
//         } \
// }

/* Bookmarks */
/* BM_ADD(readprop) */
#define BM_ADD(r) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "(echo $(xprop -id $0 $1) | cut -d '\"' -f2 " \
             "&& cat ~/.surf/bookmarks) " \
             "| awk '!seen[$0]++' > ~/.surf/bookmarks.tmp && " \
             "mv ~/.surf/bookmarks.tmp ~/.surf/bookmarks", \
             winid, r, NULL \
        } \
}

/* ? */
#define BM_SET(p) {\
    .v = (char *[]){ "/bin/sh", "-c", \
        "prop=\"`bookmarks_dmenu.sh`\" &&" \
        "xprop -id $1 -f $0 8s -set $0 \"$prop\"", \
        p, winid, NULL \
    } \
}


/* 0.6 Navhist  0.6 Navhist */
#define SELNAV { \
   .v = (char *[]){ "/bin/sh", "-c", \
       "prop=\"`xprop -id $0 _SURF_HIST" \
       " | sed -e 's/^.[^\"]*\"//' -e 's/\"$//' -e 's/\\\\\\n/\\n/g'" \
       " | dmenu -i -l 10`\"" \
       " && xprop -id $0 -f _SURF_NAV 8s -set _SURF_NAV \"$prop\"", \
       winid, NULL \
   } \
}

/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(d, r) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "st -e /bin/sh -c \"curl -# -g -L -J -O" \
             " -A '$1' -e '$2' -b '$3' -c '$3' '$0';" \
             " sleep 3;\"", \
             d, useragent, r, cookiefile, NULL \
        } \
}

/* WGET */
#define DOWNLOAD_WGET(d, r) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "st -e /bin/sh -c" \
             " \"wget --no-glob -U '$1' --referer='$2' --save-cookies '$3' '$0';" \
             " sleep 3;\"", \
             d, useragent, r, cookiefile, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){"/bin/sh", "-c", \
             "mpv --really-quiet \"$0\"", u, NULL \
        } \
}

/* styles */
/*
 * The iteration will stop at the first match, beginning at the beginning of
 * the list.
 */
static SiteStyle styles[] = {
    /* regexp               file in $styledir */
    {".*",                 "default.css"},
};

#define MODKEY GDK_CONTROL_MASK
#define MOD MODKEY
#define CTRL GDK_CONTROL_MASK
#define SHIFT GDK_SHIFT_MASK

/* hotkeys */
/*
 * If you use anything else but CTRL and SHIFT, don't forget to
 * edit the CLEANMASK() macro.
 */
static Key keys[] = {
    /* modifier      keyval          function    arg */
    {CTRL,           GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO")},
    {CTRL,           GDK_KEY_f,      spawn,      SETPROP("_SURF_FIND", "_SURF_FIND")},
    {CTRL,           GDK_KEY_slash,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND")},
/*  In surf.c, I edited updatetitle() to update with the currently loaded uri
 *  _SURF_URI or _SURF_GO often gave the wrong url and I'm not sure why
 *  _NET_WM_NAME is the x tag for the title of a window-manager window
 */

    /* Bookmarks */
    {CTRL|SHIFT,     GDK_KEY_b,      spawn,      BM_ADD("_NET_WM_NAME")}, /* Add URI to bookmarks */
    /* ? */
    {CTRL,           GDK_KEY_b,      spawn,      BM_SET("_SURF_GO")}, /* Goto URI from bookmarks */
    /* History */
    {CTRL,           GDK_KEY_Return, spawn,      SETURI("_SURF_GO")}, /* Goto URI from history */
    {0,              GDK_KEY_Escape, stop,       {0}},
    {CTRL,           GDK_KEY_c,      stop,       {0}},

 // TODO TODO TODO
 // {CTRL,           GDK_KEY_s,      download,   {"_SURF_URI"}}, /* ? */

    /* Reload page & not cache */
    {CTRL|SHIFT,     GDK_KEY_r,      reload,     {.b = 1}},
    /* Reload page & cache (i think) */
    {CTRL,           GDK_KEY_r,      reload,     {.b = 0}},

    {CTRL|SHIFT,     GDK_KEY_l,      navigate,   {.i = +1}}, /* Forward in session history */
    {CTRL|SHIFT,     GDK_KEY_h,      navigate,   {.i = -1}}, /* Backward in session history */
    /* ? */
 // {CTRL|SHIFT,     GDK_KEY_h,      selhist,    SELNAV}, /* 0.6 Navhist */

    /* Currently we have to use scrolling steps that WebKit2GTK+ gives us
     * d: step down, u: step up, r: step right, l:step left
     * D: page down, U: page up */
    {CTRL,           GDK_KEY_h,      scroll,     {.i = 'l'}},
    {CTRL,           GDK_KEY_j,      scroll,     {.i = 'd'}},
    {CTRL,           GDK_KEY_k,      scroll,     {.i = 'u'}},
    {CTRL,           GDK_KEY_r,      scroll,     {.i = 'r'}},
    {CTRL|SHIFT,     GDK_KEY_h,      scroll,     {.i = 'L'}},
    {CTRL|SHIFT,     GDK_KEY_j,      scroll,     {.i = 'D'}},
    {CTRL|SHIFT,     GDK_KEY_k,      scroll,     {.i = 'U'}},
    {CTRL|SHIFT,     GDK_KEY_l,      scroll,     {.i = 'R'}},

    {CTRL,           GDK_KEY_0,      zoom,       {.f =  0.0}},
    {CTRL,           GDK_KEY_minus,  zoom,       {.f = -0.1}},
    {CTRL,           GDK_KEY_equal,  zoom,       {.f = +0.1}},

    {CTRL,           GDK_KEY_p,      clipboard,  {.b =  1}}, /* Load clipboard URI */
    {CTRL,           GDK_KEY_y,      clipboard,  {.b =  0}}, /* Copy URI */
    {CTRL,           GDK_KEY_n,      find,       {.i = +1}},
    {CTRL|SHIFT,     GDK_KEY_n,      find,       {.i = -1}},
    {CTRL|SHIFT,     GDK_KEY_p,      print,      {0}},

    {CTRL|SHIFT,     GDK_KEY_a,      togglecookiepolicy,    {0}},
    {0,              GDK_KEY_F11,    togglefullscreen,      {0}},
    {0,              GDK_KEY_F12,    toggleinspector,       {0}},

    {CTRL|SHIFT,     GDK_KEY_c,      toggle,     {.i = CaretBrowsing}},
    {CTRL|SHIFT,     GDK_KEY_f,      toggle,     {.i = FrameFlattening}},
    {CTRL|SHIFT,     GDK_KEY_g,      toggle,     {.i = Geolocation}},
    {CTRL|SHIFT,     GDK_KEY_s,      toggle,     {.i = JavaScript}},
    {CTRL|SHIFT,     GDK_KEY_i,      toggle,     {.i = LoadImages}},
    {CTRL|SHIFT,     GDK_KEY_v,      toggle,     {.i = Plugins}},
    {CTRL|SHIFT,     GDK_KEY_b,      toggle,     {.i = ScrollBars}},
    {CTRL|SHIFT,     GDK_KEY_m,      toggle,     {.i = Style}},

    {CTRL,           GDK_KEY_d,      externalpipe, {.v = linkselect_curwin}},
    {CTRL|SHIFT,     GDK_KEY_d,      externalpipe, {.v = linkselect_newwin}},
    {CTRL,           GDK_KEY_o,      externalpipe, {.v = editscreen}},

};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask  button  function            argument        stop event */
    {OnLink,        0,          2,      clicknewwindow,     {.b =  0},      1},
    {OnLink,        CTRL,       2,      clicknewwindow,     {.b =  1},      1},
    {OnLink,        CTRL,       1,      clicknewwindow,     {.b =  1},      1},
    {OnAny,         0,          8,      clicknavigate,      {.i = -1},      1},
    {OnAny,         0,          9,      clicknavigate,      {.i = +1},      1},
    {OnMedia,       CTRL,       1,      clickexternplayer,  {0},            1},
};
