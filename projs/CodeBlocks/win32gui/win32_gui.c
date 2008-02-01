#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include <glib.h>
#include "config.h"
#include "plugin.h"
#include "gui.h"
#include "win32_gui.h"
#include "point.h"

#define ID_DISPLAY_ZOOMIN 8000
#define ID_DISPLAY_ZOOMOUT 8001

#define ID_FILE_EXIT 9001
#define ID_STUFF_GO 9002

const char g_szClassName[] = "myWindowClass";

#define ID_CHILD_GFX 2000
#define ID_CHILD_1 2001
#define ID_CHILD_2 ID_CHILD_1 + 1
#define ID_CHILD_3 ID_CHILD_2 + 1
#define ID_CHILD_4 ID_CHILD_4 + 1

#define _(text) gettext(text)

gboolean message_pump( gpointer data )
{
	Sleep( 100 );
	printf( "pump\n" );

    MSG messages;            /* Here messages to the application are saved */
    if (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }
    else{
    	exit( 0 );
    }
     return TRUE;
}



struct gui_priv *g_this_;


struct graphics_priv {
	struct point p;
	int width;
	int height;
	int library_init;
	int visible;
	HANDLE wnd_parent_handle;
	HANDLE wnd_handle;

	void (*resize_callback)(void *data, int w, int h);
	void *resize_callback_data;
	void (*motion_callback)(void *data, struct point *p);
	void *motion_callback_data;
	void (*button_callback)(void *data, int press, int button, struct point *p);
	void *button_callback_data;
// 	enum draw_mode_num mode;
};


extern struct graphics_priv *g_gra;

BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
    LPRECT rcParent;
    int idChild;

    idChild = GetWindowLong(hwndChild, GWL_ID);

	if ( idChild == ID_CHILD_GFX )
	{
		rcParent = (LPRECT) lParam;

		MoveWindow( hwndChild,  0, 0, rcParent->right, rcParent->bottom, TRUE );
		(*g_gra->resize_callback)(g_gra->resize_callback_data, rcParent->right, rcParent->bottom);
	}

    return TRUE;
}

#ifndef GET_WHEEL_DELTA_WPARAM
	#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif

static LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    RECT rcClient;

	printf( "PARENT %d %d %d \n", Message, wParam, lParam );

	switch(Message)
	{
		case WM_CREATE:
		{
			HMENU hMenu, hSubMenu;
			HICON hIcon, hIconSm;

			hMenu = CreateMenu();
			g_this_->hwnd = hwnd;

			hSubMenu = CreatePopupMenu();

			gunichar2* utf16 = NULL;

			utf16 = g_utf8_to_utf16( _( "_Quit" ), -1, NULL, NULL, NULL );
			AppendMenuW(hSubMenu, MF_STRING, ID_FILE_EXIT, utf16 );
			g_free( utf16 );

			utf16 = g_utf8_to_utf16( _( "Zoom in" ), -1, NULL, NULL, NULL );
			AppendMenuW(hSubMenu, MF_STRING, ID_DISPLAY_ZOOMIN, utf16 );
			g_free( utf16 );

			utf16 = g_utf8_to_utf16( _( "Zoom out" ), -1, NULL, NULL, NULL );
			AppendMenuW(hSubMenu, MF_STRING, ID_DISPLAY_ZOOMOUT, utf16 );
			g_free( utf16 );

			utf16 = g_utf8_to_utf16( _( "Display" ), -1, NULL, NULL, NULL );
			AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, utf16 );
			g_free( utf16 );

			hSubMenu = CreatePopupMenu();
			AppendMenu(hSubMenu, MF_STRING, ID_STUFF_GO, "&Go");
			AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Stuff");

			SetMenu(hwnd, hMenu);

#if 0
			hIcon = LoadImage(NULL, "menu_two.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
			if(hIcon)
				SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			else
				MessageBox(hwnd, "Could not load large icon!", "Error", MB_OK | MB_ICONERROR);

			hIconSm = LoadImage(NULL, "menu_two.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
			if(hIconSm)
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
			else
				MessageBox(hwnd, "Could not load small icon!", "Error", MB_OK | MB_ICONERROR);

#endif

		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
				case ID_STUFF_GO:
								 	(*g_gra->resize_callback)(g_gra->resize_callback_data, g_gra->width, g_gra->height);

//					navit_draw(g_this_->nav);
					// MessageBox(hwnd, "You clicked Go!", "Woo!", MB_OK);
				break;
			}
		break;
		case WM_USER+ 1:
			printf( "wm_user \n" );
			(*g_gra->resize_callback)( g_gra->resize_callback_data, g_gra->width, g_gra->height );
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_SIZE:
            GetClientRect(hwnd, &rcClient);
			printf( "resize gui to: %d %d \n", rcClient.right, rcClient.bottom );

            EnumChildWindows(hwnd, EnumChildProc, (LPARAM) &rcClient);
            return 0;
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;

		case WM_MOUSEWHEEL:
		{
			struct gui_priv * priv = GetWindowLongPtr( hwnd, DWLP_USER );

			short delta = GET_WHEEL_DELTA_WPARAM( wParam );
			if ( delta > 0 )
			{
				navit_zoom_in(g_this_->nav, 2, NULL);
			}
			else{
				navit_zoom_out(g_this_->nav, 2, NULL);
			}
		}
		break;

		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

HANDLE Win32Init( void )
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = NULL;
	wc.hIcon		 = NULL;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm		 = NULL;

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		_( "Navit" ),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, NULL, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, TRUE);
	UpdateWindow(hwnd);

	g_idle_add (message_pump, NULL);

	return hwnd;
}


static int win32_gui_set_graphics(struct gui_priv *this_, struct graphics *gra)
{
	HANDLE* wndHandle_ptr = graphics_get_data(gra, "wnd_parent_handle_ptr");

	*wndHandle_ptr = this_->hwnd;

	graphics_get_data(gra, "START_CLIENT");
	return 0;
}


static void win32_gui_add_bookmark_do(struct gui_priv *gui)
{
//	navit_add_bookmark(gui->nav, &gui->dialog_coord, gtk_entry_get_text(GTK_ENTRY(gui->dialog_entry)));
//	gtk_widget_destroy(gui->dialog_win);
}

static int win32_gui_add_bookmark(struct gui_priv *gui, struct pcoord *c, char *description)
{
 	return 1;
}


struct gui_methods win32_gui_methods = {
	NULL, // win32_gui_menubar_new,
	NULL, // win32_gui_toolbar_new,
	NULL, // win32_gui_statusbar_new,
	NULL, // win32_gui_popup_new,
	win32_gui_set_graphics,
	NULL,
	NULL, // win32_gui_datawindow_new,
	win32_gui_add_bookmark,
};



static struct gui_priv *win32_gui_new( struct navit *nav, struct gui_methods *meth, struct attr **attrs)
{
	struct gui_priv *this_;
	int w=792, h=547;
	unsigned xid = 0;

	*meth=win32_gui_methods;

	this_=g_new0(struct gui_priv, 1);
	this_->nav=nav;

	g_this_ = this_;

	this_->hwnd = Win32Init();
	SetWindowLongPtr( this_->hwnd , DWLP_USER, this_ );


//	navit_zoom_out(this_->nav, 2, NULL);

	return this_;
}

extern struct graphics_priv* win32_graphics_new( struct graphics_methods *meth, struct attr **attrs);


void plugin_init(void)
{
	plugin_register_gui_type("win32", win32_gui_new);
	plugin_register_graphics_type("win32_graphics", win32_graphics_new);
}
