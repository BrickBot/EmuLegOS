
/*
 *  emuLegOs 1.0
 *
 *  API functions
 *
 *  Contributor(s): Mario Ferrari <mario.ferrari@edis.it>
 *                  Marco Beri <marcob@equalis.it>
 *
 */

//---------------------------------------------------------------------------
#if EMU_LNP_SUPPORT
#  include <liblnp.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <limits.h>
#include "pthread.h"
#include "sched.h"
#include "semaphore.h"
#include "emuLegOs.h"
#include <iostream>

using std::cout;


namespace emulegos {
    
//---------------------------------------------------------------------------
// External variables
//---------------------------------------------------------------------------

    volatile MotorDirection _emu_dir[3]; // motor direction
    volatile unsigned _emu_speed[3];   // motor speed
    volatile unsigned BATTERY=0xFFFF; //BATTERY voltage
    volatile unsigned _emu_sensor[3];  // sensor value
    int _emu_active_flag[3];           // sensor activation flags
    int _emu_rotation_flag[3];         // rotation activation flags
    volatile int ds_rotations[3];    // rotations count
    volatile int ds_velocities[3];    // velocities
    volatile unsigned ds_muxs[3][3];    // mux vals
    volatile unsigned _emu_buttons;    // rcx buttons
    int _emu_ir=1;
  volatile unsigned _emu_mux_vals[3][3];//mux values
 
//---------------------------------------------------------------------------
// Internal variables
//---------------------------------------------------------------------------
    string lcd_buffer="-legos"; // buffer for lcd contents
    

    typedef struct {
	char host[512];
	int tcp_port;
	int hostid;
    } _emu_lnp_t;

    _emu_lnp_t _emu_lnp;
    
    void emu_lnp_init();
    void emu_lnp_shutdown();
    
//---------------------------------------------------------------------------
// Callback functions
//---------------------------------------------------------------------------
    
// Called to play beep
    void (*cb_playBeep)(void) = NULL;
    void setCallbackPlayBeep(void (*_playBeep)(void)){
	cb_playBeep=_playBeep;
    }
    
    
// Called to update execution status
    void (*updateExecStatus)(int,int) = NULL;
    void setCallbackUpdateExecStatus(void (*_Exec)(int,int)){
	updateExecStatus=_Exec;
    }
    
// Called to update sensor status on Rcx emulator
    void (*updateSensorStatus)(int) = NULL;
    void setCallbackUpdateSensorStatus( void (*_updateSensorStatus)(int) )
    {
	updateSensorStatus = _updateSensorStatus;
    }
    
// Called to update motor direction on Rcx emulator
    void (*updateMotorDir)(int, MotorDirection) = NULL;
    void setCallbackUpdateMotorDir( void (*_updateMotorDir) ( int , MotorDirection ) )
    {
	updateMotorDir = _updateMotorDir;
    }
    
// Called to update motor speed on Rcx emulator
    void (*updateMotorSpeed) ( int, unsigned char ) = NULL;
    void setCallbackUpdateMotorSpeed( void (*_updateMotorSpeed) ( int, unsigned char ) )
    {
	updateMotorSpeed = _updateMotorSpeed;
    }

// Called to update raw sensor values on Rcx emulator
    void (*updateSensorRawValues) ( int, unsigned, int ) = NULL;
    void setCallbackUpdateSensorRawValues( void (*_updateSensorRawValues) ( int, unsigned, int ) )
    {
	updateSensorRawValues = _updateSensorRawValues;
    }

// Called to update raw mux sensor values on Rcx emulator
    void (*updateMuxSensorRawValues) ( int, int, unsigned ) = NULL;
    void setCallbackUpdateMuxSensorRawValues( void (*_updateMuxSensorRawValues) ( int, int, unsigned) )
    {
	updateMuxSensorRawValues = _updateMuxSensorRawValues;
    }
    
// Called to update lcd on Rcx emulator
    void (*updateLcd) ( string ) = NULL;
    void setCallbackUpdateLcd( void (*_updateLcd) ( string ) )
    {
	updateLcd = _updateLcd;
    }
    
    
    
// Called to show some error message during program emulation
    void (*showErrorMessage) ( string ) = NULL;
    void setCallbackShowErrorMessage( void (*_showErrorMessage) ( string ) )
    {
	showErrorMessage = _showErrorMessage;
    }
    
// called to start the actually legOs program
    
    int rcx_start(int argc, char *argv[]){
	int retval;
	
	updateExecStatus(1,0);

	//this must be done in the same thead as rcx_main
	emu_lnp_init();


	retval=::rcx_main(0,NULL);
	lcd_clear();
	lcd_refresh();
	updateExecStatus(0,retval);
	
	
	// clean up legos
	dm_shutdown();
	ds_shutdown();

	emu_lnp_shutdown();


	return retval;
    }
    
//---------------------------------------------------------------------------
// Internal yield functions
//---------------------------------------------------------------------------
    
// yield the rest of the current timeslice and check for a pthread kill
    void my_sched_yield()
    {
	pthread_testcancel ();
	sched_yield();
	pthread_testcancel ();
    }
    
//---------------------------------------------------------------------------
// Motor functions
//---------------------------------------------------------------------------
    
// set motor x direction
    void motor_x_dir(int motor, MotorDirection _dir) {
	if ( motor >= 0 && motor < 3)
	{
	    _emu_dir[motor] = _dir;
	    if ( updateMotorDir != NULL )
		( updateMotorDir ) ( motor, _dir ) ;
	}
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "motor_x_dir: Invalid motor parameter" ) ;
	
