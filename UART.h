#if !defined(_UART_H_)
#define _UART_H_




// Константы для регистра UCSRA
//========================================================================

#define UCSRA_RXC	(1 << RXC0) //USART Receive Complete
#define UCSRA_TXC	(1 << TXC0) //USART Transmit Complete
#define UCSRA_UDRE	(1 << UDRE0) //UDREn - USART Data Register Empty
#define	UCSRA_FE	(1 << FE0) //FEn - Frame Error
#define UCSRA_DOR	(1 << DOR0) //DORn - Data OverRun
#define UCSRA_UPE	(1 << UPE0) //UPEn - USART Parity Error
#define UCSRA_U2X	(1 << U2X0)	//U2Xn - Double the USART Transmission Speed
#define UCSRA_MPCM	(1 << MPCM0) //MPCMn - Multi-processor Communication Mode




// Константы для регистра UCSRB
//========================================================================

#define UCSRB_RXCIE	(1 << RXCIE0) //RXCIEn - RX Complete Interrupt Enable
#define UCSRB_TXCIE	(1 << TXCIE0) //TXCIEn - TX Complete Interrupt Enable
#define UCSRB_UDRIE	(1 << UDRIE0) //UDRIENn - USART Data Register Empty Interrupt Enable
#define UCSRB_RXEN	(1 << RXEN0) //RXENn - Receiver Enable
#define UCSRB_TXEN	(1 << TXEN0) //TXENn - Transmitter Enable
#define UCSRB_UCSZ2	(1 << UCSZ02) //UCSZn2 - Character Size
#define UCSRB_RXB8	(1 << RXB80) //RXB8n - Receive Data Bit 8
#define UCSRB_TXB8	(1 << TXB80) //TXB8n - Transmit Data Bit 8

#define UCSRB_9BIT		(1 << UCSZ02)
#define UCSRB_5_8BIT	(0 << UCSZ02)




// Константы для регистра UCSRC
//========================================================================

#define UCSRC_UMSEL1	(1 << UMSEL01) //UMSELn1:0 - USART Mode Select
#define UCSRC_UMSEL0	(1 << UMSEL00) //UMSELn1:0 - USART Mode Select
#define UCSRC_UPM1		(1 << UPM01) //UPMn1:0 - Parity Mode
#define UCSRC_UPM0		(1 << UPM00) //UPMn1:0 - Parity Mode
#define UCSRC_USBS		(1 << USBS0) //USBSn - Stop Bit Select
#define UCSRC_UCSZ1		(1 << UCSZ01) //UCSZn1:0 - Character Size
#define UCSRC_UCSZ0		(1 << UCSZ00) //UCSZn1:0 - Character Size

#define UCSRC_ASYNC	((0 << UMSEL01) | (0 << UMSEL00)) //Asynchronous USART
#define UCSRC_SYNC	((0 << UMSEL01) | (1 << UMSEL00)) //Synchronous USART
#define UCSRC_MSPIM	((1 << UMSEL01) | (1 << UMSEL00)) //Master SPI (MSPIM)

#define UCSRC_NOPAR	((0 << UPM01) | (0 << UPM00)) //Parity Disabled
#define UCSRC_EVEN	((1 << UPM01) | (0 << UPM00)) //Even Parity
#define UCSRC_ODD	((1 << UPM01) | (1 << UPM00)) //Odd Parity

#define UCSRC_1STOP	(0 << USBS0) //1-bit
#define UCSRC_2STOP	(1 << USBS0) //2-bit

#define UCSRC_5BIT	((0 << UCSZ01) | (0 << UCSZ00)) // 5-bit
#define UCSRC_6BIT	((0 << UCSZ01) | (1 << UCSZ00)) // 6-bit
#define UCSRC_7BIT	((1 << UCSZ01) | (0 << UCSZ00)) // 7-bit
#define UCSRC_8BIT	((1 << UCSZ01) | (1 << UCSZ00)) // 8-bit
#define UCSRC_9BIT	((1 << UCSZ01) | (1 << UCSZ00)) // 9-bit




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Инициализация UART
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern void UART0_Init(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Чтение байта из приемного буфера
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern uint8_t UART0_ReadRx(char *Data);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Возвращает количество принятых байтов
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern uint8_t UART0_CntRx(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Посылает символ в UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern uint8_t UART0_SendChar(char Data);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Посылает строку в UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern uint8_t UART0_SendStr(char *Str);




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Выводит строку, расположенную во flash в UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Пример 1:
// PT_WAIT_UNTIL(pt, UART0_SendStr_P(PSTR("Text")));
//
// Пример 2:
// const char Msg[] PROGMEM = "Text";
// PT_WAIT_UNTIL(pt, UART0_SendStr_P(Msg));
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern uint8_t UART0_SendStr_P(const char *Str);

#endif
