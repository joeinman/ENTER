//--------------------------------------------------------------------+
// Includes
//--------------------------------------------------------------------+

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <bsp/board_api.h>
#include <tusb.h>

#include "usb_descriptors.h"

void hid_task();
static void send_enter();

//--------------------------------------------------------------------+
// Main Entry
//--------------------------------------------------------------------+

int main()
{
    board_init();
    tud_init(BOARD_TUD_RHPORT);
    if (board_init_after_tusb)
        board_init_after_tusb();

    while (1)
    {
        tud_task();
        hid_task();

        if (tud_mounted())
        {
            send_enter();
        }
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {}

// Invoked when device is unmounted
void tud_umount_cb(void) {}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) { (void) remote_wakeup_en; }

// Invoked when usb bus is resumed
void tud_resume_cb(void) {}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
    (void) report_id;
    (void) btn;
}

static void send_enter(void)
{
  // skip if HID is not ready yet
  if (!tud_hid_ready()) return;

  uint8_t keycode[6] = { 0 };

  // Get the current character's HID keycode
  keycode[0] = HID_KEY_ENTER;
  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

  // Release the key after a short delay
  board_delay(10);
  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if (board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD, btn);
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
    send_hid_report(next_report_id, board_button_read());
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
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}
