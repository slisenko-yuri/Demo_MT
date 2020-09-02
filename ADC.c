#include "Config.h"
#include "ADC.h"




#if defined(ADC_FREQ)

#if (ADC_FREQ > 200000UL)
#warning "ADC: WARNING! Frequency ADC > 200kHz!"

#elif (ADC_FREQ < 50000UL)
#warning "ADC: WARNING! Frequency ADC < 50kHz!"

#elif (ADC_FREQ == 50000UL)
#warning "ADC: Frequency ADC = 50kHz"

#elif (ADC_FREQ < 75000UL)
#warning "ADC: Frequency ADC in range 50kHz...75kHz"

#elif (ADC_FREQ == 75000UL)
#warning "ADC: Frequency ADC = 75kHz"

#elif (ADC_FREQ < 100000UL)
#warning "ADC: Frequency ADC in range 75kHz...100kHz"

#elif (ADC_FREQ == 100000UL)
#warning "ADC: Frequency ADC = 100kHz"

#elif (ADC_FREQ < 125000UL)
#warning "ADC: Frequency ADC in range 100kHz...125kHz"

#elif (ADC_FREQ == 125000UL)
#warning "ADC: Frequency ADC = 125kHz"

#elif (ADC_FREQ < 150000UL)
#warning "ADC: Frequency ADC in range 125kHz...150kHz"

#elif (ADC_FREQ == 150000UL)
#warning "ADC: Frequency ADC = 150kHz"

#elif (ADC_FREQ < 175000UL)
#warning "ADC: Frequency in range 150kHz...175kHz"

#elif (ADC_FREQ == 175000UL)
#warning "ADC: Frequency ADC = 175kHz"

#else
#warning "ADC: Frequency ADC in range 175kHz...200kHz"

#endif
#endif
