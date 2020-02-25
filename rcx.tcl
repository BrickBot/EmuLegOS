


source $env(EMULEGOS_ROOT)/util.tcl

#event callback function


proc RCX_UpdateExecStatus {status main_retval} {
    global emu_RCX
    #puts "Status = $status $main_retval"
    set emu_RCX(running) $status

    if ![expr $status] {
	

	emu_RCX_Stop
    }
}

proc RCX_UpdateMotorDir { n_motor dir} {
    #puts "UpdateMotorDir $n_motor $dir"
    global emu_RCX motor_direction_text motor_id_text
    set motor_text $motor_id_text($n_motor)
    set emu_RCX(motor_${motor_text}_direction) $dir
    set emu_RCX(motor_${motor_text}_direction_text) $motor_direction_text($dir)

    foreach cb $emu_RCX(motor_on_dir) {
	$cb $motor_text $emu_RCX(motor_${motor_text}_direction_text)
    }

    return
}

proc RCX_UpdateMotorSpeed { n_motor speed} {
    #puts "UpdateMotorSpeed $n_motor $speed"
    global emu_RCX motor_id_text
    set motor_text $motor_id_text($n_motor)
    set emu_RCX(motor_${motor_text}_speed) [format "%d" $speed]

    foreach cb $emu_RCX(motor_on_speed) {
	if [catch {
	    $cb $motor_text [format "%d" $speed]
	} ex] {
	    puts stderr $ex
	}
    }

  
    return
}


proc RCX_UpdateSensorRawValues { n_sensor value rotation} {
    #puts "UpdateSensorRawValues $n_sensor $value $rotation"
    global emu_RCX sensor_id_text
    set sensor_text [format "%d" [expr $n_sensor +1]]
    set emu_RCX(sensor_${sensor_text}_value_raw) $value
    set emu_RCX(sensor_${sensor_text}_value_rotation) [format "%d" $rotation]
    return
}

proc RCX_UpdateMuxSensorRawValues { n_sensor n_sub value} {
    #puts "UpdateMuxSensorRawValues $n_sensor $n_sub $value"
    global emu_RCX sensor_id_text
    set sensor_num [format "%d" [expr $n_sensor +1]]
    if [expr $n_sub == 0] {
	set sensor_text ${sensor_num}a
    } else if [expr $n_sub == 1] {
	set sensor_text ${sensor_num}b
    } else if [expr $n_sub == 2] {
	set sensor_text ${sensor_num}c
    } else {
	error "unknown sub port"
    }
    
    set emu_RCX(sensor_${sensor_text}_value_raw) $value
    return
}

proc RCX_UpdateSensorStatus {n_sensor status} {
    #puts "UpdateSensorStatus $n_sensor $status"
    global emu_RCX sensor_status_text

    set sensor_text [format "%d" [expr $n_sensor +1]]
    set status [format "0x%x" $status]
    set emu_RCX(sensor_${sensor_text}_status) $status
    set emu_RCX(sensor_${sensor_text}_status_text) $sensor_status_text($status)

    return
}

proc RCX_UpdateLcd {{strLcdBuffer {}}} {
    #puts "UpdateLcd \{$strLcdBuffer\}"
    global emu_RCX
    set emu_RCX(LCD) $strLcdBuffer
    return
}

proc RCX_ErrorMsg {{errMsg {}}} {
    puts $stderr $errMsg
    after 0 $errMsg
    return
}


proc RCX_SetCmplt {addr data} {
    emu_RCX_SetCmplt [format %d $addr] $data
    return
}


#controls

