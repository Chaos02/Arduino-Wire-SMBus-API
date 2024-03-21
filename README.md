# Arduino Wire-SMBus API

This "library" has been implemented according to the [official SMBus v3.1 specification](http://smbus.org/specs/SMBus_3_1_20180319.pdf).

It provides functions to interface, setup and manage an SMBus network on an Arduino by mimic-ing the official Wire library (which is used in the backend after some transformation etc.).

I do not suggest you just try to use this as it is **unfinished** and **yet to be confirmed to actually work**, however feel free to share your own development by opening a pull request.
The project I wanted to use this for ultimately got scrapped before I could test it.
I was planning on using this to interface with a smart-battery and do some system management+monitoring on an Arduino.
