#if !defined(_ADC_H_)
#define _ADC_H_

#include "ADC_Cfg.h"

/*************************************************************************
-��� ������������� ��� ����������:
  PRR.PRADC = 0
  ADCSRA.ADEN = 1
 
-��� ������ 10-������� ���������� �������������� ������� ����� ������
 ADCL, ����� ADCH.
-������ �������������� �������� 25 ������ ���.
-����������� �������������� �������� 13 ������ ���.
-��������� �������� ADMUX ����� ����� ADATE ��� ADEN �������.
-����� ������� ����� Free Running mode ����� �� ������ ����� � �������
 ����������.
-����� ������������ �������� ���������� ������ �������������� ����� ����
 ��������, ������� ����� ������������ ��� ���������.
-����� ���������� �������������� ADCSRA.ADIF = 1.

��� ��������� ������������ �������� ��������� ��� ������ ������������� �
�������� 50...200���. ���� �� ��������� �������� 10 ���, �� ��� �����
������������� � �������� �������, ��� 200���
*************************************************************************/




// ������� ������������� � ������������� ���

//========================================================================
// 1. ��� ����������� 10-������ ��������� �� ������ 2 � ��������� 0...1.1V
//    ��� ���������� :
//------------------------------------------------------------------------
//	DIDR0 |= 1<<ADC2D;		// ���������� ��������� ������ �� ����� ���
//	ADMUX = ADMUX_REF_1_1 | ADMUX_DATA_RIGHT | ADMUX_MUX_2;
//					//ADMUX_REF_1_1 - ������� = 1.1V (���������� ��������)
//					//ADMUX_DATA_RIGHT - ������������ ���������� ������
//					//(���������� ��� ��������� 10-������� ����������)
//					// ADMUX_MUX_2 - ����� ��������� ADC2
//	ADCSRB = 0;
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_DIV;	
//					//ADCSRA_ADEN - ���������� ���
//					//ADCSRA_DIV - ���������, ������������ �������
//					//�������������� ��� (�������� ��� F_CPU, �������
//					//����������� �� ��������� ��������� ADC_FREQ_MAX
//					//��� ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;			//ADCSRA_ADSC - ����� ��������������
//	while (ADCSRA & ADCSRA_ADSC);	//�������� ���������� ��������������
//	uint16_t Result = ADC;			//������ ����������
//
//========================================================================

//========================================================================
// 2. ��� �������������� 8-������ ��������� �� ������ 3 � ���������
//    0...AREF � �������������� ����������:
//------------------------------------------------------------------------
//	DIDR0 |= 1<<ADC3D;		// ���������� ��������� ������ �� ����� ���
//	ADMUX = ADMUX_REF_AREF | ADMUX_DATA_LEFT | ADMUX_MUX_3;
//					//ADMUX_REF_AREF - ������� = AREF
//					//ADMUX_DATA_LEFT - ������������ ���������� �����
//					//     (���������� ��� ��������� 8-������� ����������)
//					//ADMUX_MUX_3 - ����� ��������� ADC3
//
//	ADCSRB = ADCSRB_SRC_FREE_RUN;							
//					//ADCSRB_SRC_FREE_RUN - ����� �����������
//					//�������������� ���������. ���������� �������������
//					//���� �� ������� �������������� ���������
//					// (ADCSRB_SRC_...), ���� � �������� ADCSRA ����������
//					//��� ADCSRA_ADATE
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_ADATE | ADCSRA_ADIF
//		| ADCSRA_ADIE | ADCSRA_DIV;
//					//ADCSRA_ADEN - ���������� ���,
//					//ADCSRA_ADATE - ���������� ��������������
//					//               ���������,
//					//ADCSRA_ADIF - ����� ����� ����������,
//					//ADCSRA_ADIE - ���������� ���������� ���
//					//ADCSRA_DIV - ���������, ������������ �������
//					//�������������� ��� (�������� ��� F_CPU, �������
//					//����������� �� ��������� ��������� ADC_FREQ_MAX ���
//					//ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;	// ����� ��������������
//
//	...
//	ISR(ADC_vect)				// ������� ��������� ���������� ���
//	{
//		uint8_t Result = ADCH;	// ���������� 8-������� ����������
//	}
//
//========================================================================

