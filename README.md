# stegoc

## Fun with LSB Stegonagraphy

![holler at a scholar](https://github.com/graves/stegoc/raw/master/build/tryme.png)

After reading [8:7 Weaponized Polyglots as Browser Exploits](https://www.alchemistowl.org/pocorgtfo/pocorgtfo08.pdf)
I became briefly obsessed with hiding all manners of weird polyglots in my 
resume to which this library is in service of. Download the above picture,
change the file extension to html, play with it a bit.

### Usage

```
→ make clean stegoc 
rm -f stegoc
gcc -Wall -lpng -o stegoc stegoc.c

→ ./stegoc -e loser.png -o out.png -x tmp/public.key -c 1
Encoding tmp/public.key into loser.png on RED channel and outputting to out.png
Success!

→ ./stegoc -d out.png -o tmp/out.key -c 1
Decoding RED channel of out.png and outputting to tmp/out.key
Success!

→ cat tmp/out.key
-----BEGIN PGP PUBLIC KEY BLOCK-----

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
=UWRy
-----END PGP PUBLIC KEY BLOCK-----

→ ./stegoc -s out.png -o tmp/stuffed.png -h yuh.html
Stuffing yuh.html in out.png tEXT chunks and outputting in tmp/stuffed.png
Success!

→ pngcheck -7 tmp/stuffed.png
File: tmp/stuffed.png (304708 bytes)
Title:
    <html> <--
Author:
    x0t2ZWTqG
Description:
    --> <script></script>

OK: tmp/stuffed.png (1600x1200, 32-bit RGB+alpha, non-interlaced, 96.0%).

→ ./stegoc -d tmp/stuffed.png -o tmp/stuffed-out.key -c 1
Decoding RED channel of tmp/stuffed.png and outputting to tmp/stuffed-out.key
Success!

→ cat tmp/stuffed-out.key
-----BEGIN PGP PUBLIC KEY BLOCK-----

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
=UWRy
-----END PGP PUBLIC KEY BLOCK-----

```