proc RCX_SetSensorValue {sensor value} {
    #puts "RCX_SetSensorValue $sensor $value"

    global emu_RCX
	
    if [expr [lsearch -exact $emu_RCX(valid_sensors) $sensor] == -1] {
	error SensorIdOutOfRange
    }
    #I'm not getting updates from the callback so I'll update manually
    set emu_RCX(sensor_${sensor}_value_raw) $value

    
    if [expr [lsearch -exact $emu_RCX(real_sensors) $sensor] == -1] {
	set x [string range $sensor 1 1]
	if {$x == {a}} {
	    set sub 0
	} elseif {$x == {b}} {
	    set sub 1
	} elseif {$x == {c}} {
	    set sub 2
	} else {
	    error "invalid sub mux port $x"
	}
	set sen [string range $sensor 0 0]
	emu_RCX_SetMuxSensorValue [expr $sen -1] $sub [format "%d" $value]
    } else {
	emu_RCX_SetSensorValue [expr $sensor -1] [format "%d" $value]
    }

    return
};#endof RCX_SetSensorValue


proc RCX_IncrSensorValue {sensor {value {}}} {
    #puts "RCX_IncrSensorValue $sensor $value"
    global emu_RCX
    
    #check range
    if [expr [lsearch -exact $emu_RCX(valid_sensors) $sensor] == -1] {
	error SensorIdOutOfRange
    }
    #check if value was supplied
    if {$value == {}} {
	#use default
	set value $emu_RCX(sensor_${sensor}_increment_raw)
    }
    #set new value
 
    RCX_SetSensorValue $sensor [expr $emu_RCX(sensor_${sensor}_value_raw) + $value]
    return
};#endof RCX_IncrSensorValue

proc RCX_DcrSensorValue {sensor {value {}}} {
    #puts "RCX_DcrSensorValue $sensor $value"
    if {$value == {}} {
	global emu_RCX
	set value $emu_RCX(sensor_${sensor}_increment_raw)
    }

    RCX_IncrSensorValue $sensor -$value
    
    return
};#endof RCX_DcrSensorValue

proc ds_scale {x} {
    return [expr $x >> 6]
}

proc ds_unscale {x} {
    return [expr $x << 6]
}

#Light sensor conversion
proc RawToLight {raw} {  
    return [expr 147 - ([ds_scale $raw]/7)]
}

proc LightToRaw {light} {
    set s1 [expr 147 - $light]
    set s2 [expr $s1 * 7]
    return [ds_unscale $s2]
}

proc RCX_SetLightValue {sen value} {
    RCX_SetSensorValue $sen [LightToRaw $value]
    return
}

proc RCX_GetLightValue {sen value} {
    return [RawToLight [RCX_GetSensorValue $sen]]
}

proc SwmuxToRaw {a b c d} {
    global emu_RCX
    
    if [expr $d] {
	return $emu_RCX(sensor_swmux_XXX1)
    } else {
	return $emu_RCX(sensor_swmux_$a$b$c$d)
    }
    
}

proc RCX_SetSwmuxValue {sen button state} {
    global emu_RCX
    set emu_RCX(multi_touch_${sen}_${button}) $state
    set a $emu_RCX(multi_touch_${sen}_a)
    set b $emu_RCX(multi_touch_${sen}_b)
    set c $emu_RCX(multi_touch_${sen}_c)
    set d $emu_RCX(multi_touch_${sen}_d)

    RCX_SetSensorValue $sen [SwmuxToRaw $a $b $c $d] 
    return
}

proc RCX_SetSwmuxValues {sen a b c d} {
    global emu_RCX
    foreach but {a b c d} {
	set emu_RCX(multi_touch_${sen}_${but}) [set $but]
    }
    RCX_SetSensorValue $sen [SwmuxToRaw $a $b $c $d] 
    return
}

proc DirpdToRaw {state} {
    global emu_RCX
    if {$state != {L} && $state != {R} && $state != {C} && $state != {none}} {
	throw "invalid Dirpd state value $state"
    }
    return $emu_RCX(sensor_dirpd_${state})
}

proc RCX_SetDirpdValue {sen state} {
    RCX_SetSensorValue $sen [DirpdToRaw $state]
    return
}




