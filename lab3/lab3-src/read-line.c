/*
 * CS354: Operating Systems.
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#define MAX_BUFFER_LINE 2048

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
int line_index = 0;

//This is cursor position
int pos;


//falg for history, check first time push up or down arrow;
int flag = -1;

// Simple history array
// This history does not change.
// Yours have to be updated.
//int history_index = 0;
/*char * history [] = {
 "0",
 "1",
 "2",
 "3",
 "4",
 "5"
};
*/
int my_history_index = 0;
char * my_history[1024];
int my_history_length = 0;

//int history_length = sizeof(history) / sizeof(char*);

void read_line_print_usage()
{
  char * usage = "\n"
                 " ctrl-?       Print usage\n"
                 " Backspace    Deletes last character\n"
                 " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/*
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  struct termios newTerminal;
  tcgetattr(0, &newTerminal);
  tty_raw_mode();
  pos = 0;
  line_length = 0;
  char ch;
  if(pos > 0){
        ch = 8;
        write(1, &ch, 1);
        pos --;

      }
      if(pos < 0){
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);
        pos++;
      }
  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch >= 32 && ch < 127) {

      int i;
      //modify the line, add character before print
      for (i = line_length ; i > pos ; i--) {
        line_buffer[i] = line_buffer[i - 1];

      }
      line_buffer[i] = ch;
      //goes to right end
      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }

      //clear screen, ready to reprint
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);
      }

      // delete character
      for (i = 0; i < line_length; i++) {
        ch = ' ';
        write(1, &ch, 1);

      }
      // cursor goes to the very beginning
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);

      }
      line_length++;

      //print nre line that already modified
      for (i = 0; i < line_length; i++) {
        ch = line_buffer[i];
        write(1, &ch, 1);

      }
      pos++;

      //set cursor to pos, match their position
      for (i = line_length ; i > pos; i--) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

      }


    }
    else if (ch == 10) {
      // <Enter> was typed. Return line

      //store the line on the screen
      //if(line_buffer){

      //set pos and cursor to zero
      if(line_length <= 0){
        flag = -1;
        write(1,&ch,1);
        break;
      }
      if(line_buffer[line_length] != '\0'){
        line_buffer[line_length] = '\0';
      }
      my_history[my_history_index] = strdup(line_buffer);
      //printf("Enter my_history[%d]:%s\n",my_history_index,my_history[my_history_index]);
      my_history_index++;
      my_history_length ++;//= sizeof(my_history) / sizeof(char*);

      //     printf("length: %d\n",my_history_length);
      //     printf("index: %d\n",my_history_index);
      //}
      // Print newline
      write(1, &ch, 1);
      //   printf("Enter:\npos:%d\n", pos);
      //     printf("line_length:%d\n",line_length);
      //     printf("line_buffer:%s\n",line_buffer);
      flag = -1;
      //    pos = 0;
      // line_length = 0;
      // printf("strlen:%d\n",strlen(my_history[my_history_index-1]));

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0] = 0;
      break;

    }
    else if (ch == 1) {
      // printf("pos:%d, line_length:%d",pos,line_length);
      //printf("ctrl-A\n");
      //HOME, goes to very beginning
      if (pos == 0) {
        continue;

      }
      int k;
      for (k = pos; k > 0 ; k--) {
        ch = 8;
        write(1, &ch, 1);

      }
      pos = 0;

    } else if (ch == 5) {
      //END,CTRL-E
      if (line_length == 0 || pos == line_length) {
        continue;

      }
      int k;
      for (k = pos; k < line_length ; k++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }
      pos = line_length;

    }
    else if (ch == 127 || ch == 8) {
     // <backspace> was typed. Remove previous character read.

      //check cursor goes to the first space
      if (pos <= 0) {
        continue;
      }

      //modify line_buffer before print
      int i;
      //modify the line, add character before print
      for (i = pos - 1; i < line_length; i++) {
        line_buffer[i] = line_buffer[i + 1];

      }

      line_buffer[line_length - 1] = '\0';
      //printf("buffer: %s\n", line_buffer);


      //cursor goes to right end
      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }



      //clear screen, ready to reprint
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);
      }

      // delete character
      for (i = 0; i < line_length; i++) {
        ch = ' ';
        write(1, &ch, 1);

      }
      // cursor goes to the very beginning
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);

      }
      line_length--;


      //print the line that already modified
      for (i = 0; i < line_length; i++) {
        ch = line_buffer[i];
        write(1, &ch, 1);

      }

      pos--;

      for (i = line_length ; i > pos; i--) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

      }







      // Go back one character


    }else if(ch == 4){
      //Delete
      if (pos >= line_length - 1) {
        continue;
      }

      //modify line_buffer before print
      int i;
      //modify the line, add character before print
      for (i = pos ; i < line_length; i++) {
        line_buffer[i] = line_buffer[i + 1];

      }

      line_buffer[line_length - 1] = '\0';
      //printf("buffer: %s\n", line_buffer);


      //cursor goes to right end
      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }



      //clear screen, ready to reprint
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);
      }

      // delete character
      for (i = 0; i < line_length; i++) {
        ch = ' ';
        write(1, &ch, 1);

      }
      // cursor goes to the very beginning
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);

      }
      line_length--;


      //print the line that already modified
      for (i = 0; i < line_length; i++) {
        ch = line_buffer[i];
        write(1, &ch, 1);

      }


      for (i = line_length ; i > pos; i--) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

      }




    }
    else if (ch == 27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1;
      char ch2;
      char ch3;
      read(0, &ch1, 1);
      read(0, &ch2, 1);

    if(ch1 == 79 && ch2 == 72){
    //HOME

      if (pos == 0) {
        continue;

      }
      int k;
      for (k = pos; k > 0 ; k--) {
        ch = 8;
        write(1, &ch, 1);

      }
      pos = 0;


    }else if(ch1 == 91 && ch2 == 51 && ch3 == 126){
    	//DELETE
    	
      if (pos >= line_length - 1) {
        continue;
      }

      //modify line_buffer before print
      int i;
      //modify the line, add character before print
      for (i = pos ; i < line_length; i++) {
        line_buffer[i] = line_buffer[i + 1];

      }

      line_buffer[line_length - 1] = '\0';
      //printf("buffer: %s\n", line_buffer);


      //cursor goes to right end
      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }



      //clear screen, ready to reprint
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);
      }

      // delete character
      for (i = 0; i < line_length; i++) {
        ch = ' ';
        write(1, &ch, 1);

      }
      // cursor goes to the very beginning
      for (i = 0; i < line_length; i++) {
        ch = 8;
        write(1, &ch, 1);

      }
      line_length--;


      //print the line that already modified
      for (i = 0; i < line_length; i++) {
        ch = line_buffer[i];
        write(1, &ch, 1);

      }


      for (i = line_length ; i > pos; i--) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

      }

    
    }
    else if (ch1 == 91 && ch2 == 65) {
        // Up arrow. Print next line in history.
        //     printf("UP Arrow\n");
        // Erase old line
        // Print backspaces
int i = 0;
  /*      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

     }

   */     
        for (i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // Print spaces on top
        for (i = 0; i < line_length; i++) {
          ch = ' ';
          write(1, &ch, 1);
        }

        // Print backspaces
        for (i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        //test
        int len;
        //check history_length is empty
        if (my_history_length > 0) {
          // if (flag >= 0) {
          flag = 1;
          if (my_history_index >= 0) {
            my_history_index--;
          }

          if (my_history_index < 0) {
            my_history_index = 0;

          }
          strcpy(line_buffer, my_history[my_history_index]);
          len = strlen(line_buffer);
          if (line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
          }
          //      printf("my_history[%d]:%s\n",my_history_index,my_history[my_history_index]);
          line_length = strlen(line_buffer);
          // line_length--;
          //   printf("length:%d\n",line_length);
          write(1, line_buffer, line_length);
  //        pos = line_length;

        }

        // echo line

      } else if (ch1 == 91 && ch2 == 66) {
        //DOWN ARROW,look down history
        //printf("DOWN Arrow\n");
        // Erase old line

        // move cursor to the begining of the line

        int i = 0;
        int len;

  /*      for (i = pos; i < line_length; i++) {
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);

      }
*/

        for (i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        // replace the character to ' '
        for (i = 0; i < line_length; i++) {
          ch = ' ';
          write(1, &ch, 1);
        }

        // move cursor to the begining
        for (i = 0; i < line_length; i++) {
          ch = 8;
          write(1, &ch, 1);
        }

        if (my_history_length > 0) {
          if (flag >= 0) {
            if (my_history_index < my_history_length) {
              my_history_index ++;

            }
            //   printf("my_history[%d]:%s\n",my_history_index,my_history[my_history_index]);
            //   printf("my_history_length:%d\n",my_history_length);
            if (my_history[my_history_index] == NULL) {
              //           printf("Down my_history[%d]:%s\n",my_history_index,my_history[my_history_index]);
              //         printf("my_history_length:%d\n",my_history_length);
              my_history_index = my_history_length;
              ch = ' ';
              write(1, &ch, 1);
              ch = 8;
              write(1, &ch, 1);
              line_length = 0;
              continue;

            }

            strcpy(line_buffer, my_history[my_history_index]);
            len = strlen(line_buffer);
            if (line_buffer[len - 1] == '\n') {
              line_buffer[len - 1] = '\0';
            }
            line_length = strlen(line_buffer);
            write(1, line_buffer, line_length);
  //          pos = line_length;
            //else if (history_index < 0) {
            //history_index = 0;
            //}

          } else if (flag < 0) {
            my_history_index = my_history_length - 1;
            continue;
          }
        }

        // Copy line from history


      } else if (ch1 == 91 && ch2 == 68) {
        //LEFT ARROW
        //printf("LEFT Arrow\n");
        if (pos == 0) {
          continue;

        }
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 68;
        write(1, &ch, 1);

        pos--;


      } else if (ch1 == 91 && ch2 == 67) {
        //RIGHT ARROW
        //printf("Right Arrow\n");
        // printf("line_length:%d\n",line_length);
        if (pos == line_length) {
          continue;
        }
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);
        pos++;


      }

    }

  }

/*
  if(pos > 0){
        ch = 8;
        write(1, &ch, 1);
        pos --;

      }
      if(pos < 0){
        ch = 27;
        write(1, &ch, 1);
        ch = 91;
        write(1, &ch, 1);
        ch = 67;
        write(1, &ch, 1);
        pos++;
      }

*/
  // Add eol and null char at the end of string
  line_buffer[line_length] = 10;
  line_length++;
  line_buffer[line_length] = 0;


  tcsetattr(0, TCSANOW, &newTerminal);
  return line_buffer;
}

