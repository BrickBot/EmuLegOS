//---------------------------------------------------------------------------
#ifndef emuLegOsH
#define emuLegOsH
//---------------------------------------------------------------------------
#include <string>
#include <unistd.h>
#include <semaphore.h>
//---------------------------------------------------------------------------

  using namespace std;


namespace emulegos {
// Constants
// ---------


// min and max motor speed
#define  MIN_SPEED	0
#define  MAX_SPEED	255

// standard sensor values
#define TOUCH_ON             (0)
#define TOUCH_OFF       (0xFFFF)
#define MIN_SENSOR           (0)
#define MAX_SENSOR      (0xFFFF)
#define STATE_0_VALUE (1023U<<6)
#define STATE_1_VALUE  (833U<<6)
#define STATE_2_VALUE  (405U<<6)
#define STATE_3_VALUE  (623U<<6)

// access to sensor values
#define SENSOR_1	(::emulegos::_emu_sensor[2])  // sensor 1
#define SENSOR_2	(::emulegos::_emu_sensor[1])  // sensor 2
#define SENSOR_3	(::emulegos::_emu_sensor[0])  // sensor 3


//! Convert raw data to touch sensor (0: off, else pressed)
#define TOUCH(a)    ((unsigned int)(a) < 0x8000)

//  Processed touch sensors
//
#define TOUCH_1     TOUCH(SENSOR_1)
#define TOUCH_2     TOUCH(SENSOR_2)
#define TOUCH_3     TOUCH(SENSOR_3)


// active light sensor: estimated raw values
#define LIGHT_RAW_BLACK 0xffc0     // active light sensor raw black value
#define LIGHT_RAW_WHITE 0x5080     // active light sensor raw white value

// convert raw values to 0 (dark) .. LIGHT_MAX (bright)
// roughly 0-100.
#define LIGHT(a)    (147 - ds_scale(a)/7) // map light sensor to 0..LIGHT_MAX
#define LIGHT_MAX   LIGHT(LIGHT_RAW_WHITE)        // maximum decoded value

// processed active light sensor
#define LIGHT_1     LIGHT(SENSOR_1)
#define LIGHT_2     LIGHT(SENSOR_2)
#define LIGHT_3     LIGHT(SENSOR_3)

// processed rotation sensor
// NOTE: I am not putting ::emulegos::ds_rotations as I did with
// the sensors this is because ds_rotations is valid in legos
// if the user makes the mistake of creating a local variable
// ds_rotations, at least they'll see the same strange behavior
// in the emulator and rcx
#define ROTATION_1  (ds_rotations[2])
#define ROTATION_2  (ds_rotations[1])
#define ROTATION_3  (ds_rotations[0])

#define VELOCITY_1  (ds_velocities[2])
#define VELOCITY_2  (ds_velocities[1])
#define VELOCITY_3  (ds_velocities[0])