proc RCX_IncrRotationValue {sensor {value {}}} {
    #puts "RCX_IncrRotationValue $sensor $value"
    global emu_RCX
    
    #check range
    if [expr [lsearch -exact $emu_RCX(real_sensors) $sensor] == -1] {
	error SensorIdOutOfRange
    }
    #check if value was supplied
    if {$value == {}} {
	#use default
	set value $emu_RCX(sensor_${sensor}_increment_rotation)
    }
    #set new value
    emu_RCX_SetRotationValue [expr $sensor -1] $value
    return
};#endof RCX_IncrRotationValue


proc RCX_DcrRotationValue {sensor {value {}}} {
	global emu_RCX
    #check range
    if [expr [lsearch -exact $emu_RCX(real_sensors) $sensor] == -1] {
	error SensorIdOutOfRange
    }

    #puts "RCX_DcrRotationValue $sensor $value"
    if {$value == {}} {
	global emu_RCX
	set value $emu_RCX(sensor_${sensor}_increment_rotation)
    }

    RCX_IncrRotationValue $sensor -$value
    
    return
};#endof RCX_DcrRotationValue


proc velocity_motor_update {motor} {
    #puts "velocity_motor_update $motor"
    global emu_velocity emu_RCX MAX_SPEED MIN_SPEED

    if {$motor == {unbound}} {
	return
    }
    
    set motor_speed $emu_RCX(motor_${motor}_speed)
    set dir $emu_RCX(motor_${motor}_direction_text)

    foreach sen {1 2 3} {
	if ![info exists emu_velocity(sensor_${sen}_bind)] {
	    continue
	}

	if {$emu_velocity(sensor_${sen}_bind) == $motor} {

	    set mot_velocity_percent [expr double($motor_speed) / \
				      ($MAX_SPEED - $MIN_SPEED)]
	    set rot_velocity [expr int($emu_velocity(velocity_max) * $mot_velocity_percent)]


	    #motor bound to this velocity sensor
	    if {$dir == {fwd}} {
		if [expr $emu_velocity(sensor_${sen}_direction) < 0] {
		    set rot_velocity [expr 0 - $rot_velocity]
		}
		set emu_velocity(sensor_${sen}_value) $rot_velocity
	    } elseif {$dir == {rev}} {
		if [expr $emu_velocity(sensor_${sen}_direction) > 0] {
		    set rot_velocity [expr 0 - $rot_velocity]
		}
		set emu_velocity(sensor_${sen}_value) $rot_velocity
	    } else {
		#off or break
		set emu_velocity(sensor_${sen}_value) 0
	    }

	    velocity_rot_updater_mod $sen
	}

    }

};#endof velocity_motor_update

proc velocity_motor_update_dir {motor dir} {
    #puts "$motor dir is $dir"
    velocity_motor_update $motor
};#endof velocity_motor_update_dir

proc velocity_motor_update_velocity {motor velocity} {
    #puts "$motor velocity is $velocity"
    velocity_motor_update $motor
};#endof velocity_motor_update_velocity



proc velocity_set {sen value} {
    global emu_velocity

    set emu_velocity(sensor_${sen}_value) $value

    velocity_rot_updater_mod $sen
};#endof velocity_set


proc velocity_dcr {sen} {
    global emu_velocity
    incr emu_velocity(sensor_${sen}_value) -$emu_velocity(sensor_${sen}_direction)

    velocity_rot_updater_mod $sen
};#endof velocity_dcr 

proc velocity_incr {sen} {
    global emu_velocity
    incr emu_velocity(sensor_${sen}_value) $emu_velocity(sensor_${sen}_direction)

    velocity_rot_updater_mod $sen
};#endof velocity_incr

