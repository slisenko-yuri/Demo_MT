/*
 * UART.c
 */ 


/*
Asynchronous Normal mode (U2Xn = 0):
	BAUD = F_CPU / (16 * (UBRRn + 1))
	UBRRn = F_CPU / (16 * BAUD) - 1
	
Asynchronous Double Speed mode (U2Xn = 1):
	BAUD = F_CPU / (8 * (UBRRn + 1))
	UBRRn = F_CPU / (8 * BAUD) - 1
	
UBRRn = 0...4095	

Frame Formats:
	- 1 start bit
	- 5,6,7,8,9 data bits
	- no, even or odd parity bit
	- 1 or 2 stop bits
*/

/*
UCSRnA:
-------------------------------------------------------------
7		6		5		4		3		2		1		0
-------------------------------------------------------------
RXCn	TXCn	UDREn	FEn		DORn	UPEn	U2Xn	MPCMn
-------------------------------------------------------------
0R		0RW		1R		0R		0R		0R		0RW		0RW
-------------------------------------------------------------
RXCn - USART Receive Complete
TXCn - USART Transmit Complete
UDREn - USART Data Register Empty
FEn - Frame Error
DORn - Data OverRun
UPEn - USART Parity Error
U2Xn - Double the USART Transmission Speed
MPCMn - Multi-processor Communication Mode

UCSRnB:
-------------------------------------------------------------
7		6		5		4		3		2		1		0
-------------------------------------------------------------
RXCIEn	TXCIEn	UDRIEn	RXENn	TXENn	UCSZn2	RXB8n	TXB8n
-------------------------------------------------------------
0RW		0RW		0RW		0RW		0RW		0RW		0R		0RW
-------------------------------------------------------------
RXCIEn - RX Complete Interrupt Enable
TXCIEn - TX Complete Interrupt Enable
UDRIENn - USART Data Register Empty Interrupt Enable
RXENn - Receiver Enable
TXENn - Transmitter Enable
UCSZn2 - Character Size (������ ������� UCSRnC)
RXB8n - Receive Data Bit 8
TXB8n - Transmit Data Bit 8

UCSRnC:
--------------------------------------------------------------
7		6		5		4		3		2		1		0
--------------------------------------------------------------
UMSELn1	UMSELn0	UPMn1	UPMn0	USBSn	UCSZn1	UCSZn0	UCPOLn
--------------------------------------------------------------
0RW		0RW		0RW		0RW		0RW		1RW		1RW		0RW
--------------------------------------------------------------

UMSELn1:0 - USART Mode Select:
------------------------------
00 - Asynchronous USART
01 - Synchronous USART
10 - Reserved
11 - Master SPI (MSPIM)

UPMn1:0 - Parity Mode:
------------------------------
00 - Disabled
01 - Reserved
10 - Enabled, Even Parity
11 - Enabled, Odd Parity

USBSn - Stop Bit Select:
------------------------------
0 -	1-bit
1 - 2-bit

UCSZn1:0 - Character Size:
------------------------------
(UCSZn2=0)	(UCSZn2=1)
00 - 5-bit	00 - Reserved
01 - 6-bit	01 - Reserved
10 - 7-bit	10 - Reserved
11 - 8-bit	11 - 9-bit
*/

#include "Config.h"
#include "UART.h"
#include "Mt.h"




//////////////////////////////////////////////////////////////////////////
// ��������� ������
//////////////////////////////////////////////////////////////////////////
#define UART0_SIZE_RX	8		// ������ ��������� ������
#define UART0_SIZE_TX	40		// ������ ������ ��� ��������
#define UART0_RDY_TX	0x01	// ����� ���������� ������ ��������
//////////////////////////////////////////////////////////////////////////




//------------------------------------------------------------------------
// ���������� ������
//------------------------------------------------------------------------

static uint8_t UART0_Status = 0; // ������� ��������� UART0

//������� 1-�� ����� � �������� ������ (� �������� ����� ������ ��������
//�����)
static uint8_t UART0_BeginRx = 0;

//������� � �������� ������, ���� ����� ������� ��������� �������� ����
static uint8_t UART0_EndRx = 0;

// �-�� �������� ������ � ������
static uint8_t UART0_CountRx = 0;

static uint8_t UART0_BufRx[UART0_SIZE_RX]; // �������� �����

//������� 1-�� ����� � ������ �������� (� �������� ���������� ����������)
static uint8_t UART0_BeginTx = 0;

//������� � ������ ��������, ���� ����� ������� ��������� ������������
//����
static uint8_t UART0_EndTx = 0;
	
//�-�� ������ � ������ ��� ��������
static uint8_t UART0_CountTx = 0;
	