  //multiplexed sensor, this available as part of a legOS patch
#define SENSOR_1A (ds_muxs[2][0])
#define SENSOR_1B (ds_muxs[2][1])
#define SENSOR_1C (ds_muxs[2][2])
#define SENSOR_2A (ds_muxs[1][0])
#define SENSOR_2B (ds_muxs[1][1])
#define SENSOR_2C (ds_muxs[1][2])
#define SENSOR_3A (ds_muxs[0][0])
#define SENSOR_3B (ds_muxs[0][1])
#define SENSOR_3C (ds_muxs[0][2])

// scale/unscale macros
#define ds_scale(x)   (x>>6)
#define ds_unscale(x) (x<<6)

// buttons
#define	BUTTON_ONOFF	0x0002
#define BUTTON_RUN	    0x0004
#define BUTTON_VIEW	    0x4000
#define BUTTON_PROGRAM	0x8000

// testing buttons
#define RELEASED(state,button)	((state) & (button))
#define PRESSED(state,button)	(!RELEASED(state,button))

// Types
// -----

// the motor directions
typedef enum {
	off=0,          //!< freewheel
	fwd=1,          //!< forward
	rev=2,          //!< reverse
	brake=3         //!< hold current position
}
MotorDirection;

// Variables
// ---------

extern volatile unsigned _emu_sensor[3];  // sensor value
extern volatile unsigned BATTERY;    //batter sensor
extern int _emu_active_flag[3];           // sensor activation flags
extern int _emu_rotation_flag[3];         // rotation activation flags
extern volatile int ds_rotations[3];    // rotations count
extern volatile int ds_velocities[3];    //velocity
extern volatile unsigned _emu_buttons;    // rcx buttons
 extern volatile unsigned _emu_mux_vals[3][3];//mux values
extern volatile unsigned ds_muxs[3][3];	//!< mux ch values

// System functions
// ----------------

/*extern unsigned int sleep(unsigned int sec);*/
extern unsigned int msleep(unsigned int msec);
extern void delay(unsigned ms);
extern int get_battery_mv();

// Time functions
// --------------
#include <time.h>
extern time_t sys_time_ ();
#define sys_time sys_time_()

// Task management function
// ----------------
#define DEFAULT_STACK_SIZE 1024
#ifdef _MSC_VER
typedef int pid_t;
#endif
typedef unsigned char priority_t;
#define PRIO_LOWEST      	1
#define PRIO_NORMAL     	10
#define PRIO_HIGHEST     	20

extern pid_t execi (int (*code_start)(int argc,char *argv[]), int argc, char **argv, priority_t priority, size_t stack_size);
extern void tm_start (void);
extern void kill(pid_t pid);
#define yield() pthread_yield()
extern void killall(priority_t p);
typedef unsigned long wakeup_t;
extern wakeup_t wait_event(wakeup_t (*wakeup)(wakeup_t),wakeup_t data);
    extern void poweroff();
#define reset() poweroff();
    int random(void);
    void srandom(int seed);



#define __persistent //does nothing, all globals are static in the emulator

// Motor functions
// ---------------
extern void motor_a_dir(MotorDirection dir);
extern void motor_b_dir(MotorDirection dir);
extern void motor_c_dir(MotorDirection dir);
extern void motor_a_speed(unsigned char speed);
extern void motor_b_speed(unsigned char speed);
extern void motor_c_speed(unsigned char speed);
extern void dm_init(void);
extern void dm_shutdown(void);

// Sensor functions
// ----------------

extern void ds_active(volatile unsigned* const sens);
extern void ds_passive(volatile unsigned* const sens);
extern void ds_rotation_set(volatile unsigned* const sens,int pos);
extern void ds_rotation_on(volatile unsigned* const sens);
extern void ds_rotation_off(volatile unsigned* const sens);
extern void ds_mux_on(volatile unsigned *sensor,
		      unsigned int ch1,
		      unsigned int ch2,
		      unsigned int ch3);
 extern void ds_mux_off(volatile unsigned *sensor);
extern void ds_init(void);
extern void ds_shutdown(void);

// Button functions
// ----------------
#define KEY_ONOFF 0x01		//!< the on/off key is pressed
#define KEY_RUN   0x02		//!< the run key is pressed
#define KEY_VIEW  0x04		//!< the view key is pressed
#define KEY_PRGM  0x08		//!< the program key is pressed

#define KEY_ANY   0x0f		//!< any of the keys

extern int dbutton(void);
#ifdef getchar
//cygwin fix
#  undef getchar
#endif
extern int getchar(void);

//! wakeup if any of the given keys is pressed.
//
extern wakeup_t dkey_pressed(wakeup_t data);

//! wakeup if all of the given keys are released.
//
extern wakeup_t dkey_released(wakeup_t data);


// Lcd functions
// -------------
//! LCD number display styles
/*! note: signed and unsigned are taken by the C programming language
 */
typedef enum {
  digit  = 0x3017,		//!< single digit on the right
      	      	      	        // works only with comma_style digit
      
  sign   = 0x3001,		//!< signed, no leading zeros
  unsign = 0x301F		//!< unsigned, 0 displayed as 0000

} lcd_number_style;

//! LCD comma display styles
/*
 */
typedef enum {
  digit_comma = 0x0000,		//!< single digit on the right
      	      	      	        // works only with number_style digit

  e0  = 0x3002,			//!< whole
  e_1 = 0x3003,	  	      	//!< 10ths
  e_2 = 0x3004,		      	//!< 100ths
  e_3 = 0x3005,		      	//!< 1000ths, problematic with negatives

} lcd_comma_style;		// lcd display comma style


//! LCD segment codes
/*! these are not to be confused with the codes defined in direct-lcd.h

   circle and dot codes cycle. cycle state is preserved on powerdown.

   each dot code should be invoked six times before using the other.
   mixing them will result in strange behaviour.

 */
typedef enum {
  man_stand = 0x3006,
  man_run   = 0x3007,

  s1_select = 0x3008,
  s1_active = 0x3009,

  s2_select = 0x300A,
  s2_active = 0x300B,

  s3_select = 0x300C,
  s3_active = 0x300D,

  a_select  = 0x300E,
  a_left    = 0x300F,
  a_right   = 0x3010,

  b_select  = 0x3011,
  b_left    = 0x3012,
  b_right   = 0x3013,

  c_select  = 0x3014,
  c_left    = 0x3015,
  c_right   = 0x3016,

  unknown_1 = 0x3017,		//!< seemingly without effect. cycle reset?

  circle    = 0x3018,		//!< 0..3 quarters: add one. 4 quarters: reset

  dot       = 0x3019,		//!< 0..4 dots: add a dot. 5 dots: reset
      	      	      	        // dots are added on the right.
  dot_inv   = 0x301A,		//!< 0 dots: show 5. 1..4 dots: subtract one
      	      	      	        // dots are removed on the left

  battery_x = 0x301B,

  ir_half   = 0x301C, 	    	//! the IR display values are mutually exclusive.
  ir_full   = 0x301D,   	//! the IR display values are mutually exclusive.

  everything= 0x3020

} lcd_segment;

void cputs(string s);
void cputw(unsigned word);

void cputc_hex_0(unsigned char c);
void cputc_hex_1(unsigned char c);
void cputc_hex_2(unsigned char c);
void cputc_hex_3(unsigned char c);
void cputc_hex_4(unsigned char c);
void cputc_hex_5(unsigned char c);

void lcd_hide(char mask);
void lcd_show(char mask);

void lcd_number(int i, lcd_number_style n, lcd_comma_style c);


//! display an integer in decimal
#define lcd_int(i)	lcd_number(i,sign,e0)

//! display an unsigned value in decimal
#define lcd_unsigned(u)	lcd_number(u,unsign,e0)

//! display a clock.
/*! passing an argument of 1015 will display 10.15
 */
#define lcd_clock(t)	lcd_number(t,unsign,e_2)

//! display a single digit right of the man symbol
#define lcd_digit(d)	lcd_number(d,digit,digit_comma)

extern void lcd_clear(void);
extern void lcd_refresh(void);
extern void cls();

// Sound functions
//----------------
//NOTE: Sound is only limitly supported.  Basically all the logic should
//act normal, but don't expect to hear much.  When ANY note list is played
//you will hear one beep.  Thats it :) of course there is no reason that
//full sound support couldn't be implemented, have at it

//! a note 
typedef struct {
  unsigned char pitch; 	    //!< note pitch, 0 ^= A_0 (~55 Hz)
  unsigned char length;     //!< note length in 1/16ths
} note_t;

//! note lengths
#define WHOLE            16
#define HALF             8
#define QUARTER          4
#define EIGHTH           2

//! note pitches
// PITCH_H is European equivalent to American B note.

#define PITCH_A0 	 0
#define PITCH_Am0 	 1
#define PITCH_H0 	 2
#define PITCH_C1 	 3
#define PITCH_Cm1 	 4
#define PITCH_D1 	 5
#define PITCH_Dm1 	 6
#define PITCH_E1 	 7
#define PITCH_F1 	 8
#define PITCH_Fm1 	 9
#define PITCH_G1 	 10
#define PITCH_Gm1 	 11
#define PITCH_A1 	 12
#define PITCH_Am1 	 13
#define PITCH_H1 	 14
#define PITCH_C2 	 15
#define PITCH_Cm2 	 16
#define PITCH_D2 	 17
#define PITCH_Dm2 	 18
#define PITCH_E2 	 19
#define PITCH_F2 	 20
#define PITCH_Fm2 	 21
#define PITCH_G2 	 22
#define PITCH_Gm2 	 23
#define PITCH_A2 	 24
#define PITCH_Am2 	 25
#define PITCH_H2 	 26
#define PITCH_C3 	 27
#define PITCH_Cm3 	 28
#define PITCH_D3 	 29
#define PITCH_Dm3 	 30
#define PITCH_E3 	 31
#define PITCH_F3 	 32
#define PITCH_Fm3 	 33
#define PITCH_G3 	 34
#define PITCH_Gm3 	 35
#define PITCH_A3 	 36
#define PITCH_Am3 	 37
#define PITCH_H3 	 38
#define PITCH_C4 	 39
#define PITCH_Cm4 	 40
#define PITCH_D4 	 41
#define PITCH_Dm4 	 42
#define PITCH_E4 	 43
#define PITCH_F4 	 44
#define PITCH_Fm4 	 45
#define PITCH_G4 	 46
#define PITCH_Gm4 	 47
#define PITCH_A4 	 48
#define PITCH_Am4 	 49
#define PITCH_H4 	 50
#define PITCH_C5 	 51
#define PITCH_Cm5 	 52
#define PITCH_D5 	 53
#define PITCH_Dm5 	 54
#define PITCH_E5 	 55
#define PITCH_F5 	 56
#define PITCH_Fm5 	 57
#define PITCH_G5 	 58
#define PITCH_Gm5 	 59
#define PITCH_A5 	 60
#define PITCH_Am5 	 61
#define PITCH_H5 	 62
#define PITCH_C6 	 63
#define PITCH_Cm6 	 64
#define PITCH_D6 	 65
#define PITCH_Dm6 	 66
#define PITCH_E6 	 67
#define PITCH_F6 	 68
#define PITCH_Fm6 	 69
#define PITCH_G6 	 70
#define PITCH_Gm6 	 71
#define PITCH_A6 	 72
#define PITCH_Am6 	 73
#define PITCH_H6 	 74
#define PITCH_C7 	 75
#define PITCH_Cm7 	 76
#define PITCH_D7 	 77
#define PITCH_Dm7 	 78
#define PITCH_E7 	 79
#define PITCH_F7 	 80
#define PITCH_Fm7 	 81
#define PITCH_G7 	 82
#define PITCH_Gm7 	 83
#define PITCH_A7 	 84
#define PITCH_Am7 	 85
#define PITCH_H7 	 86
#define PITCH_C8 	 87
#define PITCH_Cm8 	 88
#define PITCH_D8 	 89
#define PITCH_Dm8 	 90
#define PITCH_E8 	 91
#define PITCH_F8 	 92
#define PITCH_Fm8 	 93
#define PITCH_G8 	 94
#define PITCH_Gm8 	 95
#define PITCH_A8 	 96

#define PITCH_PAUSE   	 97
#define PITCH_MAX     	 98
#define PITCH_END     	 255


//! system sounds
#define DSOUND_BEEP   	 0

#define DSOUND_SYS_MAX   1

//! default duration of 1/16th note in ms
#define DSOUND_DEFAULT_16th_ms  200

//! default duration internote spacing in ms
#define DSOUND_DEFAULT_internote_ms  15

extern unsigned dsound_16th_ms;   	      	 //!< length of 1/16 note in ms
extern unsigned dsound_internote_ms;  	      	 //!< length of internote spacing in ms
extern long int _emu_dsound_finish_ms;//time current sound will finish

void dsound_play(const note_t *notes);
int dsound_playing(void);

//! play a system sound
void dsound_system(unsigned nr);
    
//! set duration of a 16th note in ms
void dsound_set_duration(unsigned duration);

//! set duration of inter-note spacing (subtracted from note duration)
/*! set to 0 for perfect legato.
*/
    void dsound_set_internote(unsigned duration);

wakeup_t dsound_finished(wakeup_t data);
void dsound_stop(void);

// IR functions
//---------------
//the following are not part of lnplib, but are part of rcx lnp api
//so are needed for emulation
#if EMU_LNP_SUPPORT
    extern int _emu_lnp_far;

#define TX_IDLE 0
typedef void (*lnp_integrity_handler_t) (const unsigned char *, unsigned char);
typedef void (*lnp_addressing_handler_t) (const unsigned char *, unsigned char, unsigned char);
typedef enum { INIT_OK, INIT_BAD_PARAM, INIT_ERROR } lnp_init_result;
typedef enum { TX_SUCCESS, TX_FAILURE, TX_ERROR } lnp_tx_result;
#define LNP_DISCARD_WHILE_TX 1


//! dummy integrity layer packet handler
#define LNP_DUMMY_INTEGRITY ((lnp_integrity_handler_t)0)

//! dummy addressing layer packet handler
#define LNP_DUMMY_ADDRESSING ((lnp_addressing_handler_t)0)

//noops

