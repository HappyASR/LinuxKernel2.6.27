/*
       rx reg0: command
       rx reg1: rx size1
       rx reg2: rx size2
       
       tx reg0  status
       tx reg1  tx size1
       tx reg2  tx size2
       tx reg3  count   
*/
#include <asm/types.h>

#define RCMD 0x01
#define WCMD 0x02

#define STS_BUSY 0x0
#define STS_READY 0x01
#define STS_RDEN 0x02

#define LBIT (0x1<<4)

#define MAXMAIL 5

struct spi_mailbox {
       u32 char_len;
       u32 rx_tri_lv;
       u32 tx_tri_lv;
       u32 cpol;
       u32 cpha;
       u32 xsb;
       u32 rx_size;
       u32 rx_cnt;
       u32 tx_size;
       u32 tx_cnt;
       u32 mail_cmd;
       u32 mail_cnt;
       u8 *mail_storage[MAXMAIL];
       u32 mail_rx_id;
       u32 mail_tx_id;
       u32 mail_state;
};