//========================================================================
// 3. ��� ����������� 8-������ ��������� � ��������� 0...AVCC �
//    �������������� ���������� � ���� ������� ��� (4 � 5):
//------------------------------------------------------------------------
//	DIDR0 |= (1<<ADC4D)|(1<<ADC5D);
//				//���������� �������� ������� �� ������ ���
//	ADMUX = ADMUX_REF_AVCC | ADMUX_DATA_LEFT | ADMUX_MUX_4;
//				//ADMUX_REF_AVCC - ������� = AVCC (���������� �������)
//				//ADMUX_DATA_LEFT - ������������ ���������� �����
//				//	(���������� ��� ��������� 8-������� ����������)
//				//ADMUX_MUX_4 - ����� ��������� ADC4
//
//	ADCSRB = 0;
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_ADIF | ADCSRA_ADIE | ADCSRA_DIV;
//				//ADCSRA_ADEN - ���������� ���,
//				//ADCSRA_ADIF - ����� ����� ����������,
//				//ADCSRA_ADIE - ���������� ���������� ���
//				//ADCSRA_DIV - ���������, ������������ ������� ��������������
//				//	��� (�������� ��� F_CPU, ������� ����������� ��
//				//	��������� ��������� ADC_FREQ_MAX ��� ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;						// ����� ��������������
//
//	...
//  // ������� ��������� ���������� ��� (����������� ��� ������� main)
//	ISR(ADC_vect)
//	{
//		uint8_t Result;
//		Result = ADCH;					// ���������� 8-������� ����������
//		if (Chanel == ADMUX_MUX_5)		// ����� ������ ���������
//			ADMUX = (ADMUX & ~ADMUX_MUX_MASK) | ADMUX_MUX_4;
//		else
//			ADMUX = (ADMUX & ~ADMUX_MUX_MASK) | ADMUX_MUX_5;
//		ADCSRA |= ADCSRA_ADSC;			// ����� ��������������
//	}
//========================================================================




// ��������� ��� �������� ADMUX
//========================================================================




// ��������� ��� ������ �������� ����������
//------------------------------------------------------------------------

//������� ���������� ������� � �������� AREF.
//���������� ������� �����������.
#define	ADMUX_REF_AREF	((0<<REFS1)|(0<<REFS0))
		
//������� ���������� ������� � �������� AVCC (�������).
#define ADMUX_REF_AVCC	((0<<REFS1)|(1<<REFS0))
	
//���������� ������� ���������� (1.1V ��� ������ ��������)
#define ADMUX_REF_1_1	((1<<REFS1)|(1<<REFS0))




// ��������� ��� ������ ������������ ����������
//------------------------------------------------------------------------

//����� ���������� ����� (8 �������� ADCH � 2 ������� ������� ADCL)
#define ADMUX_DATA_LEFT		(1<<ADLAR)
	
//������ ������������ (2 ������� ������� ADCH � 8 �������� ADCL)
#define ADMUX_DATA_RIGHT	(0<<ADLAR)




// ��������� ��� ������ ������ ���
//------------------------------------------------------------------------

// ����� 0
#define ADMUX_MUX_0	((0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0))

// ����� 1
#define ADMUX_MUX_1	((0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0))

// ����� 2
#define ADMUX_MUX_2	((0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0))

// ����� 3
#define ADMUX_MUX_3	((0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0))

// ����� 4
#define ADMUX_MUX_4	((0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0))

// ����� 5
#define ADMUX_MUX_5	((0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0))

// ����� 6
#define ADMUX_MUX_6	((0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0))

// ����� 7
#define ADMUX_MUX_7	((0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))

// ��������.
#define ADMUX_MUX_T	((1<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0))

// ���������� �������� �������� ���������� 1.1V
#define ADMUX_MUX_REF ((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0))

// 0V
#define ADMUX_MUX_GND ((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))

// ����� ��� ������� ���
#define ADMUX_MUX_MASK	((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))




// ��������� ��� �������� ADCSRA
//========================================================================




