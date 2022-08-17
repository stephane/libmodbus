Please read the following carefully before submitting this new issue.

- Please ensure, that you are really reporting a bug. When in doubt, post a
  message on <https://groups.google.com/forum/#!forum/libmodbus> or send an
  email to libmodbus@googlegroups.com

- Please do not open issues to ask questions about using libmodbus. Use the
  mailing list for this as there are many more people reading that list, who
  could help you.

- When using libmodbus from a distribution (Debian, Fedora...), please report
  the bug first in the bug tracker of the distribution. The reason for doing so
  is that the package maintainer should have a chance to look at the issue first
  as it might be a packaging error. If/when the package maintainer comes to the
  conclusion that is really an upstream bug, then he/she will usually report it
  here by himself/herself. This is because he/she is interested in staying in
  the notification chain to decide about a backport as soon as a bugfix is
  available. Otherwise you (distribution user) will be asked to do so
  explicitly.

When you get here and you are still convinced that you want to report a bug:

- *Use a clear and descriptive title* for the issue to identify

- *Which version of libmodbus are you using?* you can obtain this information
   from your package manager or by running `pkg-config --modversion libmodbus`.
   You can provide the sha1 of the commit if you have fetched the code with `git`.

- *Which operating system are you using?*

- *Describe the exact steps which reproduce the problem* in as many details as
   possible. For example, the software/equipment which runs the Modbus server, how
   the clients are connected (TCP, RTU, ASCII) and the source code you are using.

- *Enable the debug mode*, libmodbus provides a function to display the content
   of the Modbus messages and it's very convenient to analyze issues
   (<http://libmodbus.org/docs/modbus_set_debug/>).

Good bug reports provide right and quick fixes!

Finally, thank you very much for using libmodbus and taking the time to
file a good bug report. Doing so signals your respect for the developers.

The following template helps you to address the points above. Please delete
everything up to and including the following line which starts with ---.

---

## libmodbus version

  <libmodbus version or sha1 of the latest commit>

## OS and/or distribution

  <e.g. Windows, Linux... version>

## Environment

  <e.g. CPU architecture, 32 vs. 64 bit...>

## Description

  <...>

## Actual behavior if applicable

  <...>

## Expected behavior or suggestion

  <...>

## Steps to reproduce the behavior (commands or source code)

  <...>

## libmodbus output with debug mode enabled

  <...>