    extern void lnp_logical_range(int far);
    extern int lnp_logical_range_is_far();

//overwrite existing lnp funtions
extern lnp_init_result lnp_init(
	char *tcp_hostname,
	unsigned short tcp_port,
	unsigned char lnp_address,
	unsigned char lnp_mask,
	int flags);

extern void lnp_shutdown(void);
extern void lnp_integrity_set_handler(lnp_integrity_handler_t handler);
extern void lnp_addressing_set_handler(unsigned char port, lnp_addressing_handler_t handler);
extern lnp_tx_result lnp_integrity_write(const unsigned char *data,unsigned char length);
extern lnp_tx_result lnp_addressing_write(const unsigned char *data,unsigned char length,
                         unsigned char dest,unsigned char srcport);

#endif //EMU_LNP_SUPPORT

// emulegOs API
// --------------------
//entry point for legOs code, this will call rcx_main
extern int rcx_start(int argc, char *argv[]);

extern void setCallbackPlayBeep( void (*_PlayBeep)(void) );
extern void setCallbackUpdateSensorStatus( void (*_UpdateSensorStatus)(int) );
extern void setCallbackUpdateSensorRawValues( void (*_UpdateSensorRawValues)(int, unsigned, int ) );
extern void setCallbackUpdateMuxSensorRawValues( void (*_UpdateMuxSensorValues)(int, int, unsigned ) );

extern void setCallbackUpdateMotorDir( void (*_UpdateMotorDir) ( int , MotorDirection ) );
extern void setCallbackUpdateMotorSpeed( void (*_UpdateMotorSpeed) ( int, unsigned char ) );
extern void setCallbackUpdateLcd( void (*_UpdateLcd)( string ) );
extern void setCallbackUpdateExecStatus(void (*_Exec)(int,int));

extern void setCallbackErrorMessage( void (*_ErrorMessage)( string ) );

    extern void emulegOsRcxEmulate( int (*rcx_main)(int argc,char *argv[]),
				    int enable_lnp,
				    const char* lnpdhost, int lnpd_tcp_port,
				    int mylnpd_hostid);
extern void emulegOsRcxStop( ) ;

extern MotorDirection emulegOsGetMotorDir( int );
extern unsigned char emulegOsGetMotorSpeed( int );
extern void emulegOsSetSensor ( int , unsigned );
 extern void emulegOsSetMuxSensor(int port, int sub, unsigned int value);
 extern void emulegOsMuxSet(int port, int sub);

extern void emulegOsRotateSensor ( int , int );

}//end namespace emulegos

using namespace emulegos;

extern int rcx_main(int argc, char *argv[]);




#endif
