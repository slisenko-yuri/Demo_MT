#if !defined(_ADC_H_)
#define _ADC_H_

#include "ADC_Cfg.h"

/*************************************************************************
-Для использования АЦП необходимо:
  PRR.PRADC = 0
  ADCSRA.ADEN = 1
 
-При чтении 10-битного результата преобразования вначале нужно читать
 ADCL, затем ADCH.
-Первое преобразование занимает 25 тактов АЦП.
-Последующие преобразования занимают 13 тактов АЦП.
-Безопасно изменить ADMUX можно когда ADATE или ADEN очищены.
-Когда активен режим Free Running mode лучше не менять канал и опорное
 напряжение.
-После переключения опорного напряжения первое преобразование может быть
 неверным, поэтому лучше игнорировать это измерение.
-После завершения преобразования ADCSRA.ADIF = 1.

Для получения максимальной точности измерений АЦП должен тактироваться с
частотой 50...200кГц. Если не требуется точность 10 бит, то АЦП может
тактироваться с частотой большей, чем 200кГц
*************************************************************************/




// Примеры инициализации и использования АЦП

//========================================================================
// 1. Для однократных 10-битных измерений по каналу 2 в диапазоне 0...1.1V
//    без прерываний :
//------------------------------------------------------------------------
//	DIDR0 |= 1<<ADC2D;		// Отключение цифрового буфера на входе АЦП
//	ADMUX = ADMUX_REF_1_1 | ADMUX_DATA_RIGHT | ADMUX_MUX_2;
//					//ADMUX_REF_1_1 - Опорное = 1.1V (встроенный источник)
//					//ADMUX_DATA_RIGHT - Выравнивание результата вправо
//					//(оптимально для получения 10-битного результата)
//					// ADMUX_MUX_2 - Канал измерения ADC2
//	ADCSRB = 0;
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_DIV;	
//					//ADCSRA_ADEN - Разрешение АЦП
//					//ADCSRA_DIV - Константа, определяющая частоту
//					//преобразования АЦП (Делитель для F_CPU, который
//					//вычисляется на основании константы ADC_FREQ_MAX
//					//или ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;			//ADCSRA_ADSC - Старт преобразования
//	while (ADCSRA & ADCSRA_ADSC);	//Ожидание завершения преобразования
//	uint16_t Result = ADC;			//Чтение результата
//
//========================================================================

//========================================================================
// 2. Для автоматических 8-битных измерений по каналу 3 в диапазоне
//    0...AREF с использованием прерываний:
//------------------------------------------------------------------------
//	DIDR0 |= 1<<ADC3D;		// Отключение цифрового буфера на входе АЦП
//	ADMUX = ADMUX_REF_AREF | ADMUX_DATA_LEFT | ADMUX_MUX_3;
//					//ADMUX_REF_AREF - Опорное = AREF
//					//ADMUX_DATA_LEFT - Выравнивание результата влево
//					//     (оптимально для получения 8-битного результата)
//					//ADMUX_MUX_3 - Канал измерения ADC3
//
//	ADCSRB = ADCSRB_SRC_FREE_RUN;							
//					//ADCSRB_SRC_FREE_RUN - Режим непрерывных
//					//автоматических измерений. Необходимо устанавливать
//					//один из режимов автоматических измерений
//					// (ADCSRB_SRC_...), если в регистре ADCSRA установлен
//					//бит ADCSRA_ADATE
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_ADATE | ADCSRA_ADIF
//		| ADCSRA_ADIE | ADCSRA_DIV;
//					//ADCSRA_ADEN - Разрешение АЦП,
//					//ADCSRA_ADATE - Разрешение автоматических
//					//               измерений,
//					//ADCSRA_ADIF - Сброс флага прерывания,
//					//ADCSRA_ADIE - Разрешение прерываний АЦП
//					//ADCSRA_DIV - Константа, определяющая частоту
//					//преобразования АЦП (Делитель для F_CPU, который
//					//вычисляется на основании константы ADC_FREQ_MAX или
//					//ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;	// Старт преобразования
//
//	...
//	ISR(ADC_vect)				// Функция обработки прерывания АЦП
//	{
//		uint8_t Result = ADCH;	// Считывание 8-битного результата
//	}
//
//========================================================================