	my_sched_yield();
    }
    
// set motor a direction
    void motor_a_dir(MotorDirection _dir) {
	motor_x_dir ( 0 , _dir );
    }
    
// set motor b direction
    void motor_b_dir(MotorDirection _dir) {
	motor_x_dir ( 1 , _dir );
    }
    
// set motor c direction
    void motor_c_dir(MotorDirection _dir) {
	motor_x_dir ( 2 , _dir );
    }
    
// set motor x speed
    void motor_x_speed(int motor, unsigned char _speed) {
	if ( motor >= 0 && motor < 3)
	{
	    _emu_speed[motor] = _speed;
	    if ( updateMotorSpeed != NULL )
		( updateMotorSpeed ) ( motor, _speed ) ;
	}
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "motor_x_speed: Invalid motor parameter" ) ;
	
	my_sched_yield();
    }
    
// set motor a speed
    void motor_a_speed(unsigned char _speed) {
	motor_x_speed ( 0 , _speed );
    }
    
// set motor b speed
    void motor_b_speed(unsigned char _speed) {
    motor_x_speed ( 1 , _speed );
    }
    
// set motor c speed
    void motor_c_speed(unsigned char _speed) {
	motor_x_speed ( 2 , _speed );
}
    
// initialize motors
    void dm_init(void) {
	dm_shutdown();
    }
   
#if EMU_LNP_SUPPORT 
    void emu_lnp_init() {
    //initialize IR
	if(_emu_ir) {
	    cout << "Initializing LNP"<<endl;
	    cout << "\tLNPD HOST = "<<_emu_lnp.host 
		 << " PORT = " << _emu_lnp.tcp_port << endl;
	    cout << "\tMY LNP HOST ID = "<<_emu_lnp.hostid << endl;

	    int my_lnp_addr=(_emu_lnp.hostid << 4);

	    //initialize LNP
	    if(lnp_init(_emu_lnp.host,_emu_lnp.tcp_port,my_lnp_addr,0,0))
	    {
		cout << "LNP init failed, is lnpd running?"<<endl;
		exit(1);
	    }
	    cout << "LNP initialized"<<endl;      
	}     

    }

    void emu_lnp_shutdown() {
	if(_emu_ir) {
	    lnp_shutdown();
	}
    }
#else

    void emu_lnp_init() {
    }

    void emu_lnp_shutdown() {
    }
#endif //EMU_LNP_SUPPORT

// shutdown motors
    void dm_shutdown(void) {
	motor_a_dir(off);
	motor_b_dir(off);
	motor_c_dir(off);
	motor_a_speed(MIN_SPEED);
	motor_b_speed(MIN_SPEED);
	motor_c_speed(MIN_SPEED);
    }
    
//---------------------------------------------------------------------------
// Sensor functions
//---------------------------------------------------------------------------
    
// return sensor number from sensor address
    int decode_sensor_addr(volatile unsigned* const sens) {
	if(sens==&SENSOR_1)
	    return 2;
	else if(sens==&SENSOR_2)
	    return 1;
	else if(sens==&SENSOR_3)
	    return 0;
	if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "Invalid sensor parameter" ) ;
	return 0;
    }
    
// set sensor mode to active (light sensor emits light, rotation works)
    void ds_active(volatile unsigned* const sens) {
	int s;
	s=decode_sensor_addr(sens);
	_emu_active_flag[s]=1;
	if ( updateSensorStatus != NULL )
	    ( updateSensorStatus ) ( s ) ;
	my_sched_yield();
    }
    
// set sensor mode to passive (light sensor detects ambient light)
    void ds_passive(volatile unsigned* const sens) {
	int s;
	s=decode_sensor_addr(sens);
	_emu_active_flag[s]=0;
	if ( updateSensorStatus != NULL )
	    ( updateSensorStatus ) ( s ) ;
	my_sched_yield();
    }
    
  //velocity calculator
typedef struct {
  unsigned volatile *sensor;
  int sen_idx;
  int pid;
} _emu_vel_info_t;

static _emu_vel_info_t _emu_vel_info[3];

int emu_vel_proc(int argc, char **argv) {
    _emu_vel_info_t *info=(_emu_vel_info_t*)argv;
    long int cur_tm;
    long int delta_tm;
    long int last_tm;
    int prev_rot_val;

    cur_tm=sys_time;
    ds_velocities[info->sen_idx]=0;
    prev_rot_val=ds_rotations[info->sen_idx];

    while(1) {
	last_tm=cur_tm;


	msleep(1000);
	cur_tm=sys_time;

	//speed calculations are still messed up
	//this only seem to work correctly when the
	//report rate and thate poll interval are the same

	delta_tm=cur_tm-last_tm;

	
	ds_velocities[info->sen_idx]=
	  ds_rotations[info->sen_idx]-prev_rot_val;

	prev_rot_val=ds_rotations[info->sen_idx];

   }//endless loop
}//endof emu_vel_proc



