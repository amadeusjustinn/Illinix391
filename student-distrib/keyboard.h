#ifndef KEYBOARD_H
#define KEYBOARD_H
/*
  References:
  https://wiki.osdev.org/PS/2_Keyboard
  https://wiki.osdev.org/%228042%22_PS/2_Controller FOR PORTS
*/

#include "lib.h"
#include "i8259.h"
#include "scheduling.h"
#include "system_call.h"

/* For PS/2 Keyboard: Data port resides at 0x60; Status port at 0x64. */
#define DATA_PORT 0x60
#define STATUS_PORT 0x64

/* Keyboard map size holds 128 entries */
#define MAP_SIZE 128

/* Keyboard IRQ = 1 on PS/2 port (cannot be changed)   */
#define KEYBOARD_IRQ 1

/* Keycodes for shift pressed */
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36

/* Keycode for caps_lock press */
#define CAPS_LOCK 0x3A

/* Keycode for control pressed */
#define CTRL 0x1D

/* Keycode for enter pressed */
#define ENTER 0x1C

#define ENTER_REL 0x9C

/* Keycode for backspace pressed */
#define BACKSPACE 0x0E

/* Keycode for l/L pressed */
#define L 0x26

/* Size of keyboard buffer */
#define BUFFER_SIZE 128

/* Number of terminals */
#define NUM_TERMINALS 3

/* Keycode for F1-F3 pressed */
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

/* Keycode for L/R ALT pressed */
#define ALT 0x38

/* Keycode for TAB pressed */
#define TAB 0x0F

/* Keycode for arrow keys pressed */
#define UP 0x48
#define DOWN 0x50

/* Max number of history entries */
#define HISTORY_SIZE 100

typedef struct terminal{
    int32_t current_pid;
    int32_t screen_x;
    int32_t screen_y;
    int32_t view_screen_x;
    int32_t view_screen_y;
    int32_t interrupt_count;
    int32_t factor;
    // Keyboard buffer for each terminal
    uint8_t keyboard_buffer[BUFFER_SIZE];
    // Stores the current number of characters in the buffer for each terminal
    int32_t keyboard_buffer_size;
} terminal_t;

terminal_t terminals [NUM_TERMINALS];
// Stores the value of the current terminal that is being viewed
volatile int current_terminal_view;

/* Helper function to determine if the character is a letter; used to determine if need to check shift and caplock */
int is_letter(char character);

/* Helper function to determine if the character is a special character; used to determine if need to check shift */
int is_special_char(char character);

/* Autocompletes the keyboard buffer when pressing TAB */
int32_t autocomplete(void);

/* Gets history when pressing up or down*/
int32_t get_history(int direction);

/* Keyboard Initializer */
extern void keyboard_init(void);

/* Keyboard interrupt handler */
extern void keyboard_handler(void);

/* Initializes terminal */
extern int32_t terminal_open(const uint8_t* filename);

/* Clears terminal variables */
extern int32_t terminal_close(int32_t fd);

/* Reads from keyboard buffer into terminal buffer */
extern int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes);

/* Writes to the screen from buffer */
extern int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);

#endif