// ���������� ���
#define ADCSRA_ADEN				(1<<ADEN)

// ����� �������������� ���. ����� ���������� ��������������
// ������������ � 0. ����� ������������ ��� ��� ��������� ���.
#define ADCSRA_ADSC				(1<<ADSC) //ADc Start Conversion

//���������� ��������������� ������� �������������� �� �������
//(�������� ������� ������������ ������ ADTS2..ADTS0 � ��������
//ADCSRB)
#define ADCSRA_ADATE			(1<<ADATE) //ADc Auto Trigger Enable

//���� ���������� ���. ������������ ��������� � �����������
//���������� ��� ������� 1.
#define ADCSRA_ADIF				(1<<ADIF) //ADC Interrupt Flag

// ��� ���������� ���������� ���
#define ADCSRA_ADIE				(1<<ADIE) //ADC Interrupt Enable




// ��������� ��� �������� ������� ���
//------------------------------------------------------------------------

// ADC Prescaler Select
#define ADCSRA_DIV2		((0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)) // 1/2
#define ADCSRA_DIV4		((0<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)) // 1/4
#define ADCSRA_DIV8		((0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)) // 1/8
#define ADCSRA_DIV16	((1<<ADPS2)|(0<<ADPS1)|(0<<ADPS0)) // 1/16
#define ADCSRA_DIV32	((1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)) // 1/32
#define ADCSRA_DIV64	((1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)) // 1/64
#define ADCSRA_DIV128	((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)) // 1/128

//����� ��� �������� ������� ���
#define ADCSRA_DIV_MASK	((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))




// ��������� ��� �������� ADCSRB
//========================================================================




#define ADCSRB_ACME	(1<<ACME)	//Analog Comparator Multiplexer Enable
								//���� ��� ��������, �� ��� ACME ������
								//���� ����� ���.0




//��������� ��� ������ ��������� ������� ��� ��� ���������� ������
//����������� (��� ADATE=1 � �������� ADCSRA).
//������ ��� ������������ ������������� ������� ������� �������.
//------------------------------------------------------------------------

// ADC Auto Trigger Source

//Free Running mode
#define ADCSRB_SRC_FREE_RUN		((0<<ADTS2)|(0<<ADTS1)|(0<<ADTS0))
		
//Analog Comparator
#define ADCSRB_SRC_COMP			((0<<ADTS2)|(0<<ADTS1)|(1<<ADTS0))

//External Interrupt Request 0
#define ADCSRB_SRC_EINT0		((0<<ADTS2)|(1<<ADTS1)|(0<<ADTS0))

//Timer/Counter0 Compare Match A
#define ADCSRB_SRC_TIM0_COMPA	((0<<ADTS2)|(1<<ADTS1)|(1<<ADTS0))

//Timer/Counter0 Overflow
#define ADCSRB_SRC_TIM0_OVF		((1<<ADTS2)|(0<<ADTS1)|(0<<ADTS0))

//Timer/Counter1 Compare Match B
#define ADCSRB_SRC_TIM1_COMPB	((1<<ADTS2)|(0<<ADTS1)|(1<<ADTS0))

//Timer/Counter1 Overflow
#define ADCSRB_SRC_TIM1_OVF		((1<<ADTS2)|(1<<ADTS1)|(0<<ADTS0))

//Timer/Counter1 Capture Event
#define ADCSRB_SRC_TIM1_CAPTURE	((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0))
		
//����� ��� ��������� ������� ���
#define ADCSRB_SRC_MASK			((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0))




// ��������, ����� ���� ���������� ������ ���� �� ��������,
// �������� �������� ������� ���
#if defined(ADC_FREQ_MIN) && defined(ADC_FREQ_MAX)
#undef ADC_FREQ_MIN
#undef ADC_FREQ_MAX
#error "ADC: Must be defined ADC_FREQ_MIN only or ADC_FREQ_MAX only!"
#endif




#if defined(ADC_FREQ_MAX)

// ������������ �������� ������� ��� ��� ������� ��������������
// ADC_FREQ_MAX

#define DIVIDER (F_CPU / ADC_FREQ_MAX)