//========================================================================
// 3. Для однократных 8-битных измерений в диапазоне 0...AVCC с
//    использованием прерываний и двух каналов АЦП (4 и 5):
//------------------------------------------------------------------------
//	DIDR0 |= (1<<ADC4D)|(1<<ADC5D);
//				//Отключение цифровых буферов на входах АЦП
//	ADMUX = ADMUX_REF_AVCC | ADMUX_DATA_LEFT | ADMUX_MUX_4;
//				//ADMUX_REF_AVCC - Опорное = AVCC (Напряжение питания)
//				//ADMUX_DATA_LEFT - Выравнивание результата влево
//				//	(оптимально для получения 8-битного результата)
//				//ADMUX_MUX_4 - Канал измерения ADC4
//
//	ADCSRB = 0;
//
//	ADCSRA = ADCSRA_ADEN | ADCSRA_ADIF | ADCSRA_ADIE | ADCSRA_DIV;
//				//ADCSRA_ADEN - Разрешение АЦП,
//				//ADCSRA_ADIF - Сброс флага прерывания,
//				//ADCSRA_ADIE - Разрешение прерываний АЦП
//				//ADCSRA_DIV - Константа, определяющая частоту преобразования
//				//	АЦП (Делитель для F_CPU, который вычисляется на
//				//	основании константы ADC_FREQ_MAX или ADC_FREQ_MIN)
//
//	ADCSRA |= ADCSRA_ADSC;						// Старт преобразования
//
//	...
//  // Функция обработки прерывания АЦП (расположена вне функции main)
//	ISR(ADC_vect)
//	{
//		uint8_t Result;
//		Result = ADCH;					// Считывание 8-битного результата
//		if (Chanel == ADMUX_MUX_5)		// Смена канала измерения
//			ADMUX = (ADMUX & ~ADMUX_MUX_MASK) | ADMUX_MUX_4;
//		else
//			ADMUX = (ADMUX & ~ADMUX_MUX_MASK) | ADMUX_MUX_5;
//		ADCSRA |= ADCSRA_ADSC;			// Старт преобразования
//	}
//========================================================================




// Константы для регистра ADMUX
//========================================================================




// Константы для выбора опорного напряжения
//------------------------------------------------------------------------

//Опорное напряжение берется с контакта AREF.
//Внутренний опорник отключается.
#define	ADMUX_REF_AREF	((0<<REFS1)|(0<<REFS0))
		
//Опорное напряжение берется с контакта AVCC (питание).
#define ADMUX_REF_AVCC	((0<<REFS1)|(1<<REFS0))
	
//Внутреннее опорное напряжение (1.1V или другое значение)
#define ADMUX_REF_1_1	((1<<REFS1)|(1<<REFS0))




// Константы для выбора выравнивания результата
//------------------------------------------------------------------------

//Сдвиг результата влево (8 разрядов ADCH и 2 старших разряда ADCL)
#define ADMUX_DATA_LEFT		(1<<ADLAR)
	
//Правое выравнивание (2 младших разряда ADCH и 8 разрядов ADCL)
#define ADMUX_DATA_RIGHT	(0<<ADLAR)




// Константы для выбора канала АЦП
//------------------------------------------------------------------------

// Канал 0
#define ADMUX_MUX_0	((0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0))

// Канал 1
#define ADMUX_MUX_1	((0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0))

// Канал 2
#define ADMUX_MUX_2	((0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(0<<MUX0))

// Канал 3
#define ADMUX_MUX_3	((0<<MUX3)|(0<<MUX2)|(1<<MUX1)|(1<<MUX0))