// ����� ��� ��������
static uint8_t UART0_BufTx[UART0_SIZE_TX];




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ������������� UART
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void UART0_Init(void)
{
	// ����������� UART0

	//��������� UBRRH_VALUE � UBRRL_VALUE ����������� �������������� ��
	//��������� �������� F_CPU � BAUD, ����������� � ����� Config.h
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	//USE_2X ������������ �������������� �� ��������� �������� F_CPU �
	//BAUD, ����������� � ����� Config.h
	#if USE_2X
	UCSR0A = (1 << TXC0) | (1 << U2X0) | (0 << MPCM0);
	#warning "UART: U2X0 = 1"
	#else
	UCSR0A = (1 << TXC0) | (0 << U2X0) | (0 << MPCM0);
	#warning "UART: U2X0 = 0"
	#endif

	//������������� ���� ���������� ������ � ��������, � ����� ����
	//���������� ���������� �� ������ � ��������
	UCSR0B = (1 << RXCIE0) | (0 << TXCIE0) | (1 << UDRIE0)
		| (1 << RXEN0) | (1 << TXEN0) | UCSRB_5_8BIT
		| (0 << TXB80);

	// ����������� �����, ��� �������� ������ ��������, 1 ����-���,
	// ������� 8-���.
	UCSR0C = UCSRC_ASYNC | UCSRC_NOPAR | UCSRC_1STOP | UCSRC_8BIT;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//------------------------------------------------------------------------
// ���������� ���� � �������� ����� UART.
// ���������� � ����������� ���������� �� ������ �����.
// ���������� 1 � ������ ������, ����� ���������� 0.
//------------------------------------------------------------------------
static uint8_t WrRx(char Data)
{
	//���� ���������� ������ � ������ ����� ������� ������, �� ����������
	//0 (� ������ ��� �����)
	if (UART0_CountRx >= UART0_SIZE_RX) return 0;
	
	//���������� ���� � �������� ����� � �������������� ������� �����
	//������ UART_EndRx
	UART0_BufRx[UART0_EndRx++] = Data;

	// ���� UART_EndRx ����� ������� ������, �� ������������� ��� � 0
	if (UART0_EndRx >= UART0_SIZE_RX) UART0_EndRx = 0;

	// �������������� ������� ������ ������
	UART0_CountRx++;
	
	return 1; // ���� ������� ������� � �����
}
//------------------------------------------------------------------------




//------------------------------------------------------------------------
// ���������� ���������� �� ������ �� UART
//------------------------------------------------------------------------
#if defined(__AVR_ATmega164A__) || defined(__AVR_ATmega164P__)\
	|| defined(__AVR_ATmega164PA__)\
	|| defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324P__)\
	|| defined(__AVR_ATmega324PA__)\
	|| defined(__AVR_ATmega640__)\
	|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__)\
	|| defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)\
	|| defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__)\
	|| defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)\
	|| defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(USART0_RX_vect)
#else
ISR(USART_RX_vect)
#endif
{
	WrRx(UDR0); // ��������� �������� ���� � ������ ������

	// ���������� ������, ������� ���� ������������ ������� ��������
	MT_DrvTaskRunIrq(DRV_UART_RX);
}
//------------------------------------------------------------------------




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ������ ����� �� ��������� ������
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uint8_t UART0_ReadRx(char *Data)
{
	if (UART0_CountRx == 0)
	{
		//� ������ ������ ����������� ������

		//������� ������� ������ � ��������� ��������
		MT_DrvTaskWait(DRV_UART_RX);

		return 0;
	}

	*Data = UART0_BufRx[UART0_BeginRx++];
	if (UART0_BeginRx >= UART0_SIZE_RX) UART0_BeginRx = 0;

	ENTER_CRITICAL();
	UART0_CountRx--; // ��������� �������� �������� ������
	EXIT_CRITICAL();

	return 1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ���������� ���������� �������� ������
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uint8_t UART0_CntRx(void)
{
	return UART0_CountRx;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//------------------------------------------------------------------------
// ���������� ���������� �� �������� ����� � UART
//------------------------------------------------------------------------
#if defined(__AVR_ATmega164A__) || defined(__AVR_ATmega164P__)\
	|| defined(__AVR_ATmega164PA__)\
	|| defined(__AVR_ATmega324A__) || defined(__AVR_ATmega324P__)\
	|| defined(__AVR_ATmega324PA__)\
	|| defined(__AVR_ATmega640__)\
	|| defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__)\
	|| defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)\
	|| defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__)\
	|| defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)\
	|| defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