proc velocity_rot_updater_mod {sen} {
    global emu_velocity

    #cancel existing updater
    if [info exists emu_velocity(sensor_${sen}_updater)] {
	after cancel $emu_velocity(sensor_${sen}_updater)
    }

    if [expr $emu_velocity(sensor_${sen}_value) == 0] {
	#no updater needed
	return
    }
    
    set update_ms [expr abs(1000 / $emu_velocity(sensor_${sen}_value))]
  
    set emu_velocity(sensor_${sen}_updater) \
	[after $update_ms velocity_rot_updater $sen]

    return
};#endof velocity_rot_updater_mod

proc velocity_rot_updater {sen} {
    global emu_velocity

    set myvelocity $emu_velocity(sensor_${sen}_value)

    #determine update interval
    if [expr $myvelocity == 0] {
	return
    } 


    if [expr $emu_velocity(sensor_${sen}_direction) > 0] {
	if [expr $myvelocity> 0] {
	    RCX_IncrRotationValue $sen
	} else {
	    RCX_DcrRotationValue $sen
	}	
    } else {
	if [expr $myvelocity > 0] {
	    RCX_DcrRotationValue $sen
	} else {
	    RCX_IncrRotationValue $sen
	}
    }
    
    set update_ms [expr abs(1000 / $emu_velocity(sensor_${sen}_value))]
  
    set emu_velocity(sensor_${sen}_updater) \
	    [after $update_ms velocity_rot_updater $sen]
    
    return

};#endof velocity_rot_updater


proc RCX_GetSensorValue {sensor} {
    #puts "RCX_GetSensorValue $sensor"
    if [expr ($sensor < 1) || ($sensor >3)] {
	error SensorIdOutOfRange
    }
    set value [emu_RCX_GetSensorValue [expr $sensor -1]]
    return $value
}    

proc RCX_GetRotationValue {sensor} {
    #puts "RCX_GetSensorValue $sensor"
    if [expr ($sensor < 1) || ($sensor >3)] {
	error SensorIdOutOfRange
    }
    set value [emu_RCX_GetRotationValue [expr $sensor -1]]
    return $value
}  


#buttons


proc RCX_PrgmDown {} {
    global emu_RCX BUTTON_PROGRAM
    
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) & ~$BUTTON_PROGRAM]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_PrgmDown

proc RCX_PrgmUp {} {
    global emu_RCX BUTTON_PROGRAM
    
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) | $BUTTON_PROGRAM]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_PrgmUp

proc RCX_Prgm {} {
    RCX_PrgmDown
    RCX_PrgmUp
}

proc RCX_ViewDown {} {
    global emu_RCX BUTTON_VIEW
    #puts "ViewDown"
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) & ~$BUTTON_VIEW]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_ViewDown

proc RCX_ViewUp {} {
    global emu_RCX BUTTON_VIEW
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) | $BUTTON_VIEW]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_ViewUp

proc RCX_View {} {
    RCX_ViewDown
    RCX_ViewUp
}

proc RCX_OnOffDown {} {
    global emu_RCX BUTTON_ONOFF
    

    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) & ~$BUTTON_ONOFF]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    exit
};#endof RCX_OnOffDown

proc RCX_OnOffUp {} {
    global emu_RCX BUTTON_ONOFF
    
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) | $BUTTON_ONOFF]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_OnOffUp

proc RCX_OnOff {} {
    RCX_OnOffDown
    RCX_OnOffUp
}

proc RCX_RunDown {} {
    global emu_RCX BUTTON_RUN
    
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) & ~$BUTTON_RUN]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)

    if ![expr $emu_RCX(running)] {
	#puts "RCX_Start"
	global ir_enabled ir_serverhost ir_serverport ir_lnp_id 
	emu_RCX_Start $ir_enabled $ir_serverhost \
	    [format %d $ir_serverport] [format %d $ir_lnp_id]
    } else {
	#puts "RCX Stop"
	foreach stopper $emu_RCX(on_stop) {
	    eval $stopper
	}
	emu_RCX_Stop
	set emu_RCX(running) 0
	RCX_UpdateLcd
    }

    return
};#endof RCX_RunDown

