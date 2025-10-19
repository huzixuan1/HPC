@echo off

adb root
adb remount

:: push test_CVE_host_api and data folder
:: test_CVE_host_api:是host的执行程序
:: libmcve.so是host执行的依赖的共享库
:: 推送数据文件 --sync表示只同步变化的文件 避免重复传输
:: 送 kernel 二进制文件，并改名为 CustomKernel.bin
adb push .\test_CVE_host_api vendor/bin/
adb push .\..\lib64\libmcve.so /data/local/tmp/
adb push --sync .\..\cve_data\ /data/local/tmp/
adb push CustomKernel_device.bin /data/local/tmp/CustomKernel.bin

:: run
:: xport LD_LIBRARY_PATH=/data/local/tmp/:$LD_LIBRARY_PATH:把 /data/local/tmp/ 加入共享库搜索路径，保证程序能找到 libmcve.so
:: cd /data/local/tmp/; → 切换工作目录到 /data/local/tmp/
:: taskset 70 test_CVE_host_api → 在 指定 CPU 核心（十六进制 0x70 → CPU 4,5,6）上运行 host 程序
adb shell "export LD_LIBRARY_PATH=/data/local/tmp/:$LD_LIBRARY_PATH; cd /data/local/tmp/; taskset 70 test_CVE_host_api"

pause
