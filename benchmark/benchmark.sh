#!/bin/bash
../lib/wrk/wrk --script ./pipelined.lua --latency -d 10s -t 4 -c 32 http://127.0.0.1:8000 -- 128