proc RCX_RunUp {} {
    global emu_RCX BUTTON_RUN
    
    set emu_RCX(buttonStates) [expr $emu_RCX(buttonStates) | $BUTTON_RUN]
    
    emu_RCX_SetButtonStates $emu_RCX(buttonStates)
    return
};#endof RCX_RunUp

proc RCX_Run {} {
    RCX_RunDown
    RCX_RunUp
}

#endof RCX button procedures

proc RCX_Init {} {
    emu_RCX_Init
    return
}




#get initial values
foreach sen {1 2 3} {
    set emu_RCX(sensor_${sen}_value_raw) [RCX_GetSensorValue $sen]
    set emu_RCX(sensor_${sen}_value_rotation) [RCX_GetRotationValue $sen]

    #multiplexor init
    foreach sub {a b c} {
	set emu_RCX(sensor_${sen}${sub}_value_raw) 0x0
    }
}





set BUTTON_ONOFF 0x02
set BUTTON_RUN 0x04
set BUTTON_VIEW 0x4000
set BUTTON_PROGRAM 0x8000
set TOUCH_ON 0x0
set TOUCH_OFF 0xFFFF
set LIGHT_RAW_BLACK 0xffc0
set LIGHT_RAW_WHITE 0x5080
set MIN_SPEED 0
set MAX_SPEED 255

set ir_serverhost localhost
set ir_serverport 0x1E60
set ir_lnp_id 8
set ir_enabled 0

set emu_RCX(valid_sensors) {
	1 1a 1b 1c
	2 2a 2b 2c
	3 3a 3b 3c
}
set emu_RCX(real_sensors) {1 2 3}

#controls how much to increment the raw sensor values
foreach sen $emu_RCX(valid_sensors) {
    set emu_RCX(sensor_${sen}_increment_raw) 1
    set emu_RCX(sensor_${sen}_increment_rotation) 1
}


#register motor velocity and direction call backs
lappend emu_RCX(motor_on_dir) velocity_motor_update_dir
lappend emu_RCX(motor_on_speed) velocity_motor_update_velocity


set emu_RCX(sensor_light_max) 100
set emu_RCX(sensor_light_min) 0

set emu_velocity(velocity_max) 50
set emu_velocity(velocity_min) 0



set emu_RCX(sensor_swmux_0000) 0xfe00
set emu_RCX(sensor_swmux_1000) 0x9640
set emu_RCX(sensor_swmux_1100) 0x8180
set emu_RCX(sensor_swmux_1010) 0x8d80
set emu_RCX(sensor_swmux_1110) 0x7800
set emu_RCX(sensor_swmux_0100) 0xc000
set emu_RCX(sensor_swmux_0110) 0xa6c0
set emu_RCX(sensor_swmux_0010) 0xd6c0
set emu_RCX(sensor_swmux_XXX1) 0x0600

set emu_RCX(sensor_dirpd_L) 0xdec0
set emu_RCX(sensor_dirpd_R) 0xb700
set emu_RCX(sensor_dirpd_C) 0xffc0
set emu_RCX(sensor_dirpd_none) 0x8d40


set emu_RCX(motor_a_velocity) 0
set emu_RCX(motor_b_velocity) 0
set emu_RCX(motor_c_velocity) 0


#functions to call on RCX stop
set emu_RCX(on_stop) {}

#initialize all buttons to down position
set emu_RCX(buttonStates) [expr $BUTTON_RUN | $BUTTON_VIEW | $BUTTON_ONOFF | $BUTTON_PROGRAM]
set emu_RCX(running) 0

array set motor_id_text {0x0 a 0x1 b 0x2 c}
array set motor_direction_text {0x0 off 0x1 fwd 0x2 rev 0x3 brake}
array set sensor_status_text {0x0 passive 0x1 active}

