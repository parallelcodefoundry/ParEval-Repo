#!/bin/bash

if ! strace -e openat "$@" 2>&1 | grep -i nvidia ; then echo 'No GPU kernels launched!!' ; fi
