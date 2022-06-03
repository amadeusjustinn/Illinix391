/*
  References:
  https://wiki.osdev.org/PS/2_Keyboard
  https://wiki.osdev.org/%228042%22_PS/2_Controller FOR PORTS
*/

#include "keyboard.h"

// Keyboard mapping from: http://www.osdever.net/bkerndev/Docs/keyboard.htm
unsigned char keyboard_map[MAP_SIZE] =
{
    0,  0, /* Escape */ '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', 0,	/* Backspace */
  0,			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

// Boolean flag for each key; 1 if key is currently being pressed; 0 if not; used for special keys
static volatile unsigned char keys_pressed[NUM_TERMINALS][BUFFER_SIZE];
// Boolean flag to determine if caps_lock is on; 1 if on; 0 if off
// Only switches on press of caps_lock; not release
static volatile unsigned char caps_lock_toggle;
// Stores all the special characters that are affected by shift
static unsigned char shiftable_keys[21] = {'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
                                         '[', ']', '\\', ';', '\'', ',', '.', '/'};
// Stores the ASCII offsets for each shiftable key to get the special character when shift is pressed
static int shiftable_keys_offset[21] = {30, -16, 14, -16, -16, -16, 40, -17, -14, -17, -7, 50, -18, 
                                        32, 32, 32, -1, -5, 16, 16, 16};
// Stores up to 1000 previously entered buffers
static volatile unsigned char history[HISTORY_SIZE][BUFFER_SIZE];
// Current number of previously entered buffers
static volatile uint32_t num_history;
// Pointer to the entry in the history array; moved using up and down arrow keys
static volatile uint32_t cur_history_position;

/* Helper function to determine if a character is a letter
   Input: Character to check
   Output: 1 if letter; 0 else
*/
int is_letter(char character){
    if ((character >= 'a') && (character <= 'z')){
      return 1;
    }
    return 0;
}

/* Helper function to determine if a character is a special character
   Input: Character to check
   Output: integer representing the offset to add to the character to get shifted special characeter; 0 if not a special character
*/
int is_special_char(char character){
    unsigned int i;
    // Search through the shiftable keys array and determine if the current character is a shiftable key
    for (i = 0; i < 21; i++){
        if (shiftable_keys[i] == character){
            // Return the offset needed to add to the character to get the shifted character
            return shiftable_keys_offset[i];
        }
    }
    return 0;
}

/* Clears the keyboard buffer; resets keyboard buffer size to 0
   Input: None
   Output: None
*/
void clear_current_buffer(void){
    unsigned int i;
    for (i = 0; i < BUFFER_SIZE; i++){
        terminals[current_terminal_view].keyboard_buffer[i] = '\0';
    }
    terminals[current_terminal_view].keyboard_buffer_size = 0;
}

/* Autocompletes when pressing TAB by copying the filename into current keyboard buffer
   Input: None
   Output: -1 if fail; 0 if success
*/
int32_t autocomplete(void){
    // Don't autocomplete if nothing is typed yet
    if (terminals[current_terminal_view].keyboard_buffer_size == 0){
        return -1;
    }
    // Get the string in the current keyboard buffer
    uint8_t* fname = terminals[current_terminal_view].keyboard_buffer;
    int i;
    for(i = 0; i < global_boot_block_t->entries_number;i++){
		// Find the closest file name matching the buffer
		if(strncmp((char*)fname, (char*)global_boot_block_t->block_entires[i].fileName, terminals[current_terminal_view].keyboard_buffer_size) == 0){
			// Copy the rest of the name into the buffer
            int j;
            for (j = terminals[current_terminal_view].keyboard_buffer_size; j < strlen((char*)global_boot_block_t->block_entires[i].fileName); j++){
                terminals[current_terminal_view].keyboard_buffer[j] = global_boot_block_t->block_entires[i].fileName[j];
                terminal_putc(terminals[current_terminal_view].keyboard_buffer[j]);
            }
            terminals[current_terminal_view].keyboard_buffer_size = strlen((char*) global_boot_block_t->block_entires[i].fileName);
			return 0;
		}
	}
    return -1;
}

/* Get history when pressing up or down
   Input: 0 if press up ; 1 if press down (more recent history)
   Output: 0 if success; -1 if fail
*/
int32_t get_history(int direction){
    if (num_history == 0){
        return -1;
    }
    int i;
    // Press up
    if ((direction == 0) && (cur_history_position != 0)){
        cur_history_position--;
    // Press down
    }else if ((direction == 1) && (cur_history_position < (num_history - 1))){
        cur_history_position++;
    // At the end
    }else if ((direction == 1) && (cur_history_position == num_history)){
        cur_history_position = num_history - 1;
    }
    // Remove the currently typed buffer
    for (i = 0; i < terminals[current_terminal_view].keyboard_buffer_size; i++){
        terminal_putc_backspace();
    }
    terminals[current_terminal_view].keyboard_buffer_size = 0;
    // Copy the history
    for (i = 0; i < strlen((char *) history[cur_history_position]); i++){
        terminals[current_terminal_view].keyboard_buffer[i] = history[cur_history_position][i];
        terminal_putc(terminals[current_terminal_view].keyboard_buffer[i]);
    }
    terminals[current_terminal_view].keyboard_buffer_size = strlen((char *) history[cur_history_position]);
    return 0;
}

/* Initialize all terminals to 0
   Input: None
   Output: None
*/
void clear_all_terminals(void){
    unsigned int i = 0;
    unsigned int j = 0;
    for (i = 0; i < NUM_TERMINALS; i++){
        terminals[i].current_pid = -1;
        // Clear buffer
        for (j = 0; j < MAP_SIZE; j++){
            terminals[i].keyboard_buffer[j] = '\0';
        }
        // No characters in buffer
        terminals[i].keyboard_buffer_size = 0;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
        terminals[i].factor = FACTOR_INIT;
        terminals[i].interrupt_count = 0;
    }
    for (i = 0; i < MAP_SIZE; i++){
        keys_pressed[0][i] = 0;
        keys_pressed[1][i] = 0;
        keys_pressed[2][i] = 0;
    }
}

/*  Initializing keyboard
    Inputs: none
    Outputs: none
    Description: Enables IRQ for keyboard interrupts
*/
void keyboard_init(void) {
    // Start with terminal 0
    current_terminal_view = 0;
    // Caps_lock is off
    caps_lock_toggle = 0;
    // Clear the keyboard buffer
    clear_all_terminals();
    unsigned int i = 0;
    unsigned int j = 0;
    // Clear history buffer
    for (i = 0; i < HISTORY_SIZE; i++){
        for (j = 0; j < BUFFER_SIZE; j++){
            history[i][j] = 0;
        }
    }
    num_history = 0;
    cur_history_position = 0;
    // Enable the keyboard on the PIC
    enable_irq(KEYBOARD_IRQ);
}

/*  Keyboard interrupt handler 
    Inputs: none
    Outputs: none
    Description: Reads from data_port and maps entry to a character on keyboard_map
*/
void keyboard_handler(void) {
    cli();
    unsigned char status = inb(STATUS_PORT); // Grabbing the status
    if (status & 0x1) { // If last bit = 1, handle a new keypress
        unsigned char keycode = inb(DATA_PORT); // Grabbing the data
        // 128 is the number of entries in our keyboard map
        if (keycode >= MAP_SIZE) {
            // Clear buffer when enter is released
            if (keycode == ENTER_REL){
                clear_current_buffer();
            }
            // Keycode is released; subtract by 128 to get the key being released
            keycode = keycode - 0x80;
            // Set the flag for the keycode to 0 since released
            keys_pressed[current_terminal_view][keycode] = 0;
            // Sending an EOI to reset PIC busy state
            send_eoi(KEYBOARD_IRQ);
            return; 
        }
        // Key is pressed so set the flag for the keycode to 1
        keys_pressed[current_terminal_view][keycode] = 1;
        // If key is caps_lock
        if (keycode == CAPS_LOCK){
          // Invert the toggle
          if (caps_lock_toggle == 0){
            caps_lock_toggle = 1;
          }else{
            caps_lock_toggle = 0;
          }
        }
        // Finding the entry associated with data in data port
        char character = keyboard_map[keycode];
        // If the character is a letter, need to check shift and caps_lock
        if (is_letter(character)){
            // If shift is pressed XOR caps_lock is on, convert to uppercase; otherwise, shift and caps_lock is both on so keep as lowercase
            if (!((keys_pressed[current_terminal_view][LEFT_SHIFT] == 1) || (keys_pressed[current_terminal_view][RIGHT_SHIFT] == 1)) != !(caps_lock_toggle == 1)){
              character -= 32;
            }
        }
        // If shift is pressed, need to check if character is special character and add offset if it is
        if ((keys_pressed[current_terminal_view][LEFT_SHIFT] == 1) || (keys_pressed[current_terminal_view][RIGHT_SHIFT] == 1)){
            character += is_special_char(character);
        }
        // Only print for letters and special characters; not function keys such as shift
        if (character != 0){
            // Add the character to the buffer as long as CTRL + l/L is not pressed
            if ((terminals[current_terminal_view].keyboard_buffer_size < 127) && !((keys_pressed[current_terminal_view][CTRL] == 1) && (keys_pressed[current_terminal_view][L] == 1))){
                terminals[current_terminal_view].keyboard_buffer[terminals[current_terminal_view].keyboard_buffer_size] = character;
                terminals[current_terminal_view].keyboard_buffer_size++;
                terminal_putc(character);
            // If already inputted 127 characters, only accept characters if it is a newline
            }else if ((terminals[current_terminal_view].keyboard_buffer_size == 127) && (character == '\n')){
                terminals[current_terminal_view].keyboard_buffer[terminals[current_terminal_view].keyboard_buffer_size] = character;
                terminals[current_terminal_view].keyboard_buffer_size++;
                terminal_putc(character);
            }
        }

        // If backspace is pressed, erase the previous character from buffer
        if (keys_pressed[current_terminal_view][BACKSPACE] == 1){
            if (terminals[current_terminal_view].keyboard_buffer_size > 0){
                terminals[current_terminal_view].keyboard_buffer_size--;
                // Erase the previous character on console
                terminal_putc_backspace();
                terminals[current_terminal_view].keyboard_buffer[terminals[current_terminal_view].keyboard_buffer_size] = 0;
            }
        }

        // Clear video memory when CTRL + l/L is pressed
        if ((keys_pressed[current_terminal_view][CTRL] == 1) && (keys_pressed[current_terminal_view][L] == 1)){
            terminal_clear();
        }

        // Autocomplete if TAB is pressed
        if (keys_pressed[current_terminal_view][TAB] == 1){
            autocomplete();
        }
        
        // Get history
        if (keys_pressed[current_terminal_view][UP] == 1){
            get_history(0);
        }else if (keys_pressed[current_terminal_view][DOWN] == 1){
            get_history(1);
        }

        // Switch terminal
        if ((keys_pressed[0][ALT] == 1) || (keys_pressed[1][ALT] == 1) || (keys_pressed[2][ALT] == 1)){
            if (keys_pressed[current_terminal_view][F1] == 1){
                if (current_terminal_view != 0){
                    keys_pressed[current_terminal_view][F1] = 0;
                    terminal_switch(0);
                }
            }else if (keys_pressed[current_terminal_view][F2] == 1){
                if (current_terminal_view != 1){
                    keys_pressed[current_terminal_view][F2] = 0;
                    terminal_switch(1);
                }
            }else if (keys_pressed[current_terminal_view][F3] == 1){
                if (current_terminal_view != 2){
                    keys_pressed[current_terminal_view][F3] = 0;
                    terminal_switch(2);
                }
            }
        }

    }
    // Enables IRQ for keyboard on PIC
    send_eoi(KEYBOARD_IRQ);
    sti();
}

/*  Terminal open; initializes terminal variables; nothing in this case
    Inputs: filename
    Outputs: 0
*/
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/*  Terminal close; clears terminal variables; nothing in this case
    Inputs: fd
    Outputs: -1
*/
int32_t terminal_close(int32_t fd){
    return -1;
}

/*  Terminal read; reads from keyboard buffer into input buffer
    Inputs: fd: file descriptor used
            buf: buffer to write to
            nbytes: number of characters to copy from keyboard buffer to input buffer
    Outputs: Number of bytes written to buffer
*/
int32_t terminal_read(int32_t fd, void * buf, int32_t nbytes){
    sti();
    unsigned char * buf_ptr = (unsigned char *) buf;
    // Wait until enter is pressed
    while (keys_pressed[current_terminal_run][ENTER] != 1 || current_terminal_run != current_terminal_view){}
    cli();
    int num_byte = 0;
    unsigned int i;
    for (i = 0; i < nbytes; i++){
        buf_ptr[i] = '\0';
    }
    // Filled up history array
    if (num_history == HISTORY_SIZE){
        int j;
        // Copy over half of the history
        for (j = 0; j < HISTORY_SIZE/2; j++){
            memcpy((char *) history[j], (char *) history[j + HISTORY_SIZE/2], BUFFER_SIZE);
        }
        // Clear the other half
        for (j = HISTORY_SIZE/2; j < HISTORY_SIZE; j++){
            int k;
            for (k = 0; k < BUFFER_SIZE; k++){
                history[j][k] = 0;
            }
        }
        num_history = HISTORY_SIZE/2;
    }
    // Copy keyboard buffer to buffer
    for (i = 0; ((i < nbytes) && (i < terminals[current_terminal_view].keyboard_buffer_size)); i++){
        // Save the entry to history excluding the newline character
        if (terminals[current_terminal_view].keyboard_buffer[i] != '\n'){
            history[num_history][i] = terminals[current_terminal_view].keyboard_buffer[i];
        }
        // Stop copying once it hits new line or null character
        if (terminals[current_terminal_view].keyboard_buffer[i] == '\0'){
            break;
        }
        buf_ptr[i] = terminals[current_terminal_view].keyboard_buffer[i];
        num_byte++;
    }
    // Only add non empty entries
    if (strlen((char *) history[num_history]) != 0){
        num_history++;
    }
    cur_history_position = num_history;
    // Clear buffer when done
    clear_current_buffer();
    keys_pressed[current_terminal_run][ENTER] = 0;
    sti();
    // Return number of bytes copied
    return num_byte;
}

/*  Terminal write; writes from keyboard buffer to console
    Inputs: fd: file descriptor used
            buf: buffer containing characters to write to console
            nbytes: number of characters to write to console
    Outputs: Number of bytes written to console
*/
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes){
    cli();
    // Load video data into video memory directly
    if (current_terminal_run == current_terminal_view){
        video_mem = (char *) VIDEO;
    }else{
        // Load video data into terminal video buffer
        video_mem = (char *) terminal_address[current_terminal_run];
    }
    unsigned char * buf_ptr = (unsigned char *) buf;
    if (buf_ptr == NULL || nbytes < 0){
        return -1;
    }
    int num_byte = 0;
    unsigned int i;
    // Print the character to the console
    for (i = 0; i < nbytes; i++){
        putc(buf_ptr[i]);
        // Check if the buffer contains null character
        if (buf_ptr[i] != '\0'){
            num_byte++;
        }
    }
    sti();
    return num_byte;
}
