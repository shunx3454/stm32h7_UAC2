#ifndef __BSP_GT911_H_
#define __BSP_GT911_H_

#include <stdint.h>
#include "main.h"

typedef __PACKED_STRUCT
{
    __IO uint8_t TrackID;
    uint16_t px;
    uint16_t py;
    uint16_t pSize;
    __IO uint8_t Reserve;
}TouchPointInfo;

extern TouchPointInfo TouchPoints[];

/* GT911 some registers */
#define GT9XX_COMMAND 0x8040
#define GT9XX_CONFIGVERSION 0x8047
#define GT9XX_OUTMAX_XL 0x8048
#define GT9XX_OUTMAX_XH 0x8049
#define GT9XX_OUTMAX_YL 0x804A
#define GT9XX_OUTMAX_YH 0x804B
#define GT9XX_TOUCHNUM 0x804C
#define GT9XX_MODSW1 0x804D
#define GT9XX_MODSW2 0x804E
#define GT9XX_SHAKECNT 0x804F

#define GT9XX_PID1 0x8140
#define GT9XX_PID2 0x8141
#define GT9XX_PID3 0x8142
#define GT9XX_PID4 0x8143
#define GT9XX_FWVL 0x8144
#define GT9XX_FWVH 0x8145
#define GT9XX_CORDXL 0x8146
#define GT9XX_CORDXH 0x8147
#define GT9XX_CORDYL 0x8148
#define GT9XX_CORDYH 0x8149
#define GT9XX_VID   0x814A

#define GT9XX_STATUS 0x814E

#define GT9XX_TRACKID1 0x814F
#define GT9XX_P1XL 0x8150 
#define GT9XX_P1XH 0x8151 
#define GT9XX_P1YL 0x8152 
#define GT9XX_P1YH 0x8153
#define GT9XX_P1SIZEL 0x8154 
#define GT9XX_P1SIZEH 0x8155
// Reserve 0x8156
#define GT9XX_TRACKID2 0x8157
#define GT9XX_P2XL 0x8158 
#define GT9XX_P2XH 0x8159 
#define GT9XX_P2YL 0x815A 
#define GT9XX_P2YH 0x815B
#define GT9XX_P2SIZEL 0x815C 
#define GT9XX_P2SIZEH 0x815D 
// Reserve 0x815E
#define GT9XX_TRACKID3 0x815F
#define GT9XX_P3XL 0x8160 
#define GT9XX_P3XH 0x8161 
#define GT9XX_P3YL 0x8162 
#define GT9XX_P3YH 0x8163
#define GT9XX_P3SIZEL 0x8164 
#define GT9XX_P3SIZEH 0x8165 
// Reserve 0x81566
#define GT9XX_TRACKID4 0x8167
#define GT9XX_P4XL 0x8168 
#define GT9XX_P4XH 0x8169 
#define GT9XX_P4YL 0x816A 
#define GT9XX_P4YH 0x816B
#define GT9XX_P4SIZEL 0x816C 
#define GT9XX_P4SIZEH 0x816D 
// Reserve 0x816E
#define GT9XX_TRACKID5 0x816F
#define GT9XX_P5XL 0x8170 
#define GT9XX_P5XH 0x8171 
#define GT9XX_P5YL 0x8172 
#define GT9XX_P5YH 0x8173
#define GT9XX_P5SIZEL 0x8174 
#define GT9XX_P5SIZEH 0x8175 

void touch_init(void);
void touch_read(uint8_t num);
uint8_t touch_is_pressed(void);

#endif