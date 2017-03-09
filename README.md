Initializes a full-screen display which OpenGL ES2 graphics can be drawn using a WebGL-compliant interface.

Supported targets:

Raspberry PI 1/2/3
Use Raspbian or make sure that includes and libs are in /opt/vc.
Raspbian has a default GPU memory setting of 64M, which is quite low. It may lead to 0x0505 (out of memory) errors.
You can increase this to a higher number using raspi-config.
Also, for best performance, please set the resolution to 720p in raspi-config.

X11 (Linux)
Installation: install packages libx11-dev and libgles2-dev.

In the future, we may add support for other platforms.