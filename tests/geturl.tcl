#!/aitools/bin/tclsh
package require http

set query  {Opr_Login aiobs Opr_Passwd aiobs}
set query  [eval "::http::formatQuery $query"] 
if 0 {
set url http://10.1.8.240:2222/pub/chk_login.vet
set fd [::http::geturl $url -query $query ]
set mydata [::http::data $fd]
puts mydata=$mydata
}

puts -------------------------------


set url http://e3:8106/aiobs4/ext/adm-docs/pub/chk_login.ktcl
set fd [::http::geturl $url -query $query ]
set mydata [::http::data $fd]
puts mydata=$mydata


