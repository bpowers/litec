#include <EVB.h>
#include <stdio.h>
#include <stdlib.h>

#define INLINE __attribute__((always_inline))
#define MAX_UINT16 (65435)

typedef int32 long
typedef uint16 unsigned int
typedef int16 int
typedef uint8 unsigned 

static volatile bool new_speed;

static uint8 actual_speed;
static uint8 prev_error;

static uint8 lost_side;
static uint8 lost_track;

static const int16 DEFAULT_GAIN = 3;

// Set the steering limits
static const uint16 LIM_LEFT = 2150;
static const uint16 LIM_RIGHT = 3245;
static const uint16 LIM_CENTER = 2675;


static bool
pushbutton_enabled(void) INLINE
{
	if (_H11PORTA & 0x01)
		return true;
	return false;
}

static bool
slideswitch_enabled(void) INLINE
{
	if (_H11PORTA & 0x04)
		return true;
	return false;
}

static bool
standby_mode(void) INLINE
{
	return !slideswitch_enabled();
}

static void
standby(void)
{
	_H11TMSK2 &= ~0x40; /* disable inturrupts	*/
	_H11OC1M = 0x00;	/* disable TOC1	*/

	// wait for the slideswitch to turn back on...
	while (standby_mode());
	
	_H11TMSK2 |= 0x40;	/* enable inturrupts	*/
	_H11OC1M = 0x18;	/* TOC1 affects PA3 and PA4 */
}

static void
init(void)
{
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

	steer(LIM_CENTER);
	_H11TOC5 = 100;
}

static int16
sense_line(void)
{
	int16 sense_l, sense_r, sense_diff;

	_H11ADCTL = 0x14;
	while (!(_H11ADCTL & 0x80));

	sense_l = _H11ADR2;
	sense_r = _H11ADR3;
	sense_diff = sense_l - sense_r;

	if (sense_diff > 175) {
		side = 'l';
		lost_track = 0;
	} else if (sense_diff < -175) {
		side = 'r';
		lost_track = 0;
	}

	if (abs(sense_diff) < 25 && abs(sense_l) + abs(sense_r) < 100)
		lost_track = side;

	return sense_diff;
}

static int16
percent_to_pwm(int16 percent) INLINE
{
	uint16 lim;
	uint16 abs_percent;

	if (percent == 0)
		return LIM_CENTER;

	if (percent < 0)
		lim = LIM_LEFT;
	else
		lim = LIM_RIGHT;

	// clamp to 100%
	abs_percent = abs(percent);
	if (abs_percent > 100)
		abs_percent = 100;

	return (lim - LIM_CENTER)*abs_percent + LIM_CENTER;
}

static void
steer(int16 percent)
{
	// Don't change if the pins about to toggle, bad things could happen
	if (likely(abs(_H11TOC5 - _H11TCNT) > 100)) {
		_H11TOC5 = percent_to_pwm(percent);;
	}
}

static void
drive(uint8 percent)
{
	int32 curr;
	uint8 error;

	error = (_H11ADR1 * percent)/300 - actual_speed;

	curr = _H11TOC5 + ( 300 * error ) - ( 100 * prev_error );
	prev_error = error;

	if (curr > MAX_UINT16)
		curr = MAX_UINT16;
	else if (curr < 100)
		curr = 100;

	if (abs(motorpw - _H11TCNT) > 100 )
		motorpw = curr;

	return;
}

__mod2__ void
timer_interrupt(void)
{
	// set the value stored in _H11PACNT equal to the actual speed and reset _H11PACNT here
	actual_speed = _H11PACNT;
	_H11PACNT = 0;

	new_speed = 1;				// this indicates that a new speed value is ready to be processed
	_H11TFLG2 |= 0x40;			// reset the real time interrupt flag

	return;
}

int
main(void)
{
	uint16 speed;
	int16 gain;
	int16 dir;

	gain = DEFAULT_GAIN;

	init();

	while (true) {
		if (standby_mode()) {
			standby();
			continue;
		}

		dir = sense_line() * gain;

		if (lost_track) {
			speed = 75;
			if (lost_side == 'l')
				dir = -100;
			else
				dir = 100;
		} else {
			speed = 100;
		}

		if (new_speed) {
			drive(speed);
			new_speed = 0;
		}
		steer(dir);
	}

	return 0;
}