// set rotation to an absolute value
    void ds_rotation_set(volatile unsigned* const sens,int pos) {
	int s;
	s=decode_sensor_addr(sens);
	ds_rotations[s]=pos;
	for (int n_sensor = 0 ; n_sensor < 3 ; n_sensor++)
	    if ( updateSensorRawValues != NULL )
		updateSensorRawValues(n_sensor, _emu_sensor[n_sensor], ds_rotations[n_sensor]);
	my_sched_yield();
    }

// start tracking rotation sensor
    void ds_rotation_on(volatile unsigned* const sens) {
	int s,i;
	s=decode_sensor_addr(sens);
	_emu_rotation_flag[s]=1;
	if ( updateSensorStatus != NULL )
	    ( updateSensorStatus ) ( s ) ;

	//start velocity thread
	if(sens==&SENSOR_1) {
	  i=2;
	} else if(sens==&SENSOR_2) {
	  i=1;
	} else if(sens==&SENSOR_3) {
	  i=0;
	} else {
	  //error
	  return;
	}

	_emu_vel_info[i].sensor=sens;
	_emu_vel_info[i].sen_idx=i;
	_emu_vel_info[i].pid=execi(&emu_vel_proc,
				   0,
				   (char**)(&_emu_vel_info[i]),
				   PRIO_NORMAL,
				   DEFAULT_STACK_SIZE);
	

	my_sched_yield();
    }

// stop tracking rotation sensor
    void ds_rotation_off(volatile unsigned* const sens) {
	int s,i;
	s=decode_sensor_addr(sens);
	_emu_rotation_flag[s]=0;
	if ( updateSensorStatus != NULL )
	    ( updateSensorStatus ) ( s ) ;

	//kill velocity thread
	if(sens==&SENSOR_1) {
	  i=2;
	} else if(sens==&SENSOR_2) {
	  i=1;
	} else if(sens==&SENSOR_3) {
	  i=0;
	} else {
	  //error
	  return;
	}

	kill(_emu_vel_info[i].pid);
	my_sched_yield();
    }

typedef struct {
  pid_t pid;
  unsigned volatile *sensor_ptr;//sensor  
  unsigned int mux_i; //index in mux array
  unsigned int read[3];//sub ports to read (and extra timings)
} _emu_mux_data_t;

static _emu_mux_data_t _emu_mux_data[3];

#define DS_MUX_PULSE_TM_MS 10

void _emu_mux_set(unsigned volatile *sensor_ptr,
		     int mport) {

  int sen_port;
  if(sensor_ptr==&SENSOR_1) {
    sen_port=0;
  } else if(sensor_ptr==&SENSOR_2) {
    sen_port=1;
  } else if(sensor_ptr==&SENSOR_3) {
    sen_port=2;
  } else {
	return;
  }

    msleep(DS_MUX_PULSE_TM_MS*(mport+1));
    msleep(DS_MUX_PULSE_TM_MS*(mport));//one less high pulse
  emulegOsMuxSet(sen_port,mport);

}//endof _emu_mux_set

//this thread constantly reads the values from the multiplexors
int _emu_mux_thread(int argc, char **argv) {
  int mport;
  _emu_mux_data_t *data=(_emu_mux_data_t*)argv;
  ds_active(data->sensor_ptr);//activate sensor
  while(1) {
    for(mport=0;mport <3 ; mport++) {
      //logint(stdout,"port ",mport);
      if(data->read[mport]) {
	_emu_mux_set(data->sensor_ptr,mport);
	msleep(data->read[mport]);//delay the loop and let value settle
	ds_muxs[data->mux_i][mport]=*(data->sensor_ptr);
      }
    }//switch sub ports
  }//loop forever
  return 0;
}//endof _emu_mux_thread



void ds_mux_on(unsigned volatile *sensor_ptr, //the real sensor port
	       unsigned int ch1, //is there something on multiplexor port a
	       unsigned int ch2, //is there something on multiplexor port b 
	       unsigned int ch3  //is there something on multiplexor port c
		    ) {
  _emu_mux_data_t *data=NULL;
  if(sensor_ptr==&SENSOR_1) {
    data=&(_emu_mux_data[0]);
    data->mux_i=2;
  } else if(sensor_ptr==&SENSOR_2) {
    data=&(_emu_mux_data[1]);
    data->mux_i=1;
  } else if(sensor_ptr==&SENSOR_3) {
    data=&(_emu_mux_data[2]);
    data->mux_i=0;
  } else {
	return;
  }
  
  //extend time to allow virtual mux to switch
  //this is just to match real mux timing
  if(ch1)
    ch1+=160;
  if(ch2)
    ch2+=135;
  if(ch3)
    ch3+=25;
  
  data->sensor_ptr=sensor_ptr;
  data->read[0]=ch1;
  data->read[1]=ch2;
  data->read[2]=ch3;

  data->pid=execi(&_emu_mux_thread,
		  0,(char**)data,
		  PRIO_NORMAL,DEFAULT_STACK_SIZE);


  //sleep long enough for at least one reach on each port
  //otherwise our initial readings are garbage and could
  //cause troubles
  for(int j=0;j<3;j++) {
    if(data->read[j]) {
      long ms=((j+1)*2*DS_MUX_PULSE_TM_MS) /*train*/ +
			(data->read[j]) /*settle*/;
      msleep(ms);
    }
  }
  //now sleep just a bit longer for safety
  msleep(100);
}//endof ds_mux_on

