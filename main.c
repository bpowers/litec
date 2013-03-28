/* whole integrated system... boo yah! */
#include <EVB.h>
#include <stdlib.h>	/* needed for abs function*/
#include <stdio.h>	/* needed for printf*/

//  ********************************  //
//  CHANGE SPEED POT FROM PE3 TO PE4  //
//  ********************************  //

__mod2__ void timer_interrupt(void);

void initialize( void );

int pushbutton( void );
int slideswitch( void );

int otu( void );

void steer( int steerpw );
void drive( void );

unsigned char actual_speed;
unsigned char prev_error;

unsigned char side;
unsigned char lost_track;

// Set the steering limits
unsigned int limit_left = 2150;
unsigned int limit_right = 3245;
unsigned int limit_center = 2675;


void main(void) { 

	unsigned char drive_speed;
	int multiplier = 3;
	int drive_speed;

	initialize();

	while( 1 ){	

		if ( slideswitch() ) {

			if ( lost_track ) {

				drive_speed = 75;

				if ( lost_track == "l" )
					steer( limit_left );
				else
					steer( limit_right );
			} else {

				drive_speed = 100;
				steer( multiplier*otu() + limit_center );
			}

			if ( new_speed ) {

				drive( 100 );
				new_speed = 0;
			}
		} else {

			_H11TMSK2 &= ~0x40; /* disable inturrupts	*/
			_H11OC1M = 0x00;	/* disable TOC1	*/

			// wait for the slideswitch to turn back on...
			while ( !slideswitch() );
			
			_H11TMSK2 |= 0x40;	/* enable inturrupts	*/
			_H11OC1M = 0x18;	/* TOC1 affects PA3 and PA4 */
		}
	}

	return;
}


void initialize(void) {

	_H11OC1M = 0x18;	/* TOC1 affects PA3 and PA4 */
	_H11OC1D = 0x18;	/* TOC1 sets PA3 and PA4	*/
	_H11TOC1 = 0x00;	/* TOC1 occurs when _H11TCNT equals zero */

	_H11TCTL1 = 0x0A;	/* TOC5 clears PA3, TOC4 clears PA4 */

	// Get port C set for all output, and set the drive motor to forward.
	_H11DDRC  = 0xFF;
	_H11PORTC = 0x06;
	
	// setup timer_interrupt to respond to real time interrupts with _VECTOR and TMSK2 here
	_VECTOR(timer_interrupt, 13);
	// setup real time interrupt rate for 32.77ms and the pulse accumulator to detect rising edges here
	_H11PACTL |= 0x53;
	// enable real time inturrupts
	_H11TMSK2 |= 0x40;
	_H11TFLG2 |= 0x40;

	steer( limit_center );
	_H11TOC5 = 100;

	return;
}


int otu( void ) {

	_H11ADCTL = 0x14;
	while ( !(_H11ADCTL & 0x80) );

	if ( (int)(_H11ADR2 - _H11ADR3) > 175 ) {
		side = 'l';
		lost_track = 0;
	} else if ( (int)(_H11ADR2 - _H11ADR3) < (-175) ) {
		side = 'r';
		lost_track = 0;
	}

	if ( (abs(_H11ADR2 - _H11ADR3) < 25) && ( ( abs(_H11ADR2) + abs(_H11ADR3) ) < 100 ) )
		lost_track = side;

	return ( _H11ADR2 - _H11ADR3 );
}


void steer( int steerpw ) {

	// Don't change if the pins about to toggle, bad things could happen
	if (abs(_H11TOC5 - _H11TCNT) > 100) { 

		if ( steerpw > limit_right )
			_H11TOC5 = limit_right;
		else if ( steerpw < limit_left )
			_H11TOC5 = limit_left;
		else
			_H11TOC5 = steerpw;
	}

	return;
}


void drive( unsigned char percent ) {

	long int temp_motorpw;
	unsigned char error;

	// TYPECAST

	error = (_H11ADR1 * percent)/300 - actual_speed;

	temp_motorpw = _H11TOC5 + ( 300 * error ) - ( 100 * prev_error );
	prev_error = error;

	if( temp_motorpw > 65435 )
		temp_motorpw = 65435;
	else if( temp_motorpw < 100 )
		temp_motorpw = 100;

	if( abs( motorpw - _H11TCNT ) > 100 )
		motorpw = temp_motorpw;

	return;
}


__mod2__ void timer_interrupt( void ) {

	// set the value stored in _H11PACNT equal to the actual speed and reset _H11PACNT here
	actual_speed = _H11PACNT;
	_H11PACNT = 0;

	new_speed = 1;				// this indicates that a new speed value is ready to be processed
	_H11TFLG2 |= 0x40;			// reset the real time interrupt flag

	return;
}


int pushbutton( void ) {

	// If the pushbutton is not pressed, then pa1 is high, so return false.
	if ( _H11PORTA & 0x01 )
		return 0;

	// Otherwise return true.
	return 1;
}


int slideswitch( void ) {

	// If slideswitch is high, return true, otherwise false.
	if ( _H11PORTA & 0x04 )
		return 1;

	return 0;
}
