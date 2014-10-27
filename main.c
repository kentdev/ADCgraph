#include "m_general.h"
#include "m_usb.h"

#define FREQ (F_CPU / pow (2, CLOCKDIVIDE))

#define NUM_STORED 80

#define GRAPH_ROWS 10
#define GRAPH_COLS 80

uint16_t adc_history[NUM_STORED];

uint16_t refresh_counter = 0;

int main (void)
{
	// set reference voltage to internal 5V
	set   (ADMUX, REFS0);
	clear (ADMUX, REFS1);
	
	m_clockdivide (0);  // 16MHz clock
	
	m_usb_init();
	
	// set the ADC prescaler to get the ADC clock to 125kHz
	set (ADCSRA, ADPS0);
	set (ADCSRA, ADPS1);
	set (ADCSRA, ADPS2);
	
	// select F0 as the ADC input
	clear (ADMUX,  MUX0);
	clear (ADMUX,  MUX1);
	clear (ADMUX,  MUX2);
	clear (ADCSRB, MUX5);
	
	// enable ADC subsystem
	set (ADCSRA, ADEN);
	
	m_green (ON);
	
	for (;;)
	{
		// VT100 command codes to draw info
		m_usb_tx_char (0x1b);
		m_usb_tx_string ("[H");  // send cursor to home
		
		set (ADCSRA, ADSC);  // begin an ADC conversion
		
		// push the stored values to make room at the end
		for (uint8_t i = 0; i < NUM_STORED - 1; i++)
		    adc_history[i] = adc_history[i + 1];
		
		while (check (ADCSRA, ADSC));  // wait until the conversion is done
		
		adc_history[NUM_STORED - 1] = ADC;  // store the new value at the end
		
		
		
		// clear the screen roughly every couple seconds, in case extra junk was drawn
		if (refresh_counter >= 200)
		{
		    m_usb_tx_char (0x1b);  // send ESC
        	m_usb_tx_string ("[2J");  // erase screen
        	
        	refresh_counter = 0;
		}
		else
		{
    		refresh_counter++;
		}
		
		// print most recent value
		for (uint8_t i = 0; i < GRAPH_COLS - 6; i++)
		    m_usb_tx_char (' ');
		
        if (adc_history[NUM_STORED - 1] < 1000)
            m_usb_tx_char (' ');
        if (adc_history[NUM_STORED - 1] < 100)
            m_usb_tx_char (' ');
        if (adc_history[NUM_STORED - 1] < 10)
            m_usb_tx_char (' ');
        m_usb_tx_uint (adc_history[NUM_STORED - 1]);
		    
	    m_usb_tx_string ("\n\n");
	    
	    for (uint8_t i = 0; i < GRAPH_COLS; i++)
	        m_usb_tx_char ('-');
        m_usb_tx_string ("\n");
	    
	    // draw a graph (don't need to clear this part of the screen,
	    // since it gets completely overwritten each time)
	    for (uint8_t y = 0; y < GRAPH_ROWS; y++)
	    {
	        for (uint8_t x = 0; x < GRAPH_COLS; x++)
	        {
	            if (adc_history[x * NUM_STORED / GRAPH_COLS] >= 1023 * (GRAPH_ROWS - y) / GRAPH_ROWS)
	                m_usb_tx_char ('#');
                else
                    m_usb_tx_char (' ');
	        }
	        
	        m_usb_tx_string ("\n");
		}
		
		for (uint8_t i = 0; i < GRAPH_COLS; i++)
	        m_usb_tx_char ('-');
        m_usb_tx_string ("\n");
		
		m_wait (10);
	}
}