void ds_mux_off(unsigned volatile *sensor_ptr) {
  _emu_mux_data_t *data=NULL;
  if(sensor_ptr==&SENSOR_1) {
    data=&(_emu_mux_data[0]);
  } else if(sensor_ptr==&SENSOR_2) {
    data=&(_emu_mux_data[1]);
  } else if(sensor_ptr==&SENSOR_3) {
    data=&(_emu_mux_data[2]);
  } else {
	return;
  }

  kill(data->pid);
  ds_passive(sensor_ptr);
}//endof ds_mux_off

 




// initialize sensors
    void ds_init(void) {
	ds_shutdown();
    }

// shutdown sensors
    void ds_shutdown(void) {
	ds_passive(&SENSOR_1);
	ds_passive(&SENSOR_2);
	ds_passive(&SENSOR_3);
	ds_rotation_off(&SENSOR_1);
	ds_rotation_off(&SENSOR_2);
	ds_rotation_off(&SENSOR_3);
    }

//---------------------------------------------------------------------------
// Button functions
//---------------------------------------------------------------------------

// return buttons state
    extern int dbutton(void) {
	my_sched_yield();
	return _emu_buttons;
    }


    int dkey() {
	int bs=dbutton();
	int dk=0;
	if(PRESSED(bs,BUTTON_ONOFF)) {
	    dk|=KEY_ONOFF;
	    //cout << "onoff"<<endl;
	}
	if(PRESSED(bs,BUTTON_VIEW)) {
	    dk|=KEY_VIEW;
	    //cout << "view"<<endl;
	}
	if(PRESSED(bs,BUTTON_PROGRAM)) {
	    dk|=KEY_PRGM;
	    //cout << "prgm"<<endl;
	}
	if(PRESSED(bs,BUTTON_RUN)) {
	    dk|=KEY_RUN;
	    //cout << "run"<<endl;
	}

	return dk;
    }

//! wakeup if any of the given keys is pressed.
//
    wakeup_t dkey_pressed(wakeup_t data) {
	return dkey() & (unsigned char)data;
    }

//! wakeup if all of the given keys are released.
//
    wakeup_t dkey_released(wakeup_t data) {
	return ! (dkey() & (unsigned char)data);
    }


    int getchar(void) {
	wait_event(dkey_released,KEY_ANY);
	wait_event(dkey_pressed ,KEY_ANY);
	return dkey();
    }


//---------------------------------------------------------------------------
// Lcd functions
//---------------------------------------------------------------------------

// put string s in lcd buffer
    void cputs(string s) {
	//first isn't updated, and last is only updated if string is long enough
	//this mimics legos behavior
	for(unsigned int i=1;i<=4;i++) {
	    if(i-1 < s.length()) {
		lcd_buffer[i]=s[i-1];
	    } else {
		lcd_buffer[i]=' ';
	    }
	}

	if(s.length()>=5) {
	    //overflow out of user space
	    lcd_buffer[5]=s[4];
	}

	lcd_refresh();
	my_sched_yield();
    }

// put an integer on lcd
    void lcd_number (int i,lcd_number_style n, lcd_comma_style c) {
	char buffer[7];
	char spec[5];
	spec[0]='%';
	spec[1]='.';

	switch (n) {
	case digit:
	case sign:
	    if(i<0)
		spec[2]='3';
	    else
		spec[2]='4';
	    spec[3]='i';
	    break;
	case unsign:
	default:
	    spec[2]='4';
	    spec[3]='u';
	    break;
	}

	spec[4]='\0';

	sprintf(buffer,spec,i);

	//currently we don't output the commas, it messes up the char array
	//on the rcx these don't take a full character space but they do on
	//emulegos, so for now they just don't exist
/*
  switch(c) {
  case digit_comma:
  //todo
  break;
  case e_1:
  buffer[4]=buffer[3];
  buffer[3]='.';
  break;
  case e_2:
  buffer[4]=buffer[3];
  buffer[3]=buffer[2];
  buffer[2]='.';
  break;
  case e_3:
  buffer[4]=buffer[3];
  buffer[3]=buffer[2];
  buffer[2]=buffer[1];
  buffer[1]='.';
  break;
  case e0:
  default:
  //whole do nothing
  break;
  }
*/
    
	for(unsigned int i=1;i<=4;i++) {
	    lcd_buffer[i]=buffer[i-1];
	}

	lcd_refresh();
	my_sched_yield();
    }

