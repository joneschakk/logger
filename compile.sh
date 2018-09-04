#!/bin/bash

set -e
#MESOS_VERSION="${MESOS_VERSION:-1.3.0}"
cmake -DMESOS_SRC_DIR="/home/jones/mesos"
make -j 6 V=0
