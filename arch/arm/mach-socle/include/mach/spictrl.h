// SPI register address define

#include <mach/platform.h>

#define SPI_TxR             IO_ADDRESS(SOCLE_APB0_SPI1 + 0x0)	// SPI transmit data regisgetr (Write only)
#define SPI_RxR             IO_ADDRESS(SOCLE_APB0_SPI1 + 0x0)	// SPI receive data register (Read only)
#define SPI_IER             IO_ADDRESS(SOCLE_APB0_SPI1 + 0x4)	// SPI interrupt enable control register (R/W)
#define SPI_FCR             IO_ADDRESS(SOCLE_APB0_SPI1 + 0x8)	// SPI FIFO control regisgter (R/W)
#define SPI_FWCR            IO_ADDRESS(SOCLE_APB0_SPI1 + 0xc)	// SPI transaction flow control register (R/W)
#define SPI_DLYCR           IO_ADDRESS(SOCLE_APB0_SPI1 + 0x10)	// SPI delay control register (R/W)
#define SPI_TxCR            IO_ADDRESS(SOCLE_APB0_SPI1 + 0x14)	// SPI transmit counter register (R/W)
#define SPI_RxCR            IO_ADDRESS(SOCLE_APB0_SPI1 + 0x18)	// SPI receive counter register (R/W)
#define SPI_SSCR            IO_ADDRESS(SOCLE_APB0_SPI1 + 0x1c)	// SPI slave select & characteristic register (R/W)
#define SPI_ISR             IO_ADDRESS(SOCLE_APB0_SPI1 + 0x20)	// SPI interrupt status register (R/W)
#define PHY_SPI_RxR			SPI_RxR 

// Transcation flow control
#define	LOOPBACK_ENABLE			0x0001		// Enable internal loopback mode
#define	BIDIRECTION_ENABLE		0x0002		// Enable bidirection mode
#define	LSB_FIRST_EN			0x0004		// Enable LSB first mode
#define	CLOCK_PHASE				0x0008		// Select clock phase
#define	CLOCK_POLARITY			0x0010		// Select clock polarity
#define	CONCURRENT_ENABLE		0x0020		// Tx & Rx concurrent mode enable
#define	CLOCK_IDLE_ASSERT		0x0100		// SPI clock idle enable control
#define	SPI_RUN					0x0200		// SPI transmit & receive control, set this bit will start SPI controller
#define	MASETR_ENABLE			0x0400		// Enable master mode
#define	RESET_CONTROL			0x0800		// Reset SPI controller

#define SPI_LOOPBACK_EN()       (writel(readl(SPI_FWCR) | LOOPBACK_ENABLE ,    SPI_FWCR))
#define SPI_LOOPBACK_DIS()      (writel(readl(SPI_FWCR) & ~LOOPBACK_ENABLE,    SPI_FWCR))
#define SPI_BIDIRECTION_EN()    (writel(readl(SPI_FWCR) | BIDIRECTION_ENABLE , SPI_FWCR))
#define SPI_BIDIRECTION_DIS()   (writel(readl(SPI_FWCR) & ~BIDIRECTION_ENABLE, SPI_FWCR))
#define SPI_LSBFIRST_EN()       (writel(readl(SPI_FWCR) | LSB_FIRST_EN ,       SPI_FWCR))
#define SPI_MSBFIRST_EN()       (writel(readl(SPI_FWCR) & ~LSB_FIRST_EN,       SPI_FWCR))
#define SPI_SECOND_PHASE()      (writel(readl(SPI_FWCR) | CLOCK_PHASE ,        SPI_FWCR))
#define SPI_FIRST_PHASE()       (writel(readl(SPI_FWCR) & ~CLOCK_PHASE,        SPI_FWCR))
#define SPI_LOW_IDLE()          (writel(readl(SPI_FWCR) | CLOCK_POLARITY ,     SPI_FWCR))
#define SPI_HIGH_IDLE()         (writel(readl(SPI_FWCR) & ~CLOCK_POLARITY,     SPI_FWCR))
#define SPI_CONCURRENT_EN()     (writel(readl(SPI_FWCR) | CONCURRENT_ENABLE ,  SPI_FWCR))
#define SPI_CONCURRENT_DIS()    (writel(readl(SPI_FWCR) & ~CONCURRENT_ENABLE,  SPI_FWCR))
#define SPI_CLOCK_IDLE_EN()     (writel(readl(SPI_FWCR) | CLOCK_IDLE_ASSERT ,  SPI_FWCR))
#define SPI_CLOCK_IDLE_DIS()    (writel(readl(SPI_FWCR) & ~CLOCK_IDLE_ASSERT,  SPI_FWCR))
#define SPI_START_TRANSFER()    (writel(readl(SPI_FWCR) | SPI_RUN ,            SPI_FWCR))
#define SPI_MASETR_EN()         (writel(readl(SPI_FWCR) | MASETR_ENABLE ,      SPI_FWCR))
#define SPI_MASETR_DIS()        (writel(readl(SPI_FWCR) & ~MASETR_ENABLE,      SPI_FWCR))
#define SPI_RESET()             (writel(readl(SPI_FWCR) | RESET_CONTROL,       SPI_FWCR))
#define IS_SPI_RUNNING()        (readl(SPI_FWCR) & SPI_RUN)