#define CPUTC_HEX(i) \
void cputc_hex_##i(unsigned char c) { \
    char buffer[7]; \
    sprintf(buffer,"%x",c); \
    lcd_buffer[5 - i ]=buffer[0]; \
    lcd_refresh(); \
    my_sched_yield(); \
}

    CPUTC_HEX(0);
    CPUTC_HEX(1);
    CPUTC_HEX(2);
    CPUTC_HEX(3);
    CPUTC_HEX(4);

    void cputc_hex_5(unsigned char c) {
	//the far right can only display a - sign ???
	if(c==0 || c==1 || c==7)
	    lcd_buffer[0]=' ';
	else
	    lcd_buffer[0]='-';
	lcd_refresh();
	my_sched_yield();
    }

    void lcd_hide(char mask) {
	cout << "EMULEGOS-NOOP lcd_hide("<<mask<<")"<<endl;
    }//endof lcd_hide

    void lcd_show(char mask) {
	cout << "EMULEGOS-NOOP lcd_show("<<mask<<")"<<endl;
    }//endof lcd_show

    void cputw(unsigned i){
	char buffer[10];
	sprintf(buffer, "%.4x", i);

	for(unsigned int i=1;i<=4;i++) {
	    lcd_buffer[i]=buffer[i-1];
	}
	lcd_refresh();
	my_sched_yield();
    }


// clear lcd
    void lcd_clear(void) {
	for(int i=0;i<=6;i++)
	    lcd_buffer[i]=' ';
	lcd_refresh();
    }

// clear user lcd
    void cls(void) {
	for(int i=0;i<=4;i++)
	    lcd_buffer[i]=' ';
	lcd_refresh();
    }


// refresh lcd
    void lcd_refresh(void) {
	if ( updateLcd != NULL )
	    ( updateLcd ) ( lcd_buffer );
	my_sched_yield();
    }

//---------------------------------------------------------------------------
// System functions
//---------------------------------------------------------------------------

#ifdef WIN32
    unsigned int msleep(unisgned int msec){
	my_sched_yield();
	return Sleep(msec);
    }
#else
// sleep msec milliseconds
    unsigned int msleep(unsigned int msec) {
	my_sched_yield();
	usleep(msec*1000);
	return 0;
    }
#endif

    int get_battery_mv() {
	long b = ds_scale(BATTERY) * 0xABD4L;
	return (int)(b / 0x618L);
    }


// sleep sec seconds
    unsigned int sleep(unsigned int sec) {
	return msleep(sec*1000);
    }

// sleep ms milliseconds
    void delay(unsigned ms) {
	msleep(ms);
    }

//---------------------------------------------------------------------------
// Time functions
//---------------------------------------------------------------------------

// return the system time in millisecs
    time_t _start_time_ms=sys_time;
    time_t sys_time_ ()
    {
	static int first_call=1;
	time_t retval;
	timeb tb;
	ftime(&tb);
	retval=(((tb.time)*1000)+tb.millitm);
	if(!first_call)
	    retval-=_start_time_ms;
	else
	    first_call=0;

	return retval;
    }

//---------------------------------------------------------------------------
// Task management functions
//---------------------------------------------------------------------------

// Task management variables
    pid_t n_proc = 0; // number of tasks
#define MAX_THREAD 100 // maximum number of concurrent tasks
    int (*rcxmain_proc)(int argc, char *argv[]); // main tasks
    typedef struct {
	int (*proc)(int argc, char *argv[]);
	int argc;
	char **argv;
	priority_t priority;
	pid_t pid;
    } procdat_t;

    procdat_t lista_proc[MAX_THREAD]; // array of tasks

    pthread_t lego_thread[MAX_THREAD];  // array of pthreads
    pthread_t lego_rcxmain_thread = 0; // main thread pid


// run the nth task
    void *legos_thread_run(void * rawdat)
    {
	procdat_t *procdat=(procdat_t*)rawdat;
	( procdat->proc ) (procdat->argc,
			   procdat->argv) ;
	lego_thread[procdat->pid]=0;//mark slot as available
	return NULL;
    }
// create a new task
    pid_t execi (int (*code_start)(int argc, char *argv[]), int argc, char **argv, priority_t priority, size_t stack_size)
    {
	//check for existing empty slot
	int i;
	for(i=0;i<n_proc;i++) {
	    if(lego_thread[i]==0) {
		break;//slot found
	    }
	}
	if(i==MAX_THREAD) {
	    cerr << "unable to allocate an additional thread, MAX is "
		 <<MAX_THREAD<<endl;
	    return -1;
	}
	if(i==n_proc) {
	    n_proc++;
	}
	lista_proc[i].proc = code_start;
	lista_proc[i].argc=argc;
	lista_proc[i].argv=argv;
	lista_proc[i].priority=priority;
	lista_proc[i].pid=i;

	pthread_create(& lego_thread[i], NULL, legos_thread_run ,&lista_proc[i] );
	return i;
    }

    void poweroff() {
	exit(0);
    }

