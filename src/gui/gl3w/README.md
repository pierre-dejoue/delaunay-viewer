GL3W
====


[Gl3w](https://github.com/skaslev/gl3w) is an OpenGL loader. It loads pointers to OpenGL functions at runtime to enable the use of modern OpenGL in the program. Usually, the OpenGL functions made available by the OS are limited to antiquated versions of the standard (e.g. `GL/gl.h` on Windows is for version 1.1 of OpenGL (1997)). More info [here](https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library).

Gl3w is [UNLICENSED](https://github.com/skaslev/gl3w/blob/master/UNLICENSE). Alternatives to Gl3w are glad, glew (with an 'e'), to name a few.

## Code Generation

The files in this folder were generated with the following command line:

```
python gl3w_gen.py --root ./generated
```

Doing so I first ran into a certificate issue discussed below.

## SSL Certificate Issue

In my environment (I'm using Git bash on Windows) I got the following error while running the script:

```
urllib.error.URLError: <urlopen error [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: certificate has expired (_ssl.c:1002)>
```

The way to solve it was to edit the script to use a different SSL context:

```
import certifi
import ssl

...

certifi_context = ssl.create_default_context(cafile=certifi.where())
web = urllib2.urlopen(urllib2.Request(url, headers={'User-Agent': 'Mozilla/5.0'}), context=certifi_context)
```

## Other Changes

- Add a generation date in files `gl3w.h` and `gl3w.c`: `/* Generated: 2023-08-15 */`
- Replace tabs by 4 spaces
