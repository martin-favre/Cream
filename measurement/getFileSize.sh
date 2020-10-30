#prints filesize in bytes of arg 1 
val=$(stat --printf="%s" $1)
echo $val