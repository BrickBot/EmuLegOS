//---------------------------------------------------------------------------

#include <string>
#include <iostream>
#ifdef WIN32
#pragma hdrstop
#endif

#include "Main.h"
#include "RealWorld.h"
#include <tcl.h>
//---------------------------------------------------------------------------
#ifdef WIN32
#pragma package(smart_init)
#endif

extern Tcl_Interp *tcl_interp;


// you must always allocate this dynamically
//it will be freed when RCX_Cmplt is called from Tcl
class tcl_cmplt {
  enum status_t {waiting,ready,done};

  status_t status;
  const char *data;

public:
  void setStatusReady(){status=ready;}
  void waitStatusReady(){while(status==waiting){msleep(10);}}
  void setStatusDone(){status=done;}
  void waitStatusDone(){while(status==ready){msleep(10);}}
  void setData(const char *_data){data=_data;}
  const char *getData(){return data;}

  tcl_cmplt(){
    status=waiting;
    data=NULL;
  }
};

//Tcl TimerHandler, this is used to perform tcl callback from multiple threads
//simply calling Tcl_Eval from within a thread is not safe, instead the
//thread should set a timer for 0ms, and this handler will be called when it 
//is safe to call the function.
void Tcl_TimerHanlder(ClientData clientdata){
  tcl_callback *cmd=(tcl_callback*)clientdata;
  int result=TCL_OK;
  //evaluate tcl command
  if(cmd->tclcmd) {
    //cout << "Executing TclCmd "<<cmd->tclcmd<<endl;
    if ((result=Tcl_Eval(tcl_interp,cmd->tclcmd))!=TCL_OK) {
      cerr << "Tcl Error in "<<cmd->tclcmd<<endl;
    }
  }


  if(cmd->callback){
    cmd->callback(result,cmd->data);
  }

  delete cmd;
}//endof Tcl_TimerHandler



void UpdateExecStatus(int status,int main_retval) {

  tcl_callback *cb=new tcl_callback(100);
  
  sprintf(cb->tclcmd,"RCX_UpdateExecStatus 0x%x 0x%x",status,main_retval);

  SchedCallback(cb);

}//endof UpdateExecStatus


void PlayBeep() {

  tcl_callback *cb=new tcl_callback(100);
  
  sprintf(cb->tclcmd,"bell");

  SchedCallback(cb);

}//endof PlayBeep



void UpdateMotorDir ( int n_motor, MotorDirection dir )
{
  //cout << "UpdateMotorDir"<<endl;
  //cout << n_motor << ' '<<dir<<endl;

  tcl_callback *cb=new tcl_callback(100);
  
  sprintf(cb->tclcmd,"RCX_UpdateMotorDir 0x%x 0x%x",n_motor,dir);

  SchedCallback(cb);

}

void UpdateMotorSpeed ( int n_motor, unsigned char speed )
{
  //cout << "UpdateMotorSpeed"<<endl;
  //cout << n_motor<<' '<<speed<<endl;
  tcl_callback *cb=new tcl_callback(100);

  sprintf(cb->tclcmd,"RCX_UpdateMotorSpeed 0x%x 0x%x",n_motor,speed);
  
  SchedCallback(cb);

}

// Sensor functions
// ----------------
void UpdateSensorRawValues ( int n_sensor, unsigned value, int rotation )
{
  //cout << "UpdateSensorRawValues"<<endl;
  //cout << n_sensor<<' '<<rotation<<endl;

  tcl_callback *cb=new tcl_callback(100);

  sprintf(cb->tclcmd,"RCX_UpdateSensorRawValues 0x%x 0x%x 0x%x",2-n_sensor,value,rotation);
  
  SchedCallback(cb);


}

void UpdateMuxSensorRawValues ( int n_sensor, int sub, unsigned value )
{
  //cout << "UpdateMuxSensorRawValues"<<endl;

  tcl_callback *cb=new tcl_callback(100);

  sprintf(cb->tclcmd,"RCX_UpdateMuxSensorRawValues 0x%x 0x%x 0x%x",
	  2-n_sensor,2-sub,value);
  
  SchedCallback(cb);


}

void UpdateSensorStatus(int s)
{
  //cout << "UpdateSensorStatus"<<endl;
  //cout << s<<endl;

  tcl_callback *cb=new tcl_callback(100);

  sprintf(cb->tclcmd,"RCX_UpdateSensorStatus 0x%x 0x%x",2-s,_emu_active_flag[s]);
  
  SchedCallback(cb);


}

// Lcd functions
// -------------
void UpdateLcd(string strLcdBuffer)
{
  static char lastBuffer[10]="";
  char str[10];
  unsigned int i;

  //stupid way of copying the string, I don't know how to use this string class
  for(i=0;i<strLcdBuffer.length();i++)
    str[i]=strLcdBuffer[i];
  str[i]=0;

  if(!strcmp(str,lastBuffer))//don't do an update if the string hasn't changed
    return;

  strcpy(lastBuffer,str);

  tcl_callback *cb=new tcl_callback(100);

  sprintf(cb->tclcmd,"RCX_UpdateLcd {%s}",str);
  
  SchedCallback(cb);


}




//c functions called from tcl

int ChangeSensorValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  int value = atoi(argv[2]);
  //  cout << "change "<<sensor_id<<' '<<value<<endl;
  _emu_sensor[sensor_id]=value;
  return TCL_OK;
}

int ChangeMuxSensorValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  int sub_id = 2-atoi(argv[2]);
  int value = atoi(argv[3]);
  //  cout << "change "<<sensor_id<<' '<<value<<endl;
  _emu_mux_vals[sensor_id][sub_id]=value;
  return TCL_OK;
}


int ChangeRotationValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  int value = atoi(argv[2]);
  emulegOsRotateSensor(sensor_id,value);
  return TCL_OK;
}


int GetRotationValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  char resp[100];
  sprintf(resp,"0x%x",ds_rotations[sensor_id]);
  Tcl_SetResult(tcl_interp,resp,TCL_VOLATILE);    
  return TCL_OK;
}

int GetSensorValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  char resp[100];
  sprintf(resp,"0x%x",_emu_sensor[sensor_id]);
  Tcl_SetResult(tcl_interp,resp,TCL_VOLATILE);  
  return TCL_OK;
}

int GetMuxSensorValue(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int sensor_id = 2-atoi(argv[1]);
  int sub_id = 2-atoi(argv[2]);
  char resp[100];
  sprintf(resp,"0x%x",_emu_mux_vals[sensor_id][sub_id]);
  Tcl_SetResult(tcl_interp,resp,TCL_VOLATILE);  
  return TCL_OK;
}


int GetMotorDir(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int motor_id = atoi(argv[1]);
  char resp[100];
  sprintf(resp,"0x%x",emulegOsGetMotorDir(motor_id));
  Tcl_SetResult(tcl_interp,resp,TCL_VOLATILE);
  return TCL_OK;
}

int GetMotorSpeed(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  int motor_id = atoi(argv[1]);
  char resp[100];
  sprintf(resp,"0x%x",emulegOsGetMotorSpeed(motor_id));
  Tcl_SetResult(tcl_interp,resp,TCL_VOLATILE);  
  return TCL_OK;
}

int Start(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  // set up the "real world" emulation
  RealWorldInit();

  if(argc!=5) {
      cerr << "not enough parameters to start" <<endl;
      exit(1);
  }

  int enable_lnp=atoi(argv[1]);
  const char* lnpdhost=argv[2];
  int lnpd_tcp_port=atoi(argv[3]);
  int mylnp_hostId=atoi(argv[4]);


  emulegOsRcxEmulate(rcx_start, enable_lnp, lnpdhost, lnpd_tcp_port,mylnp_hostId);
  return TCL_OK;
}


int Init(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  // Reset everything
  dm_init();
  ds_init();
  lcd_clear();
  cputs("emu");
  lcd_refresh();

  return TCL_OK;
}

int Stop(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  emulegOsRcxStop();
  return TCL_OK;
}

int SetButtonStates(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  _emu_buttons=atoi(argv[1]);
  return TCL_OK;
}

int RCX_SetCmplt(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]){
  tcl_cmplt *cmplt=(tcl_cmplt*)atoi(argv[1]);
  //cout << "RCX_SetCmplt "<<argv[1]<<" invoked "<<argv[2]<<endl;
  cmplt->setData(argv[2]);

  //set to ready
  cmplt->setStatusReady();

  //wait for done
  cmplt->waitStatusDone();
  delete cmplt;

  return TCL_OK;
}

void registerCallBacks(){

    setCallbackPlayBeep(PlayBeep);
  setCallbackUpdateExecStatus(UpdateExecStatus);
  setCallbackUpdateMotorDir ( UpdateMotorDir );
  setCallbackUpdateMotorSpeed ( UpdateMotorSpeed );
  setCallbackUpdateSensorStatus ( UpdateSensorStatus );
  setCallbackUpdateSensorRawValues ( UpdateSensorRawValues );
  setCallbackUpdateMuxSensorRawValues ( UpdateMuxSensorRawValues );
  setCallbackUpdateLcd ( UpdateLcd );
  //  setCallbackErrorMessage ( handleErrorMsg );

  Tcl_CreateCommand(tcl_interp, "emu_RCX_SetSensorValue", (Tcl_CmdProc *)ChangeSensorValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
  Tcl_CreateCommand(tcl_interp, "emu_RCX_SetMuxSensorValue", (Tcl_CmdProc *)ChangeMuxSensorValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

 Tcl_CreateCommand(tcl_interp, "emu_RCX_Start", (Tcl_CmdProc *)Start, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
 Tcl_CreateCommand(tcl_interp, "emu_RCX_Stop", (Tcl_CmdProc *)Stop, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
 Tcl_CreateCommand(tcl_interp, "emu_RCX_SetButtonStates", (Tcl_CmdProc *)SetButtonStates, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

 Tcl_CreateCommand(tcl_interp, "emu_RCX_SetRotationValue", (Tcl_CmdProc *)ChangeRotationValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

 Tcl_CreateCommand(tcl_interp, "emu_RCX_GetRotationValue", (Tcl_CmdProc *)GetRotationValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
 Tcl_CreateCommand(tcl_interp, "emu_RCX_GetSensorValue", (Tcl_CmdProc *)GetSensorValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
 Tcl_CreateCommand(tcl_interp, "emu_RCX_GetMuxSensorValue", (Tcl_CmdProc *)GetMuxSensorValue, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

Tcl_CreateCommand(tcl_interp, "emu_RCX_GetMotorDir", (Tcl_CmdProc *)GetMotorDir, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
Tcl_CreateCommand(tcl_interp, "emu_RCX_GetMotorSpeed", (Tcl_CmdProc *)GetMotorSpeed, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
Tcl_CreateCommand(tcl_interp, "emu_RCX_Init", (Tcl_CmdProc *)Init, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

Tcl_CreateCommand(tcl_interp, "emu_RCX_SetCmplt", (Tcl_CmdProc *)RCX_SetCmplt, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

}