//reimplemented from stdlib.h because unix stdlib uses a long
//and legos usses ints
    int random(void) {
	return (::random())%INT_MAX;
    }
    void srandom(int seed) {
	::srandom(seed);
    }




// run the main task
    void *legos_rcxmain_thread_run( void * )
    {
	( rcxmain_proc ) (0,NULL ) ;
	return NULL;
    }

// kill a task
    void kill(pid_t pid)
    {
	if ( pid < n_proc && lego_thread[pid]!=0) {
	    pthread_cancel( lego_thread [ pid ] );
	    lego_thread[pid]=0;//mark slot as available
	}
    }

    void killall(priority_t priority)
    {
	for(int i=0;i<n_proc;i++) {
	    if(lista_proc[n_proc].priority==priority) {
		kill(lista_proc[n_proc].pid);
	    }
	}
    }//endof killall

// wait for a particular event
    wakeup_t wait_event(wakeup_t (*wakeup)(wakeup_t),wakeup_t data)
    {
	while ( wakeup( data ) == 0 ) {
	    msleep(1);
	    my_sched_yield();
	}
	return true;
    }

//---------------------------------------------------------------------------
// Main functions
//---------------------------------------------------------------------------

// start Rcx emulation
    void emulegOsRcxEmulate( int (*rcxmain)(int argc, char *argv[] ),
			     int enable_lnp,
			     const char* lnpdhost, int lnpd_tcp_port,
			     int mylnp_hostid)
    {
	// Initialize motors, sensors and button
	dm_init();
	ds_init();

	_emu_ir=enable_lnp;

#if EMU_LNP_SUPPORT == 0
	if(_emu_ir) {
	    cerr << "LNP support was not compiled in"<<endl;
	    exit(1);
	}
#endif

	strcpy(_emu_lnp.host,lnpdhost);
	_emu_lnp.tcp_port=lnpd_tcp_port;
	_emu_lnp.hostid=mylnp_hostid;

	//_emu_buttons = BUTTON_RUN | BUTTON_PROGRAM | BUTTON_VIEW | BUTTON_ONOFF;

    // Create and execute main pthread
	rcxmain_proc = rcxmain;
	pthread_create(&lego_rcxmain_thread, NULL, legos_rcxmain_thread_run , NULL );
    }

// stop Rcx emulation
    void emulegOsRcxStop( )
    {
	// Terminate all pending thread
	for ( pid_t cont = n_proc - 1 ; cont >= 0 ; cont-- ) {
	    kill ( cont ) ;
	}
	n_proc = 0;

	// Terminate main thread
	pthread_cancel( lego_rcxmain_thread );

	// clean up legos
	dm_shutdown();
	ds_shutdown();

	emu_lnp_shutdown();

    }

//---------------------------------------------------------------------------
// External world functions
//---------------------------------------------------------------------------

// Return motor x direction
    MotorDirection emulegOsGetMotorDir( int motor )
    {
	if (motor >= 0 && motor <= 2)
	    return _emu_dir[ motor ];
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsGetMotorDir: Invalid motor parameter" ) ;
	return off;
    }

// Return motor x speed
    unsigned char emulegOsGetMotorSpeed( int motor )
    {
	if (motor >= 0 && motor <= 2)
	    return (unsigned char) _emu_speed[ motor ];
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsGetMotorSpeed: Invalid motor parameter" ) ;
	return 0;
    }

// Set sensor x value
    void emulegOsSetSensor ( int _sensor, unsigned value )
    {
	if (_sensor >= 0 && _sensor <= 2)
	{                                                                            
	    _emu_sensor[ _sensor ] = value ;
	    if ( updateSensorRawValues != NULL )
		updateSensorRawValues(_sensor, _emu_sensor[_sensor], ds_rotations[_sensor]);
	}                                                                            
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsSetSensor: Invalid sensor parameter" ) ;
    }

// Set sensor x value
    void emulegOsSetMuxSensor ( int port, int sub, unsigned value )
    {
	if (port>= 0 && port <= 2)
	{                                                                            
	    _emu_mux_vals[2-port][2-sub] = value ;
	    if ( updateMuxSensorRawValues != NULL )
		updateMuxSensorRawValues(port,sub,  _emu_mux_vals[port][sub]);
	}                                                                            
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsSetMuxSensor: Invalid sensor parameter" ) ;
    }//endof emulegOsSetMuxSensor

  //copy value from mux sub port to main port
  void emulegOsMuxSet(int port, int sub) {
	if (port >= 0 && port <= 2)
	{                                                                            
	    emulegOsSetSensor(2-port,_emu_mux_vals[2-port][2-sub]) ;
	                                                                     
	} else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsMuxSet: Invalid sensor parameter" ) ;

  }//emulegOsMuxSet




// Set rotation sensor x value
    void emulegOsRotateSensor ( int _sensor , int rots)
    {
	if (_sensor >= 0 && _sensor <= 2)
	{                                                                            
	    ds_rotations[ _sensor ] += rots ;                                               
	    if ( updateSensorRawValues != NULL )                                         
		updateSensorRawValues(_sensor, _emu_sensor[_sensor], ds_rotations[_sensor]);     
	}
	else if ( showErrorMessage != NULL )
	    ( showErrorMessage ) ( "emulegOsRotateSensor: Invalid sensor parameter" ) ;
    }


