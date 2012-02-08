#ifndef _FM_RADIO_H_
#define  _FM_RADIO_H_

#include "qndriver.h"

// init the fm module's clock and mode, set to mute 
extern void fm_init();

// auto scan fm radio from 87.5Mhz  to 108Mhz, and reture the avaliable radio num. 
extern   UINT16* fm_rx_scan(int *cnt, int db);

// tune the receive freq to @freq, example : 87.5Mhz=8750
extern void fm_rx_freq(int freq);

// show the radio's signal 
extern UINT8 fm_get_rx_rssi(UINT16 freq);

// change the FM mode to rx mode, and set the transmit  audio at @freq.
extern void fm_tx_freq(int freq);

//  set FM to sleep mode, and close fm device
extern void fm_clean();

// turn codec on/off
// @value  0 --- turn off 1 --- turn on
extern int linein_to_hp(int value);

#endif //  _FM_RADIO_H_

