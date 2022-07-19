# rss2mp3

Command-line utility for downloading podcasts.

To use:

```
$ ./run

(build output trimmed)

[h for help] h
- h: help (this text)
- a: add a feed
- d: delete a feed
- l: list feeds
- u: update feeds
- q: quit
[h for help] 
```

The primary use case for this tool is running inside Termux.  Users not
attempting to run on a phone will probably appreciate the TUI client
[castero](https://github.com/xgi/castero) more than this project.

We depend on libcurl and libxml2.  If not present, these can be installed
with:

```
$ apt install libxml2 libcurl
```

or the appropriate equivalent for your distro.