// FIFO control
#define SPI_FIFO_SIZE           8

#define SPI_TX_THRESH_2         (0<<8)
#define SPI_TX_THRESH_4         (1<<8)
#define SPI_TX_THRESH_6         (2<<8)
#define SPI_RX_THRESH_2         (0<<11)
#define SPI_RX_THRESH_4         (1<<11)
#define SPI_RX_THRESH_6         (2<<11)

#define	RECEIVE_DATA_AVAILABLE  0x0001		// when this bit is set, there at last one data in receive FIFO
#define	TRANSMIT_FIFO_FULL      0x0002		// when this bit is set, there transmit FIFO is full
#define	CLEAR_TRANSMIT          0x0004		// clear transmit FIFO
#define	CLEAR_RECEIVE           0x0008		// clear receive FIFO
#define	TRANSMIT_LEVEL_MASK     0x0700//0x0070		// transimt FIFO level mask
#define	RECEIVE_LEVEL_MASK      0x3800//0x0380		// receive FIFO level mask

#define IS_RX_DATA_AVAILABLE()  (readl(SPI_FCR) & RECEIVE_DATA_AVAILABLE)
#define IS_TX_FIFO_FULL()       (readl(SPI_FCR) & TRANSMIT_FIFO_FULL)
#define SPI_CLEAR_TX_FIFO()     (writel(readl(SPI_FCR) | CLEAR_TRANSMIT, SPI_FCR))
#define SPI_CLEAR_RX_FIFO()     (writel(readl(SPI_FCR) | CLEAR_RECEIVE,  SPI_FCR))
#define SPI_TX_FIFO_THRESH_2()  (writel((readl(SPI_FCR) & ~TRANSMIT_LEVEL_MASK) | SPI_TX_THRESH_2, SPI_FCR))
#define SPI_TX_FIFO_THRESH_4()  (writel((readl(SPI_FCR) & ~TRANSMIT_LEVEL_MASK) | SPI_TX_THRESH_4, SPI_FCR))
#define SPI_TX_FIFO_THRESH_6()  (writel((readl(SPI_FCR) & ~TRANSMIT_LEVEL_MASK) | SPI_TX_THRESH_6, SPI_FCR))
#define SPI_RX_FIFO_THRESH_2()  (writel((readl(SPI_FCR) & ~RECEIVE_LEVEL_MASK)  | SPI_RX_THRESH_2, SPI_FCR))
#define SPI_RX_FIFO_THRESH_4()  (writel((readl(SPI_FCR) & ~RECEIVE_LEVEL_MASK)  | SPI_RX_THRESH_4, SPI_FCR))
#define SPI_RX_FIFO_THRESH_6()  (writel((readl(SPI_FCR) & ~RECEIVE_LEVEL_MASK)  | SPI_RX_THRESH_6, SPI_FCR))

// Interrupt enable control
#define	TRANSFER_COMPLETE       0x0001		// transfer complete interrupt control
#define	RECEIVE_FIFO_OVERRUN    0x0002		// receive FIFO overrun interrupt control
#define	RECEIVE_DATA            0x0004		// receive FIFO level reach interrupt control
#define	TRANSMIT_DATA           0x0008		// transmit FIFO level reach interrupt control

#define	IE_RXCP                 0x0001		// transfer complete interrupt control
#define	IE_OR                   0x0002		// receive FIFO overrun interrupt control
#define	IE_RX                   0x0004		// receive FIFO level reach interrupt control
#define	IE_TX                   0x0008		// transmit FIFO level reach interrupt control

