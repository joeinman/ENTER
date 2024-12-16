#include <stdlib.h>
#include <stdio.h>

#include <hardware/gpio.h>
#include <bsp/board_api.h>
#include <tusb.h>

#include "usb_descriptors.h"

void hid_task();
static void send_enter();

static uint32_t last_debounce_time = 0;
static uint32_t repeat_start_time = 0;

#define DEBOUNCE_DELAY 100    // 50ms debounce delay
#define REPEAT_DELAY 600     // 200ms before repeating starts
#define REPEAT_INTERVAL 30   // 20ms interval for holding the button
#define BOARD_BUTTON_PIN 0   // Button is on pin 0

bool button_pressed = false;

//--------------------------------------------------------------------+
// Main Entry
//--------------------------------------------------------------------+

int main()
{
    board_init();
    tud_init(BOARD_TUD_RHPORT);
    if (board_init_after_tusb)
        board_init_after_tusb();

    gpio_set_dir(BOARD_BUTTON_PIN, GPIO_IN);

    while (1)
    {
        tud_task();
        hid_task();

        // Read the button state
        bool current_state = gpio_get(BOARD_BUTTON_PIN);

        // Handle debounce and initial press
        if (current_state && !button_pressed)
        {
            if ((board_millis() - last_debounce_time) > DEBOUNCE_DELAY)
            {
                send_enter(); // Send Enter for the initial press
                button_pressed = true; // Mark as pressed
                repeat_start_time = board_millis(); // Start repeat timer
                last_debounce_time = board_millis();
            }
        }
        else if (!current_state)
        {
            button_pressed = false; // Reset button state when released
        }

        // Handle repeat sending while the button is held
        if (button_pressed && (board_millis() - repeat_start_time) > REPEAT_DELAY)
        {
            if ((board_millis() - last_debounce_time) > REPEAT_INTERVAL)
            {
                send_enter(); // Send Enter repeatedly
                last_debounce_time = board_millis();
            }
        }
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en) { (void) remote_wakeup_en; }
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
    if (!tud_hid_ready()) return;

    uint8_t keycode[6] = { 0 };

    keycode[0] = HID_KEY_ENTER;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

    board_delay(10); // Short delay to simulate a key press
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL); // Release key
}

void hid_task(void)
{
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) return;
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    if (tud_suspended() && btn)
    {
        tud_remote_wakeup();
    }
    else
    {
        send_hid_report(REPORT_ID_KEYBOARD, btn);
    }
}

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

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}
