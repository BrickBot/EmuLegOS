#utility functions

#check a list of supplied parameters
#a list of key,value pairs will be returned
#returns error if reqired are missing, or supplied are not recognized
#supplying only -? will return help
#optional is key value pairs, where the value will be assigned if it
#isn't in the provided section
#switches is a list of supported switches which don't take arguments
#if a switches are always in the returned list, they have a value of 1 if
#they were found in the provided list, and 0 otherwise
proc checkParms {procName provided required {optional {}} {switches {}}} {
    #create help message

    set optlist {}
    array set opts $optional
    foreach opt [array names opts] {
	set optlist [concat $optlist "?${opt} arg?"]
    }
    
    set reqlist {}
    foreach req [array names required] {
	set reqlist [concat $reqlist "${req} arg"]
    }

    set switchlist {}
    foreach switch $switches {
	set switchlist [concat $switchlist "?${switch}?"]
    }

    set help [concat $procName $reqlist $optlist $switchlist]

    if {$provided == {-?}} {
	error $help
    }

    #set up a list of all valid keys
    set validKeys [concat $required [array names opts] $switches]

    #initialize result, to have all optional values, and all switches
    #set to 0

    array set result $optional
    
    foreach switch $switches {
	set result($switch) 0
    }


    #scan provided to make sure all keys are valid
    set top [llength $provided]

    for {set i 0} {[expr $i < $top]} {incr i} {
	set key [lindex $provided $i]
	
	#make sure that the set of valid keys
	
	if [expr [lsearch -exact $validKeys $key] == -1] {
	    #key not valid return error
	    error "$key not a valid key in $help"
	}
	
	#key is valid, determine if it is a switch key
	if [expr [lsearch -exact $switches $key] != -1] {
	    #it's a switch key, don't increment index
	    #next element in list should be a key
	    #store a 1 for the value of this key
	    set value 1
	} else {
	    #not a switch key, auto incrment index to skip over value
	    incr i
	    #make sure index is less then length of list
	    if [expr $i >= $top] {
		#there is no value
		error "no value supplied for $key in $help"
	    }
	    set value [lindex $provided $i]
	}
	set result($key) $value

    }


    #check that all required parms are supplied
    foreach parm $required {
	if ![info exists result($parm)] {
	    error "$parm is a required parameter in $help"
	}
    }
    
 
    #return result
    return [array get result]
}


#build a lookup table of ascii values

for {set val 0} {[expr $val <= 255]} {incr val} {
    set int2ascii($val) [format %c $val]
    set ascii2int([format %c $val]) $val
}

proc string2bytes {string} {
    global ascii2int

    #seperate string into a list

    set charlist [split $string {}]
    
    set bytelist {}
    foreach char $charlist {
	lappend bytelist [format 0x%x $ascii2int($char)]
    }
    return $bytelist
}

proc bytes2string {bytelist} {

    set string {}
    foreach byte $bytelist {
	set string ${string}[format %c $byte]
    }

    return $string
}
