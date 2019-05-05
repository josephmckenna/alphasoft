#!/bin/sh


topagana() {
    conv=$(echo "scale=6; 10^-6 * `getconf PAGESIZE`" | bc )
    printf '%-8s %-40s %-6s %-10s %-10s %-10s\n' "PID" "Directory" "Run" "Status" "Time" "Mem. Usage [MB]"
    count=0
    for pid in `pgrep agana`; do
	run=$(echo `cat /proc/$pid/cmdline` | grep -o '[0-9]\{4,5\}' | head -1)
	mem=$(awk '{ print $1 }' /proc/$pid/statm)
	meu=$(echo "scale=1; $conv * $mem" | bc)
	printf '%-8s %-40s %-6s %-10s %-10s %-8s\n' $pid `readlink -f /proc/$pid/cwd` `expr $run + 0` `cat /proc/$pid/syscall` `ps -o etime= -p $pid` "$meu"
	count=$((count+1))
    done
    echo -e "\n\n\n\nNumber of processes:" $count
}

export -f topagana
watch -x bash -c topagana

#topagana