#if (DIVIDER <= 2)
#define ADCSRA_DIV	ADCSRA_DIV2
#define ADC_FREQ	(F_CPU / 2)
#warning "ADC: A frequency divider for the ADC = 2"

#elif (DIVIDER <= 4)
#define ADCSRA_DIV	ADCSRA_DIV4
#define ADC_FREQ	(F_CPU / 4)
#warning "ADC: A frequency divider for the ADC = 4"

#elif (DIVIDER <= 8)
#define ADCSRA_DIV	ADCSRA_DIV8
#define ADC_FREQ	(F_CPU / 8)
#warning "ADC: A frequency divider for the ADC = 8"

#elif (DIVIDER <= 16)
#define ADCSRA_DIV	ADCSRA_DIV16
#define ADC_FREQ	(F_CPU / 16)
#warning "ADC: A frequency divider for the ADC = 16"

#elif (DIVIDER <= 32)
#define ADCSRA_DIV	ADCSRA_DIV32
#define ADC_FREQ	(F_CPU / 32)
#warning "ADC: A frequency divider for the ADC = 32"

#elif (DIVIDER <= 64)
#define ADCSRA_DIV	ADCSRA_DIV64
#define ADC_FREQ	(F_CPU / 64)
#warning "ADC: A frequency divider for the ADC = 64"

#elif (DIVIDER <= 128)
#define ADCSRA_DIV	ADCSRA_DIV128
#define ADC_FREQ	(F_CPU / 128)
#warning "ADC: A frequency divider for the ADC = 128"

#else
// ���� �� ������ �������, �.�. ��� ����� ������� F_CPU ������
// ���� ������ 25.6MHz
#define ADCSRA_DIV	ADCSRA_DIV128
#define ADC_FREQ	(F_CPU / 128)
#warning "ADC: A frequency divider for the ADC = 128"
#warning "ADC: WARNING! Frequency ADC > ADC_FREQ_MAX"
#endif

#endif	// #if defined(ADC_FREQ_MAX)




#if defined(ADC_FREQ_MIN)

// ������������ �������� ������� ��� ��� ������� ��������������
// ADC_FREQ_MIN

#define DIVIDER (F_CPU / ADC_FREQ_MIN)

#if (DIVIDER < 2)
#define ADCSRA_DIV	ADCSRA_DIV2
#define ADC_FREQ	(F_CPU / 2)
#warning "ADC: A frequency divider for the ADC = 2"
#warning "ADC: WARNING! Frequency ADC < ADC_FREQ_MIN"

#elif (DIVIDER < 4)
#define ADCSRA_DIV	ADCSRA_DIV2
#define ADC_FREQ	(F_CPU / 2)
#warning "ADC: A frequency divider for the ADC = 2"

#elif (DIVIDER < 8)
#define ADCSRA_DIV	ADCSRA_DIV4
#define ADC_FREQ	(F_CPU / 4)
#warning "ADC: A frequency divider for the ADC = 4"

#elif (DIVIDER < 16)
#define ADCSRA_DIV	ADCSRA_DIV8
#define ADC_FREQ	(F_CPU / 8)
#warning "ADC: A frequency divider for the ADC = 8"

#elif (DIVIDER < 32)
#define ADCSRA_DIV	ADCSRA_DIV16
#define ADC_FREQ	(F_CPU / 16)
#warning "ADC: A frequency divider for the ADC = 16"

#elif (DIVIDER < 64)
#define ADCSRA_DIV	ADCSRA_DIV32
#define ADC_FREQ	(F_CPU / 32)
#warning "ADC: A frequency divider for the ADC = 32"

#elif (DIVIDER < 128)
#define ADCSRA_DIV	ADCSRA_DIV64
#define ADC_FREQ	(F_CPU / 64)
#warning "ADC: A frequency divider for the ADC = 64"

#else
#define ADCSRA_DIV	ADCSRA_DIV128
#define ADC_FREQ	(F_CPU / 128)
#warning "ADC: A frequency divider for the ADC = 128"
#endif

#endif	// #if defined(ADC_FREQ_MIN)

#endif	// #if !defined(_ADC_H_)
