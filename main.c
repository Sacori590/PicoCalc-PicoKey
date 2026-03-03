/* 
Sacori590
https://github.com/Sacori590/PicoCalc-PicoKey
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

#include <time.h>

#include <hardware/spi.h>
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "i2ckbd.h"
#include "lcdspi.h"

#define LEDPIN 25

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+




uint8_t mouseOnOff = 1;
void hid_task(void);

const uint8_t special_pico_calc_key[256][2] = {
    [165] = { KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_CONTROL_LEFT },
    [161] = { KEYBOARD_MODIFIER_LEFTALT,  HID_KEY_ALT_LEFT },
    [177] = { 0, HID_KEY_ESCAPE },
    [212] = { 0, HID_KEY_DELETE },
    [210] = { 0, HID_KEY_HOME },

    [208] = { KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C },

    [213] = { 0, HID_KEY_END },

    [129] = { 0, HID_KEY_F1 },
    [130] = { 0, HID_KEY_F2 },
    [131] = { 0, HID_KEY_F3},
    [132] = { 0, HID_KEY_F4 },
    [133] = { 0, HID_KEY_F5 },
    [134] = { 0, HID_KEY_F6 },
    [135] = { 0, HID_KEY_F7 },
    [136] = { 0, HID_KEY_F8 },
    [137] = { 0, HID_KEY_F9 },
    [144] = { 0, HID_KEY_F10 },
    
    [181] = {0, HID_KEY_ARROW_UP},
    [180] = {0, HID_KEY_ARROW_LEFT},
    [182] = {0, HID_KEY_ARROW_DOWN},
    [183] = {0, HID_KEY_ARROW_RIGHT}
};

/*------------- MAIN -------------*/
int main(void)
{
  //--------------pico init--------------
  set_sys_clock_khz(200000, true);
  stdio_init_all();

  uart_init(uart0, 115200);

  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);  // 8-N-1
  uart_set_fifo_enabled(uart0, false);

  init_i2c_kbd();
  lcd_init();

  gpio_init(LEDPIN);
  gpio_set_dir(LEDPIN, GPIO_OUT);

  lcd_clear();
  lcd_print_string("F10: Toggle Mouse to Arrow\nF1: Left Click\nF2: Right Click\nBreak: Ctrl+c\nThe mouse is holded,consider type twice\n");

  
  // --------------HID init--------------
  board_init();
  
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }
  

  while (1)
  {
    tud_task(); // tinyusb device task
    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  lcd_print_string("Connected...\n"); 
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  set_kbd_backlight(255);
  lcd_print_string("restart everything I'm dead...\n");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

uint8_t random_push(){
  srand(time((NULL)));
  return (uint8_t)(rand()%123+48);
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+
static void send_hid_report(uint8_t report_id, int16_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready()) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn!=-1 )
      {
        
        uint8_t const conv_table[128][2] = { HID_ASCII_TO_KEYCODE };
        uint8_t keycode[6] = { 0 };
        uint8_t modifier = 0;
        
        if (127 >= btn && btn >= 0){
          if (conv_table[btn][0]) {modifier = KEYBOARD_MODIFIER_LEFTSHIFT;};
            keycode[0] = conv_table[btn][1];
        }
        else {
          modifier = special_pico_calc_key[btn][0];
          keycode[0] = special_pico_calc_key[btn][1];
        }
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
        has_keyboard_key = true;
        
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
          has_keyboard_key = false;
      }
      return;
    }


    case REPORT_ID_MOUSE:
    {
      int8_t x = 0;
      int8_t y = 0;
      uint8_t v = 20;
      static uint8_t action = 0x00;
      
      switch(btn){
        case -1:
          break;
        case 181 ://UP
          y=-1;break;
        case 183 ://RIGHT
          x=1;break;
        case 180 ://LEFT
          x=-1;break;
        case 182 ://DOWN
          y=1;break;
        case 130 : //CLICK RIGHT
          action = (action ==MOUSE_BUTTON_LEFT || action == 0x00) ? MOUSE_BUTTON_RIGHT : 0x00;
          break;
        case 129 :
          action = (action ==MOUSE_BUTTON_RIGHT || action == 0x00) ? MOUSE_BUTTON_LEFT : 0x00;
          break;
      }
      
      
      tud_hid_mouse_report(REPORT_ID_MOUSE, action, x*v, y*v, 0, 0);
    }//je vais ttireerr unesssssssC bqklle
      

    default:
     return;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete


void hid_task(void)
{
  // Poll every 20ms
  const uint32_t interval_ms = 5;
  static uint32_t start_ms = 0;

  
  if ( board_millis() - start_ms < interval_ms) {return;}; // not enough time
    
    start_ms += interval_ms;
    const int16_t btn = (int16_t)(read_i2c_kbd());
    //const int16_t btn = random_push();
    uint8_t event = (mouseOnOff == 1 && (180 <= btn && btn <= 183 || 129==btn || 130 ==btn)) ? REPORT_ID_MOUSE : REPORT_ID_KEYBOARD;
    if (btn == 144){mouseOnOff=!mouseOnOff;};
    
    if (btn!=-1 && btn <128){
      
      lcd_putc(0,btn);
      count = (count+1)%256;
    if (count == 255) {lcd_clear();};
    
    }
  // Remote wakeup
    if ( tud_suspended() && btn)
    {
      // Wake up host if we are in suspend mode
      // and REMOTE_WAKEUP feature is enabled by host
      tud_remote_wakeup();
    }
    else 
    {
      
      // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb(n)
      send_hid_report(event, btn);
    }
}


// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    //send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}
// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
}


