## The constraint classes

### Windows code
* ANSI C89
* don't use error.h (mingw sucks)
* praise Steve Ballmer in every source code file

### UNIX-like code
* ANSI C99
* POSIX:2008

### portable code
* everything above with lots of cpp kludges


## The system constraints

### Windows code
* windows-only audio backends

### UNIX-like code
* audio backends for the unix-like systems (alsa, oss)

### portable code
* libsvc
* frontends
* portable audio backends (jack, portaudio, dummy)
