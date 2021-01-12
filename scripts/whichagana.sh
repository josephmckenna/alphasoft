#!/bin/sh


topagana() {
    conv=$(echo "scale=6; 10^-6 * `getconf PAGESIZE`" | bc )
    printf '%-8s %-40s %-8s %-10s %-10s %-10s\n' "PID" "Directory" "Run" "Status" "Time" "Mem. Usage [MB]"
    count=0
    for pid in `pgrep ^ag[a-zA-Z]\{3,\}.exe`; do       
	dir=$(readlink -f /proc/$pid/cwd)

	run=$(echo `cat /proc/$pid/cmdline` | grep -o '[0-9]\{4,6\}' | head -1)
	runno=$(expr $run + 0)

	stat=$(cat /proc/$pid/syscall | head | cut -d" " -f1)

	clock=$(ps -o etime= -p $pid | tr -d '[:space:]')

	mem=$(awk '{ print $1 }' /proc/$pid/statm)
	meu=$(echo "scale=1; $conv * $mem" | bc)

	printf '%-8s %-40s %-8s %-10s %-10s %-10s\n' "$pid" "$dir" "$runno" "$stat" "$clock" "$meu"
	count=$((count+1))
    done
    echo -e "\n\n\n\nNumber of processes:" $count
}

export -f topagana
watch -n 1 -x bash -c topagana
