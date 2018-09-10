# mesos-custom-logger

custom logging module for mesos with enhanced feature set.


## prerequisites

* **compiler**

    (tested only in gcc 4.8.5)
 
    should support c++11 standard  
    
    Same compiler used to build mesos is recommented to avoid any strange 
    failures that may occure in runtime.

* **cmake** 

    Any version > 2 (cmake3 is recommended)

*  **devel packages**

    - mesos and its 3rdparty dependencies like glog, libprocess etc

        (dynamically linked to libmesos)
            
            if mesos is installed in build machine, above dependency will be satisfied provided that the compiler used is same as configured. 
            Else chances are to break in runtime. (due to linked libc++ difference.)
    
    - libcurl and headers
        > `$ yum install libcurl libcurl-devel`
    - 


## build

> `$ ./bake.sh`

        targets will be at ./build directory


## notes
---
in mesos 1.3.1 with gcc version 4.8.5 20150623 (Red Hat 4.8.5-28) (GCC) 
symbol `svn_txdelta2` was shown to be undefined in `libmesos.so` in compainion-binary link stage.

since this symbol `svn_txdelta2` doesn not seem to be used in runtime, error is suppressed with  `-warn-unresolved-symbols` flag.

    /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../libmesos.so: warning: undefined reference to `svn_txdelta2'

---


## docs


### sample module parameters

```json
{
    "libraries": [{
        "file": "/opt/mesos/custom_logger/liblogship_container_logger.so",
        "modules": [{
 
            "name": "com_flytxt_mesos_LogShipContainerLogger",
 
            "parameters": [{
                    "key": "max_stdout_size",
                    "value": "50MB"
                },
                {
                    "key": "logrotate_stdout_options",
                    "value": "rotate 9"
                },
                {
                    "key": "max_stderr_size",
                    "value": "50MB"
                },
                {
                    "key": "logrotate_stderr_options",
                    "value": "rotate 9"
                },
                {
                    "key": "companion_binary_path",
                    "value": "/opt/mesos/custom_logger/logger-companion"
                },
                {
                    "key": "log_path_template",
                    "value": "{framworkId}/{executorId}/{executorStartTime}/"
                },
                {
                    "key": "time_stamp_format",
                    "value": "%d-%b-%Y_%H.%M.%S"
                },
                {
                    "key": "exclude_executorid_regex",
                    "value": "(?:^ct.*)|(?:[0-9]+)"
                },
                {
                    "key": "log_ship_base_directory",
                    "value": "/opt/mesos-logs"
                }
            ]
        }]
    }]
 }
```

### log_path_template supported keys
* frameworkId
* executorId
* executorStartTime