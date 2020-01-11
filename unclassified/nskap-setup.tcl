#!/bin/sh
# the next line restarts using tclsh \
exec tclsh8.0 "$0" "$@"
#
# This file is part of KAP, the Kinetic Application Processor.
# Copyright (c) 1999, AsiaInfo Technologies, Inc.
#
# See the file kap-license.txt for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.  Share and Enjoy!
#
# The KAP home page is: http://www.markharrison.net/kap
#

#Id: $Id: nskap-setup.tcl,v 1.1 1999/11/08 09:55:57 markh Exp $

proc debug args {
    eval "puts $args"
}

proc getinput {name default desc} {
    puts "$desc"
    set confirm "n"
    set answer $default
    while {! [regexp -nocase "^y" $confirm]} {
        puts -nonewline "$name \[$answer\]: "
        flush stdout
        gets stdin newvalue
        #debug "newvalues -- $newvalue"
        if {[string compare $newvalue ""] != 0} {
            set answer $newvalue
        }
        if [file writable $answer] {
            puts -nonewline "Are you sure? \[y\]: "
            flush stdout
            gets stdin confirm
            if {[string compare $confirm ""] == 0} {
                set confirm "y"
            }
        } else {
            puts "$answer doesn't exist or isn't writable.\n"
        }
    }
    puts ""
    return $answer
}

proc chkver ns_home {
    set fd [open "|strings $ns_home/../bin/https/ns-httpd"]
    set junk1 0
    while {[gets $fd s] >= 0} {
        if {[regexp {^Netscape-Enterprise/(.*)$} $s foo ver]} {
            if {$junk1 == 0} {
                puts "The Netscape Enterprise Server version: $ver"
            }
            set junk1 1
        }
    }
    if {[lindex [split $ver "."] 0] < 3} {
        puts stderr "The Netscape Enterprise Server version must be 3.0 or above!!!"
        flush stderr
        exit 1
    }
    close $fd
}

proc backup_old orgf {
    global time_str
    append bakf $orgf . $time_str
    file copy -- $orgf $bakf
    file delete -force $orgf
    return $bakf
}
        
proc cfg_obj {bakf newf} {
    global obj_in
    if [catch {set obj_fd [open $bakf r]} err_msg] {
        puts stderr $err_msg
        exit -1
    }

    set line_num 0
    while {[gets $obj_fd fileline($line_num)] >= 0} {
        #debug "$line_num --- $fileline($line_num)"
	#skip early config.
        if [regexp -nocase "kap" $fileline($line_num)] {
            continue
        }

	#find edit point
        if [regexp -- {^<Object name=\"*default\"*} $fileline($line_num)] {
            set line_tmp $fileline($line_num)
            incr line_num -1
	    #eliminate space lines
            while {[regexp -- {^[         ]*$} $fileline($line_num)]} {
                incr line_num -1
            }
            incr line_num
            set fileline($line_num) $obj_in(init)
            incr line_num
            set fileline($line_num) $line_tmp
        }
	#find edit point
        if [regexp -- {^ObjectType fn="*force-type"* type="*text/plain"*} $fileline($line_num)] {
            incr line_num
            set fileline($line_num) $obj_in(service)
            incr line_num
	    #eliminate space lines
            while {[gets $obj_fd fileline($line_num)] >= 0} {
                if {[regexp -- {^[ 	]*$} $fileline($line_num)] == 0} {
                    break
                }
                incr line_num
            }
        }
        incr line_num
    }
    #write new file
    set fd_new [open $newf w]
    for {set i 0} {$i < $line_num} {incr i} {
        puts $fd_new $fileline($i)
    }
    close $fd_new
    close $obj_fd
}

proc cfg_mime {bakf newf} {
    global mime_in
    if [catch {set mime_fd [open $bakf r]} err_msg] {
        puts stderr $err_msg
        exit -1
    }

    set line_num 0
    while {[gets $mime_fd fileline($line_num)] >= 0} {
	#skip early config.
        if [regexp -nocase "ktcl" $fileline($line_num)] {
            continue
        }
        incr line_num
    }
    set fileline($line_num) $mime_in
    incr line_num
    #write new file
    set fd_new [open $newf w]
    for {set i 0} {$i < $line_num} {incr i} {
        puts $fd_new $fileline($i)
    }
    close $fd_new
    close $mime_fd

}

#Main starts

set product "Kinetic Application Server"
set hostname [info hostname]
set engnum 4
set time_str [clock format [clock second] -format %Y%m%d%H%M%S]

switch -- $argc {
    2 {
        set ns_home [lindex $argv 0]
        #debug $ns_home
        set prod_home [lindex $argv 1]
        #debug $prod_home
    }

    0 {
        set ns_desc "Please entry the location of Netscape Enterprise Server."
        append ns_home "/usr/local/suitespot/https-" [lindex [split $hostname "."] 0]
        set ns_home [getinput "Netscape Enterprise Server home" $ns_home $ns_desc]
        #debug $ns_home

        set prod_home [pwd]
        set prod_desc "Please enter the location where $product will reside."
        set prod_home [getinput "$product home" $prod_home $prod_desc]
        #debug $prod_home

        puts "Please enter th  number of the engines at the backend."
        puts -nonewline "Number of engines \[$engnum\]: "
        flush stdout
        gets stdin engnum
        if {[string compare $engnum ""] != 0} {
            set engnum  $engnum
        } else  {
	set engnum 4
	}
	
        #debug $engnum
    }

    default {
        puts stderr "Usage: $argv0 \[ns-home kap-home\]"
        exit -1
    }
}

set obj_f $ns_home/config/obj.conf
set mime_f $ns_home/config/mime.types
set obj_in(init0) "#------- Initialize AI KAP -------"
set obj_in(init1) "Init fn=\"load-modules\" shlib=\"$prod_home/kap-ns.so\"  funcs=\"kap_init,kap_conn\""
set obj_in(init2) "Init fn=\"kap_init\" engine_path=\"$prod_home/kap-t\" max_pipes=\"$engnum\""
set obj_in(init3) "#------- End of Initializing AI KAP -------"
append obj_in(init) "\n" $obj_in(init0) "\n" $obj_in(init1) "\n" $obj_in(init2) "\n" $obj_in(init3) "\n"


set obj_in(srvc0) "#------- Start of AI KAP Service -------"
set obj_in(srvc1) "Service fn=\"kap_conn\" method=\"(GET|POST)\" type=\"magnus-internal/ktcl\""
set obj_in(srvc2) "#------- End of AI KAP Service -------"
append obj_in(service) "\n" $obj_in(srvc0) "\n" $obj_in(srvc1) "\n" $obj_in(srvc2) "\n"

set mime_in "type=magnus-internal/ktcl     exts=ktcl"

#chkver $ns_home
set bak_obj [backup_old $obj_f]
set bak_mime [backup_old $mime_f]
#debug "--- $bak_obj --- $obj_f"
#debug "--- $bak_mime --- $mime_f"
cfg_obj $bak_obj $obj_f
cfg_mime $bak_mime $mime_f
puts "\n$product installation finished."

