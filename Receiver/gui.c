#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <ncurses.h>
#include <unistd.h>
#include "session.h"
#include "transport_l.h"
#include "neighbor.h"
#include "gui.h"


WINDOW *session_window, *files_window, *wireless_window;

WINDOW *create_newwin(int height, int width, int starty, int startx) {

        WINDOW *local_win;

        local_win = newwin(height, width, starty, startx);
        box(local_win, 0 , 0);          /* 0, 0 gives default characters for the vertical and horizontal lines */
        wrefresh(local_win);            /* Show that box */

        return local_win;
}


void session_print_neighbor(struct PEER *peer_list) {

        struct PEER *local_peer_list = peer_list;

        int x = 1, maxX = 0, maxY = 0;

        getmaxyx(stdscr, maxX, maxY);

        werase(session_window);

        box(session_window, 0, 0);
        while (local_peer_list) {
                
                mvwprintw(session_window, x, 1, "%s %15s:%d \tSESSION ID : %d TOTAL_PACKETS : %d", 
                                local_peer_list->host_name, inet_ntoa(local_peer_list->peer_address.sin_addr),
                                local_peer_list->port, local_peer_list->session_id, local_peer_list->packets);
                local_peer_list = (struct PEER *) local_peer_list->next;
                x += 1;
        }   
        wrefresh(session_window);
}


void gui_print_file (char *filename) {


        static int x = 1;

        int  maxX = 0, maxY = 0;

        getmaxyx(stdscr, maxX, maxY);

        mvwprintw(files_window, x, 1, "%s", filename);

        x += 1;

        wrefresh(files_window);


}
void draw_windows() {

        int maxX = 0, maxY = 0;
        int height = 0, width = 0, startx = 0, starty  = 0;
        WINDOW *heading_window;

        getmaxyx(stdscr, maxX, maxY);

        startx = 0;
        starty = 0;
        height = 3;
        width = maxY/2;
        heading_window = create_newwin(height, width, startx, starty);

        box(heading_window, 0, 0);
        wattron(heading_window, A_REVERSE);
        mvwprintw(heading_window, 1, maxY/4 - 4, "%s", "SESSIONS");
        wattroff(heading_window, A_REVERSE);
        wrefresh(heading_window);

        startx = 3;
        starty = 0;
        height = maxX - 3;
        width = maxY/2;
        session_window = create_newwin(height, width, startx, starty);

        startx = 0;
        starty = maxY/2;
        height = 3;
        width = maxY/2;
        heading_window = create_newwin(height, width, startx, starty); 

        box(heading_window, 0, 0); 
        wattron(heading_window, A_REVERSE);
        mvwprintw(heading_window, 1, maxY/4 - 10, "%s", "RECEIVED FILES");
        wattroff(heading_window, A_REVERSE);
        wrefresh(heading_window);

        startx = 3;
        starty = maxY/2;
        height = maxX - 3;
        width = maxY/2;
        files_window = create_newwin(height, width, startx, starty); 
}

/* 1. Start curses mode 
   2. Line buffering Disabled
   3. Enable Function Keys
   */


void gui(void) {
   
        initscr();            
        cbreak();             
        keypad(stdscr, TRUE); 
        start_color();

        init_pair(1, COLOR_CYAN, COLOR_BLACK);

        draw_windows();
}
