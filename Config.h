#if !defined(_CONFIG_H_)
#define	_CONFIG_H_

//////////////////////////////////////////////////////////////////////////
// � ����� Config.h ��������� ���������� ����������, ������������ � ������
// ������� �������.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ������� ����������
//////////////////////////////////////////////////////////////////////////

#define MAIN_OSC	16000000UL // Arduino MEGA, Arduino UNO,Arduino NANO,
								// Arduino PRO MINI 5V
//#define MAIN_OSC	8000000UL // Arduino PRO MINI 3.3V ��� ����������
								// RC-��������� 8���
#define DIVIDER_OSC	1	// �������� �������
//#define DIVIDER_OSC	2
//#define DIVIDER_OSC	4
//#define DIVIDER_OSC	8
//#define DIVIDER_OSC	16
//#define DIVIDER_OSC	32
//#define DIVIDER_OSC	64
//#define DIVIDER_OSC	128

#define F_CPU (MAIN_OSC / DIVIDER_OSC) // ������� ����������
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �������� UART.
// �� ��������� BAUD ������������ ��������� ��������� UBRRH_VALUE,
// UBRRL_VALUE, USE_2X, ����������� ��� ������������� UART. ����������
// ��������� � ����� setbaud.h ��������� ���������� Atmel Studio.
//////////////////////////////////////////////////////////////////////////
//#define BAUD	115200
//#define BAUD	57600
//#define BAUD	38400
//#define BAUD	19200
#define BAUD	9600
//#define BAUD	4800
//#define BAUD	2400
//#define BAUD	1200
//#define BAUD 600
//#define BAUD 300
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ���������� �����, ��������� � ������� �������, ��������� � �����
// c:\Program Files\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.106\avr8-
// gnu-toolchain\avr\include
//////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>	// ������ � ���������� CLKPR, PRR
#include <string.h> // memcpy(), strcat(), strlen()
#include <stdio.h>

#define __PROG_TYPES_COMPAT__ // ��� ��������� ������ ����������
								//���� prog_char

#include <avr/pgmspace.h> // ��� ����� �� flash
#include <stdlib.h> // atoi(), abs()
#include <util/setbaud.h> // � ������� ����� ����� ��������������
						// ��������� UBRRH_VALUE � UBRRL_VALUE ��� UART

// ������������ ���������� GCC "Labels as Values". 
// ����� ����� ����� ����������� ����� ���������� ������ ��������� switch.
#define LC_INCLUDE "lc-addrlabels.h"

#include "pt\pt.h" // ���������� Protothreads (�����������)

#include "pin_macros.h" // ������� DRIVER(), ON(), OFF(), CPL(), TOGGLE(),
						// CLR(), SET(), ACTIVE(), LATCH()




#define TRUE	1
#define FALSE	0




//////////////////////////////////////////////////////////////////////////
// ��������� ��� ������ � ����������� Multitasking
//////////////////////////////////////////////////////////////////////////

// DRV_UART_TX � DRV_UART_RX ������������ ����� ID ��������� �� �������� �
// ������ ������ � ������� UART. ������� �������� ������ ���� ��������
// ���������� �����, ������� �� 0 � ����� �� �������.
// ��� ������� �������� � ���������� Multitasking ������������� ������.
#define DRV_UART_TX			0
#define DRV_UART_RX			1

// ID �������� ��� ������ � ���.
// ������� �������� ������ ���� �������� ���������� �����, ������� �� 0 �
// ����� �� �������. ��� ������� �������� � ���������� Multitasking
// ������������� ������.
#define MUTEX_ADC			0


// ID ���������.
// ������� �������� ������ ���� �������� ���������� �����, ������� �� 0 �
// ����� �� �������. ��� ������� �������� � ���������� Multitasking
// ������������� ������.

// ID �������� ��� ��������� �������� ������
#define TIMEOUT_DEBOUNCE	0	

// ID ��������, ������������� ��� ���������� �����������
#define TIMEOUT_HL			1




// ������������ ����� ����������������

#define HL		B,5,H	// ���. 17. ���� B, ��� 5, �������� ������� H
						// (���������)

#define SW		C,5,L	// ���. 28. ���� C, ��� 5, �������� ������� L
						// (������)




// ���������� ������� ��������� ������.
// ���������� �� ����������� ���������� ���������� �������.
extern void CalcSec(void);

#endif