// Канал 4
#define ADMUX_MUX_4	((0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(0<<MUX0))

// Канал 5
#define ADMUX_MUX_5	((0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0))

// Канал 6
#define ADMUX_MUX_6	((0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0))

// Канал 7
#define ADMUX_MUX_7	((0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))

// Температ.
#define ADMUX_MUX_T	((1<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0))

// Внутренний источник опорного напряжения 1.1V
#define ADMUX_MUX_REF ((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(0<<MUX0))

// 0V
#define ADMUX_MUX_GND ((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))

// Маска для каналов АЦП
#define ADMUX_MUX_MASK	((1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))




// Константы для регистра ADCSRA
//========================================================================




// Разрешение АЦП
#define ADCSRA_ADEN				(1<<ADEN)

// Старт преобразования АЦП. После завершения преобразования
// сбрасывается в 0. Можно использовать как бит занятости АЦП.
#define ADCSRA_ADSC				(1<<ADSC) //ADc Start Conversion

//Разрешение автоматического запуска преобразования по событию
//(Источник события определяется битами ADTS2..ADTS0 в регистре
//ADCSRB)
#define ADCSRA_ADATE			(1<<ADATE) //ADc Auto Trigger Enable

//Флаг прерывания АЦП. Сбрасывается аппаратно в обработчике
//прерывания или записью 1.
#define ADCSRA_ADIF				(1<<ADIF) //ADC Interrupt Flag

// Бит разрешения прерывания АЦП
#define ADCSRA_ADIE				(1<<ADIE) //ADC Interrupt Enable




// Константы для делителя частоты АЦП
//------------------------------------------------------------------------

// ADC Prescaler Select
#define ADCSRA_DIV2		((0<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)) // 1/2
#define ADCSRA_DIV4		((0<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)) // 1/4
#define ADCSRA_DIV8		((0<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)) // 1/8
#define ADCSRA_DIV16	((1<<ADPS2)|(0<<ADPS1)|(0<<ADPS0)) // 1/16
#define ADCSRA_DIV32	((1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)) // 1/32
#define ADCSRA_DIV64	((1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0)) // 1/64
#define ADCSRA_DIV128	((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)) // 1/128

//Маска для делителя частоты АЦП
#define ADCSRA_DIV_MASK	((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))




// Константы для регистра ADCSRB
//========================================================================




#define ADCSRB_ACME	(1<<ACME)	//Analog Comparator Multiplexer Enable
								//Если АЦП разрешен, то бит ACME должен
								//быть равен лог.0




//Константы для выбора источника запуска АЦП при включенном режиме
//автозапуска (при ADATE=1 в регистре ADCSRA).
//Запуск АЦП производится положительным фронтом сигнала запуска.
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
		
//Маска для источника запуска АЦП
#define ADCSRB_SRC_MASK			((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0))




// Проверям, чтобы была определена только одна из констант,
// задающих желаемую частоту АЦП
#if defined(ADC_FREQ_MIN) && defined(ADC_FREQ_MAX)
#undef ADC_FREQ_MIN
#undef ADC_FREQ_MAX
#error "ADC: Must be defined ADC_FREQ_MIN only or ADC_FREQ_MAX only!"
#endif




#if defined(ADC_FREQ_MAX)

// Подсчитываем делитель частоты АЦП для частоты преобразования
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
// Сюда не должно попасть, т.к. для этого частота F_CPU должна
// быть больше 25.6MHz
#define ADCSRA_DIV	ADCSRA_DIV128
#define ADC_FREQ	(F_CPU / 128)
#warning "ADC: A frequency divider for the ADC = 128"
#warning "ADC: WARNING! Frequency ADC > ADC_FREQ_MAX"
#endif

#endif	// #if defined(ADC_FREQ_MAX)




#if defined(ADC_FREQ_MIN)

// Подсчитываем делитель частоты АЦП для частоты преобразования
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
