//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------

#include "emuLegOs.h"
#include <tcl.h>

void registerCallBacks();

#define SchedCallback(cb) Tcl_CreateTimerHandler(0,Tcl_TimerHanlder,(ClientData)cb)

void Tcl_TimerHanlder(ClientData clientdata);

class tcl_callback {
public:
  char *tclcmd;
  void (*callback)(int, void*);
  void *data;
  tcl_callback(int cmdsize=0,void (*_cb) ( int, void*)=NULL,void *d=NULL){
    if(cmdsize)
      tclcmd=(char*)malloc(cmdsize);
    else
      tclcmd=NULL;
    callback=_cb;
    data=d;
  }

  ~tcl_callback(){
    if(tclcmd)
      free(tclcmd);
  }
};

#endif