#define SPI_COMPLETE_INT_EN()   (writel(readl(SPI_IER) | TRANSFER_COMPLETE ,    SPI_IER))
#define SPI_COMPLETE_INT_DIS()  (writel(readl(SPI_IER) & ~TRANSFER_COMPLETE,    SPI_IER))
#define SPI_OVERRUN_INT_EN()    (writel(readl(SPI_IER) | RECEIVE_FIFO_OVERRUN , SPI_IER))
#define SPI_OVERRUN_INT_DIS()   (writel(readl(SPI_IER) & ~RECEIVE_FIFO_OVERRUN, SPI_IER))
#define SPI_RECEIVE_INT_EN()    (writel(readl(SPI_IER) | RECEIVE_DATA ,         SPI_IER))
#define SPI_RECEIVE_INT_DIS()   (writel(readl(SPI_IER) & ~RECEIVE_DATA,         SPI_IER))
#define SPI_TRANSMIT_INT_EN()   (writel(readl(SPI_IER) | TRANSMIT_DATA ,        SPI_IER))
#define SPI_TRANSMIT_INT_DIS()  (writel(readl(SPI_IER) & ~TRANSMIT_DATA,        SPI_IER))
#define SPI_INT_ENABLE(x)       (writel(readl(SPI_IER) | (x) , SPI_IER))
#define SPI_INT_DISABLE(x)      (writel(readl(SPI_IER) & ~(x), SPI_IER))

// Clock delay control
#define	ACTIVE_DELAY_MASK       0x0007      // Period before SPI clock active (PBCA).
#define	TRANSFER_DELAY_MASK     0x0031      // Period between two consecutive transfer (PBCT).
#define	TxRx_DELAY_MASK         0x0700      // Period between Tx and Rx transfer (PBTxRx).

#define	CLOCK_HALF              0x00        // 1/2 clock
#define	CLOCK_ZERO              0x00        // zero clock
#define	CLOCK_4                 0x01        // 4 clock
#define	CLOCK_8                 0x02        // 8 clock
#define	CLOCK_16                0x03        // 16 clock
#define	CLOCK_32                0x04        // 32 clock
#define	CLOCK_64                0x05        // 64 clock
#define	CLOCK_128               0x06        // 128 clock
#define	CLOCK_256               0x07        // 256 clock

#define ACTIVE_DELAY_HALF()     (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_HALF, SPI_DLYCR))
#define ACTIVE_DELAY_4()        (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_4,    SPI_DLYCR))
#define ACTIVE_DELAY_8()        (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_8,    SPI_DLYCR))
#define ACTIVE_DELAY_16()       (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_16,   SPI_DLYCR))
#define ACTIVE_DELAY_32()       (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_32,   SPI_DLYCR))
#define ACTIVE_DELAY_64()       (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_64,   SPI_DLYCR))
#define ACTIVE_DELAY_128()      (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_128,  SPI_DLYCR))
#define ACTIVE_DELAY_256()      (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | CLOCK_256,  SPI_DLYCR))
#define SET_ACTIVE_DELAY(x)     (writel((readl(SPI_DLYCR) & ~ACTIVE_DELAY_MASK) | (x),        SPI_DLYCR))

#define TRANSFER_DELAY_ZERO()   (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_ZERO, SPI_DLYCR))
#define TRANSFER_DELAY_4()      (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_4,    SPI_DLYCR))
#define TRANSFER_DELAY_8()      (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_8,    SPI_DLYCR))
#define TRANSFER_DELAY_16()     (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_16,   SPI_DLYCR))
#define TRANSFER_DELAY_32()     (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_32,   SPI_DLYCR))
#define TRANSFER_DELAY_64()     (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_64,   SPI_DLYCR))
#define TRANSFER_DELAY_128()    (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_128,  SPI_DLYCR))
#define TRANSFER_DELAY_256()    (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | CLOCK_256,  SPI_DLYCR))
#define SET_TRANSFER_DELAY(x)   (writel((readl(SPI_DLYCR) & ~TRANSFER_DELAY_MASK) | (x),        SPI_DLYCR))

