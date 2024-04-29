# core_samples
Collect core dump for Segmentation fault .

## set core_pattern  

```
echo ‘/var/log/%e.core.%p’ > /proc/sys/kernel/core_pattern
```
or  
```
std::string cmd = "sysctl -w kernel.core_pattern=/home/li/log/%e.core.%p.
%t";
system(cmd.c_str());
```   
## set core limit

```
ulimit -c 1024
```
or
```
struct rlimit rlim;
getrlimit(RLIMIT_CORE,&rlim);
printf("cur:%lu, max:%lu \n",rlim.rlim_cur,rlim.rlim_max);

rlim.rlim_cur = 100 * 1024 * 1024;
rlim.rlim_max = 100 * 1024 * 1024;
setrlimit(RLIMIT_CORE,&rlim);
```