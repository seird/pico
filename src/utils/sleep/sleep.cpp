// https://ghubcoder.github.io/posts/awaking-the-pico/

#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"
#include "hardware/pll.h"

#include "utils/sleep.h"


static void
save_values(uint * scb, uint * en0, uint * en1)
{
    /* store clock registers */
    *scb = scb_hw->scr;
    *en0 = clocks_hw->sleep_en0;
    *en1 = clocks_hw->sleep_en1;
}


static void
restore_values(uint * scb, uint * en0, uint * en1)
{
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    /* restore clock registers */
    scb_hw->scr             = *scb;
    clocks_hw->sleep_en0    = *en0;
    clocks_hw->sleep_en1    = *en1;

    clocks_init();
}


static void
go_to_sleep(sleepconfig_t * cfg)
{
    sleep_run_from_xosc();
    
    switch (cfg->mode) {
        case SM_SLEEP:
            rtc_init();
            rtc_set_datetime(&cfg->_t_start);
            sleep_goto_sleep_until(&cfg->_t_end, NULL);
            break;
        case SM_DORMANT:
            sleep_goto_dormant_until_pin(cfg->pin_wakeup, cfg->edge, cfg->active);
            break;
        default:
            return;
    }
}


void
sleep_run(sleepconfig_t * cfg)
{
    cfg->_t_start = {
        .year  = 2021,
        .month = 05,
        .day   = 01,
        .dotw  = 6,
        .hour  = 00,
        .min   = 00,
        .sec   = 00,
    };
    cfg->_t_end = {
        .year  = 2021,
        .month = 05,
        .day   = 01,
        .dotw  = 6,
        .hour  = cfg->hours,
        .min   = cfg->minutes,
        .sec   = cfg->seconds,
    };

    uint scb, en0, en1;
    save_values(&scb, &en0, &en1);

    while (true) {
        if (cfg->mode == SM_DEFAULT) {
            sleep_ms(1000*(cfg->hours*60*60 + cfg->minutes*60 + cfg->seconds));
        } else {
            go_to_sleep(cfg);
            restore_values(&scb, &en0, &en1);
        }

        if (cfg->loop_function) {
            cfg->loop_function(cfg->arg);
        }
    }
}
