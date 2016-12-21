# Octane Overview
libuv is a cross-platform  asynchronous I/O library that only uses a single event loop thread for TCP traffic.

Octane is a high performance library that extends libuv to scale and use all the available CPU cores on a system for TCP traffic by using multiple event loops across multiple threads.

# Optimizations
- Prefers a thread count that matches physical CPU cores excluding hyper-threaded cores.
- TCP connections pinned to physical CPU cores for low context switching and high L1/L2/L3 cache hits.
- Zero-copy optimizations.
- High performance memory allocators.
  - A high performance lockless memory allocator is available for `Linux` to scale to higher CPU core counts to reduce CPU contention and stalls between CPU cores.

# Supported Platforms
Platforms supported:
- Linux
- macOS

Windows could be supported pretty easily with a little work.

# Compiling
Compiling `octane` and a `hello_world` HTTP server example.
```
make
```

Compiling benchmarking tools.
```
make tools
```

# Memory Allocators
You can use `LD_PRELOAD` to use different memory allocators with octane.

TCMalloc
```
LD_PRELOAD="./lib/tcmalloc/.libs/libtcmalloc_minimal.so" ./build/hello_world
```

Lockless
```
LD_PRELOAD="./lib/lockless_allocator/libllalloc.so.1.3" ./build/hello_world
```


# Raw I/O Benchmarks
Benchmarking raw HTTP request/response throughput we can reach over `17 million requests/second` using `2% CPU usage` in userland. Keep in mind this benchmark isn't parsing HTTP requests. It's purely measuring the request/response I/O performance.

```
./lib/wrk/wrk --script ./benchmark/pipelined.lua --latency -d 60s -t 20 -c 512 http://IP:80 -- 64
Running 1m test @ http://IP:80
  20 threads and 512 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     5.40ms    6.19ms 296.15ms   88.31%
    Req/Sec     0.86M   134.44k    1.71M    74.11%
  Latency Distribution
     50%    3.46ms
     75%    7.14ms
     90%   12.52ms
     99%   28.41ms
  1,025,596,200 requests in 1.00m, 129.90GB read
Requests/sec: 17,056,890.50
Transfer/sec:      2.16GB
```

```
----total-cpu-usage---- -dsk/total- -net/total- ---paging-- ---system--
usr sys idl wai hiq siq| read  writ| recv  send|  in   out | int   csw
  1   0  99   0   0   0| 472B  410k|   0     0 |   0     0 |6046   538
  2  28  51   0   0  19|   0     0 | 228M 2336M|   0     0 | 359k  349k
  2  27  50   0   0  22|   0     0 | 224M 2250M|   0     0 | 344k  265k
  2  27  49   0   0  22|   0     0 | 227M 2294M|   0     0 | 345k  279k
  2  29  46   0   0  23|   0  2048B| 240M 2343M|   0     0 | 345k  283k
  2  26  46   0   0  25|   0   162k| 231M 2335M|   0     0 | 353k  258k
  2  26  45   0   0  27|   0     0 | 236M 2340M|   0     0 | 362k  274k
  2  27  45   0   0  26|   0     0 | 242M 2341M|   0     0 | 354k  281k
  2  26  45   0   0  27|   0     0 | 233M 2337M|   0     0 | 365k  269k
  2  28  46   0   0  25|   0     0 | 229M 2341M|   0     0 | 356k  303k
```