#!/bin/sh


topagana() {
    conv=$(echo "10^-6 * `getconf PAGESIZE`" | bc -l)
    #echo "PID     directory                    run   status  elapsed time    mem usage"
    printf '%-8s %-33s %-6s %-10s %-10s %-10s\n' "PID" "Directory" "Run" "Status" "Elaps. T." "Mem. Usage [MB]"
    for pid in `pgrep agana.exe`; do
	run=$(echo `cat /proc/$pid/cmdline` | grep -o '[0-9]\{4,5\}' | head -1)
	mem=$(awk '{ print $1 }' /proc/$pid/statm)
	mem=$(echo "$conv * $mem" | bc -l)
	meu="$mem"
	#echo $pid " " `readlink -f /proc/$pid/cwd` "R`expr $run + 0`" `cat /proc/$pid/syscall` `ps -o etime= -p $pid` "      " `printf "%1.3fMB\n" $mem`
	#echo $pid " " `readlink -f /proc/$pid/cwd` "R`expr $run + 0`" `cat /proc/$pid/syscall` `ps -o etime= -p $pid` "      " $mem
	printf '%-8s %-33s %-6s %-10s %-10s %-8s\n' $pid `readlink -f /proc/$pid/cwd` `expr $run + 0` `cat /proc/$pid/syscall` `ps -o etime= -p $pid` $meu
    done
}


export -f topagana
watch -x bash -c topagana

#topagana