ISR(USART0_UDRE_vect)
#else
ISR(USART_UDRE_vect)
#endif
{
	if (UART0_CountTx == 0)
	{
		UART0_Status |= UART0_RDY_TX; // UART ����� � ��������
		UCSR0B &= ~(1 << UDRIE0); //��������� ���������� �� ��������
		
		// ���������� ������, ������� ���� ������������ �������� ��������
		MT_DrvTaskRunIrq(DRV_UART_TX);
	}
	else
	{
		//�������� ���� ������ �� ������ � ���������� � ������� ������
		//���������� ��������� ������� ������ � ���������� ������
		UDR0 = UART0_BufTx[UART0_BeginTx++];
		if (UART0_BeginTx >= UART0_SIZE_TX) UART0_BeginTx = 0;
		
		// ��������� �������� ������ ������������� ������
		UART0_CountTx--; 
	}
}
//------------------------------------------------------------------------




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// �������� ������ � UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uint8_t UART0_SendChar(char Data)
{
	uint8_t i;

	if (UART0_CountTx >= UART0_SIZE_TX)
	{
		//� ������ �������� ������������ ����� ��� ���������� ������

		//������� ������� ������ � ���������� ���������
		MT_DrvTaskWait(DRV_UART_TX);

		return 0;
	}

	// ���������� ���� � ���������� ����� � ��������������
	// ������� ����� ������ UART0_EndTx
	UART0_BufTx[UART0_EndTx++] = Data;
	
	// ���� UART0_EndTx ����� ������� ������, �� ������������� ��� � 0
	if (UART0_EndTx >= UART0_SIZE_TX) UART0_EndTx = 0;
	
	ENTER_CRITICAL();
	UART0_CountTx++; // �������������� ������� ������ ������
	i = UART0_Status;
	EXIT_CRITICAL();
	
	if (i & UART0_RDY_TX)
	{
		UART0_Status &= ~UART0_RDY_TX;
		UCSR0B |= 1 << UDRIE0; // ��������� ���������� �� ��������
	}

	return 1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// �������� ������ � UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint8_t UART0_SendStr(char *Str)
{
	uint8_t len, i;
	
	if (Str == NULL) return 1;

	len = strlen(Str);
	if (len == 0) return 1;

	if (len > (UART0_SIZE_TX - UART0_CountTx))
	{	// ���� � ������ ������������ ����� ��� ���������� ������

		//������� ������� ������ � ���������� ���������
		MT_DrvTaskWait(DRV_UART_TX);

		return 0;
	}
	
	i = UART0_SIZE_TX - UART0_EndTx; // ��������� ������ ����������
									// ������������ �� ����� ������
									// ��������

	if (len <= i)
	{	// ���� ��� ������ ���������� �� ����� ������
		memcpy(UART0_BufTx + UART0_EndTx, Str, len);
		UART0_EndTx += len;
		if (UART0_EndTx >= UART0_SIZE_TX) UART0_EndTx = 0;
	}
	else
	{	// ���� ����� ������ ���������� � ����� ������, � ����� � ������
		// ������
		memcpy(UART0_BufTx + UART0_EndTx, Str, i);
		memcpy(UART0_BufTx, Str + i, len - i);
		UART0_EndTx = len - i;
	}
	
	ENTER_CRITICAL();
	UART0_CountTx += len;
	i = UART0_Status;	
	EXIT_CRITICAL();
	
	if (i & UART0_RDY_TX) // ���� ���� ���������� �������� �� UART0
	{	
		UART0_Status &= ~UART0_RDY_TX;
		UCSR0B |= 1 << UDRIE0; // ��������� ���������� �� ��������		
	}	

	return 1;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ������� ������, ������������� �� flash � UART0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ������ 1:
// PT_WAIT_UNTIL(pt, UART0_SendStr_P(PSTR("Text")));
//
// ������ 2:
// const char Msg[] PROGMEM = "Text";
// PT_WAIT_UNTIL(pt, UART0_SendStr_P(Msg));
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint8_t UART0_SendStr_P(const char *Str)
{
	uint8_t len, i;
	
	if (Str == NULL) return 1;
	
	len = strlen_P(Str);
	if (len == 0) return 1;

	if (len > (UART0_SIZE_TX - UART0_CountTx))
	{	// ���� � ������ ������������ ����� ��� ���������� ������

		//������� ������� ������ � ���������� ���������
		MT_DrvTaskWait(DRV_UART_TX);

		return 0;
	}
	
	i = UART0_SIZE_TX - UART0_EndTx; // ��������� ������ ����������
									// ������������ �� ����� ������
									// ��������

	if (len <= i)
	{	// ���� ��� ������ ���������� �� ����� ������
		memcpy_P(UART0_BufTx + UART0_EndTx, Str, len);
		UART0_EndTx += len;
		if (UART0_EndTx >= UART0_SIZE_TX) UART0_EndTx = 0;
	}
	else
	{	// ���� ����� ������ ���������� � ����� ������, � ����� � ������
		// ������
		memcpy_P(UART0_BufTx + UART0_EndTx, Str, i);
		memcpy_P(UART0_BufTx, Str + i, len - i);
		UART0_EndTx = len - i;
	}
	
	ENTER_CRITICAL();
	UART0_CountTx += len;
	i = UART0_Status;
	EXIT_CRITICAL();
	
	if (i & UART0_RDY_TX) // ���� ���� ���������� �������� �� UART0
	{
		UART0_Status &= ~UART0_RDY_TX; // ������� ��� ���������� ��������
		UCSR0B |= 1 << UDRIE0; // ��������� ���������� �� ��������
	}

	return 1;
}