#if EMU_LNP_SUPPORT
//IR functions
    int _emu_lnp_far=0;
    void lnp_logical_range(int far) {
	_emu_lnp_far=far;
    }

    int lnp_logical_range_is_far() {
	return _emu_lnp_far;
    }

    static pthread_t _emu_lnp_thread;
    pthread_mutex_t _emu_lnp_lock;
    pthread_mutex_t _emu_lnp_cmd_lock;
    pthread_mutex_t _emu_lnp_result_lock;
 
    class _emu_lnp_data_t {
    public:
	//lnp command
	enum {init,shutdown,int_hand,add_hand,int_write,add_write} cmd;
	//lnp_init
	lnp_init_result result_init;
	char *tcp_hostname;
	unsigned short tcp_port;
	unsigned char lnp_address;
	unsigned char lnp_mask;
	int flags;
	//lnp_integrity_set_handler
	lnp_integrity_handler_t integrity_handler;
	//lnp_addressing_set_handler
	unsigned char port;
	lnp_addressing_handler_t addressing_handler;
	//lnp_integrity_write
	lnp_tx_result result_tx;
	const unsigned char *data;
	unsigned char length;
	//lnp_addressing_write
	unsigned char dest;
	unsigned char srcport;
    };	

    static _emu_lnp_data_t _emu_lnp_data;

    void* emu_lnp_thread_func (void *) {
	
	while(1) {
	    //wait for lnp command request
	    //cout << "lnp wait on lock"<<endl;
	    pthread_mutex_lock(&_emu_lnp_cmd_lock);
	    
	    //cout << "lnp make call"<<endl;
	    switch(_emu_lnp_data.cmd) {
	    case _emu_lnp_data_t::init:
		_emu_lnp_data.result_init=(lnp_init_result)
		    ::lnp_init(_emu_lnp_data.tcp_hostname,
			       _emu_lnp_data.tcp_port,
			       _emu_lnp_data.lnp_address,
			       _emu_lnp_data.lnp_mask,
			       _emu_lnp_data.flags);
		break;
	    case _emu_lnp_data_t::shutdown:
		::lnp_shutdown();
	    break;
	    case _emu_lnp_data_t::int_hand:
		::lnp_integrity_set_handler(_emu_lnp_data.integrity_handler);
	    break;
	    case _emu_lnp_data_t::add_hand:
		::lnp_addressing_set_handler(_emu_lnp_data.port,
					     _emu_lnp_data.addressing_handler);
		break;
	    case _emu_lnp_data_t::int_write:
		_emu_lnp_data.result_tx=(lnp_tx_result)
		    ::lnp_integrity_write(_emu_lnp_data.data,
					  _emu_lnp_data.length);
	    break;
	    case _emu_lnp_data_t::add_write:
		_emu_lnp_data.result_tx=(lnp_tx_result)
		    ::lnp_addressing_write(_emu_lnp_data.data,
					   _emu_lnp_data.length,
					   _emu_lnp_data.dest,
					   _emu_lnp_data.srcport);
	    break;
	    default:
		cerr << "Unknown LNP function"<<endl;
		exit(1);
	    }

	    //signal that result (if any) is ready
	    pthread_mutex_unlock(&_emu_lnp_result_lock);
	}
	

    }//endof emu_lnp_thread_func

    lnp_init_result lnp_init(
	char *tcp_hostname,
	unsigned short tcp_port,
	unsigned char lnp_address,
	unsigned char lnp_mask,
	int flags) {

	pthread_mutex_init(&_emu_lnp_lock,NULL);
	pthread_mutex_init(&_emu_lnp_cmd_lock,NULL);
	pthread_mutex_init(&_emu_lnp_result_lock,NULL);

	pthread_mutex_lock(&_emu_lnp_cmd_lock);
	pthread_mutex_unlock(&_emu_lnp_lock);
	pthread_mutex_unlock(&_emu_lnp_result_lock);

	//start lnp thread
	pthread_create(&_emu_lnp_thread, NULL, emu_lnp_thread_func,NULL);
	

	//start lnp_init call
	pthread_mutex_lock(&_emu_lnp_lock);

	//call ::lnp_init from lnp thread
	_emu_lnp_data.cmd=_emu_lnp_data_t::init;
	_emu_lnp_data.tcp_hostname=tcp_hostname;
	_emu_lnp_data.tcp_port=tcp_port;
	_emu_lnp_data.lnp_address=lnp_address;
	_emu_lnp_data.lnp_mask=lnp_mask;
	_emu_lnp_data.flags=flags;

	//signal that request is ready
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);
	
	//wait for request to return
	pthread_mutex_lock(&_emu_lnp_result_lock);
	//copy result
	lnp_init_result result=_emu_lnp_data.result_init;
	pthread_mutex_unlock(&_emu_lnp_result_lock);

	

	//signal that call is over
	pthread_mutex_unlock(&_emu_lnp_lock);
	
	return result;
    }//endof lnp_init

    void lnp_shutdown(void) {
	pthread_mutex_lock(&_emu_lnp_lock);

	//setup call
	_emu_lnp_data.cmd=_emu_lnp_data_t::shutdown;

	//make call
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);


	//wait for call to finish
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_result_lock);

	//kill thread
	pthread_cancel(_emu_lnp_thread);
	

	pthread_mutex_unlock(&_emu_lnp_lock);

	//destroy mutexes
	pthread_mutex_destroy(&_emu_lnp_lock);
	pthread_mutex_destroy(&_emu_lnp_cmd_lock);
	pthread_mutex_destroy(&_emu_lnp_result_lock);


    }
    void lnp_integrity_set_handler(lnp_integrity_handler_t handler) {
	pthread_mutex_lock(&_emu_lnp_lock);

	_emu_lnp_data.cmd=_emu_lnp_data_t::int_hand;
	_emu_lnp_data.integrity_handler=handler;

	//make call
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);


	//wait for call to finish
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_result_lock);


	pthread_mutex_unlock(&_emu_lnp_lock);
    }
    void lnp_addressing_set_handler(unsigned char port,
				    lnp_addressing_handler_t handler) {
	pthread_mutex_lock(&_emu_lnp_lock);

	_emu_lnp_data.cmd=_emu_lnp_data_t::add_hand;
	_emu_lnp_data.addressing_handler=handler;
	_emu_lnp_data.port=port;

	//make call
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);


	//wait for call to finish
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_result_lock);


	pthread_mutex_unlock(&_emu_lnp_lock);
    }
    lnp_tx_result lnp_integrity_write(const unsigned char *data,
				      unsigned char length) {
	pthread_mutex_lock(&_emu_lnp_lock);

	_emu_lnp_data.cmd=_emu_lnp_data_t::int_write;
	_emu_lnp_data.data=data;
	_emu_lnp_data.length=length;

	//make call
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);


	//wait for call to finish
	pthread_mutex_lock(&_emu_lnp_result_lock);
	lnp_tx_result result=_emu_lnp_data.result_tx;
	pthread_mutex_unlock(&_emu_lnp_result_lock);


	pthread_mutex_unlock(&_emu_lnp_lock);
	return result;
    }
    lnp_tx_result lnp_addressing_write(const unsigned char *data,
				       unsigned char length,
				       unsigned char dest,
				       unsigned char srcport) {
	pthread_mutex_lock(&_emu_lnp_lock);
	_emu_lnp_data.cmd=_emu_lnp_data_t::add_write;
	_emu_lnp_data.data=data;
	_emu_lnp_data.length=length;
	_emu_lnp_data.dest=dest;
	_emu_lnp_data.srcport=srcport;

	//make call
	pthread_mutex_lock(&_emu_lnp_result_lock);
	pthread_mutex_unlock(&_emu_lnp_cmd_lock);


	//wait for call to finish
	pthread_mutex_lock(&_emu_lnp_result_lock);
	lnp_tx_result result=_emu_lnp_data.result_tx;
	pthread_mutex_unlock(&_emu_lnp_result_lock);


	pthread_mutex_unlock(&_emu_lnp_lock);
	return result;
    }
    
