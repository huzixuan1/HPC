Updated: 2024/08/15

Purpose:
    This example demonstrates how to run CVE CustomBinary on devices

Build (on PC):
    Build CustomBinary
    $ source /<mvpu_deployment_dir>/mvpu_env.sourceme
    $ ./run.sh
    Build application (test_CVE_host_api)
    $ source /<mvpu_deployment_dir>/mvpu_env.sourceme
    $ export YOUR_NDK_PATH={Please download anroid-ndk first, see Makefile}
    $ make

Execution (on device):
    $ run_CVE_host_api.bat