#define TxRx_DELAY_ZERO()       (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_ZERO, SPI_DLYCR))
#define TxRx_DELAY_4()          (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_4,    SPI_DLYCR))
#define TxRx_DELAY_8()          (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_8,    SPI_DLYCR))
#define TxRx_DELAY_16()         (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_16,   SPI_DLYCR))
#define TxRx_DELAY_32()         (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_32,   SPI_DLYCR))
#define TxRx_DELAY_64()         (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_64,   SPI_DLYCR))
#define TxRx_DELAY_128()        (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_128,  SPI_DLYCR))
#define TxRx_DELAY_256()        (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | CLOCK_256,  SPI_DLYCR))
#define SET_TxRx_DELAY(x)       (writel((readl(SPI_DLYCR) & ~TxRx_DELAY_MASK) | (x),        SPI_DLYCR))

// slave select & characteristic control
#define CLOCK_DIVISOR_MASK      0x001f
#define SLAVE_SELECT_MASK       0x0700
#define CHARACTER_LENGTH_MASK   0x7800

#define	LENGTH_4BITS            0x03
#define	LENGTH_5BITS            0x04
#define	LENGTH_6BITS            0x05
#define	LENGTH_7BITS            0x06
#define	LENGTH_8BITS            0x07
#define	LENGTH_9BITS            0x08
#define	LENGTH_10BITS           0x09
#define	LENGTH_11BITS           0x0a
#define	LENGTH_12BITS           0x0b
#define	LENGTH_13BITS           0x0c
#define	LENGTH_14BITS           0x0d
#define	LENGTH_15BITS           0x0e
#define	LENGTH_16BITS           0x0f

#define SET_CLOCK_DIVISOR(x)    (writel((readl(SPI_SSCR) & ~CLOCK_DIVISOR_MASK) | x,         SPI_SSCR))
#define SPI_SELECT_SLAVE(x)     (writel((readl(SPI_SSCR) & ~SLAVE_SELECT_MASK)  | (x << 8),  SPI_SSCR))
#define SET_BIT_LENGTH(x)       (writel((readl(SPI_SSCR) & ~CHARACTER_LENGTH_MASK)  | (x << 11), SPI_SSCR))

// interrupt status
#define TRANSFER_COMPLETE_INT	0x01    // transfer complete interrupt
#define OVERRUN_INT             0x02    // overrun interrupt
#define RECEIVE_FIFO_INT        0x04    // receive FIFO level reach interrupt
#define TRANSMIT_FIFO_INT       0x08    // transmit FIFO level reach interrupt

#define IS_TRANSFER_COMPLETE()  (readl(SPI_ISR) & TRANSFER_COMPLETE_INT)
#define IS_OVERRUN()            (readl(SPI_ISR) & OVERRUN_INT)
#define IS_RECEVIE_FIFO()       (readl(SPI_ISR) & RECEIVE_FIFO_INT)
#define IS_TRANSMIT_FIFO()      (readl(SPI_ISR) & TRANSMIT_FIFO_INT)
#define CHECK_RECEVIE_COMPLETE(x)   ((x) & TRANSFER_COMPLETE_INT)
#define CHECK_OVERRUN(x)            ((x) & OVERRUN_INT)
#define CHECK_RECEVIE_FIFO(x)       ((x) & RECEIVE_FIFO_INT)
#define CHECK_TRANSMIT_FIFO(x)      ((x) & TRANSMIT_FIFO_INT)

// data read & write control
#define SPI_WRITE_DATA(x)       (writel((x), SPI_TxR))
#define SPI_READ_DATA()         (readl(SPI_RxR))

// Transmit & Receive data count setting
#define SET_TRANSMIT_COUNT(x)   (writel((x), SPI_TxCR))
#define SET_RECEIVE_COUNT(x)    (writel((x), SPI_RxCR))
#define GET_TRANSMIT_COUNT(x)   (readl(SPI_TxCR))
#define GET_RECEIVE_COUNT(x)    (readl(SPI_RxCR))

// define
#define	MSB_FIRST				0
#define	LSB_FIRST				1

#define	SPI_SLAVE				0
#define	SPI_MASTER				1

#define	UNIDIRECTION			0
#define	BIDIRECTION				1

#define	FIRST_SCLK_EDGE			0
#define	SECOND_SCLK_EDGE		1

#define	LOW_IDLE				0
#define	HIGH_IDLE				1

#define	CLOCK_NOT_ASSERT		0
#define	CLOCK_ASSERT			1

#define	TX_RX_NO_CONCURRENT		0
#define	TX_RX_CONCURRENT		1


