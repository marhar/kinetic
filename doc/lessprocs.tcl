proc nbsp {x} {
    set r ""
    for {set i 0} {$i < $x} {incr i} {
        append r "&nbsp;&nbsp;&nbsp;&nbsp;"
    }
    return $r
}
proc 4. {str} { puts "<h4>[nbsp 3]$str</h4>" }
proc ul {str} {
    puts "<ul>"
    foreach s [split $str \n] {
        if {[string length $s] > 0} {
            puts "<li>$s"
        }
    }
    puts "</ul>"
}
proc 1. {str} { puts "<hr><h1>[nbsp 0]$str</h1>" }
proc 2. {str} { puts "<h2>[nbsp 1]$str</h2>" }
proc 3. {str} { puts "<h3>[nbsp 2]$str</h3>" }
proc assignment {title text} { puts "<h3>Assignment $title</h3>"; puts $text }