#endif //EMU_LNP_SUPPORT



//sound stuff

    unsigned dsound_16th_ms=DSOUND_DEFAULT_16th_ms;
    unsigned dsound_internote_ms=DSOUND_DEFAULT_internote_ms;
    long int _emu_dsound_finish_ms=0;

    static const note_t sys_beep[]={
	{PITCH_A4 , 1}, {PITCH_END, 0}
    };

//! system sound data
    const note_t *dsound_system_sounds[]={
	sys_beep
	    };

    void dsound_play(const note_t *notes) {
	//currently we just determine how long it would take to play
	//the requested notes, but we play nothing
	long int runtime=0;
	while(notes->pitch!=PITCH_END) {
	    runtime+=(dsound_16th_ms*notes->length);
	    notes++;
	    if(notes->pitch!=PITCH_END)
		runtime+=dsound_internote_ms;
	}
	/*cout << "dsound has limited support: fake sound will finish in "
      <<runtime<<"ms"<<endl;*/
	if(runtime>0)
	    (cb_playBeep)();//at least we get some sound

	_emu_dsound_finish_ms=sys_time+runtime;
    }//endof dsound_play

    int dsound_playing(void) {
	return _emu_dsound_finish_ms < sys_time;
    }//endof dsound_playing

    void dsound_system(unsigned nr) {
	if(nr<DSOUND_SYS_MAX)
	    dsound_play(dsound_system_sounds[nr]);
    }

    void dsound_set_duration(unsigned duration) {
	dsound_16th_ms=duration;
    }

    void dsound_set_internote(unsigned duration) {
	dsound_internote_ms=duration;
    }

    wakeup_t dsound_finished(wakeup_t data) {
	return !dsound_playing();
    }

    void dsound_stop(void) {
	_emu_dsound_finish_ms=sys_time;
    }


}//end emulegos namespace


