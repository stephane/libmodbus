How Do I Submit A Good Bug Report?
----------------------------------

Please, don't send direct emails to Stéphane Raimbault unless you want
commercial support.

Take care to read the documentation at http://libmodbus.org/documentation/.

- *Be sure it's a bug before creating an issue*, in doubt, post a message on
  https://groups.google.com/forum/#!forum/libmodbus or send an email to
  libmodbus@googlegroups.com

- *Use a clear and decriptive title* for the issue to identify

- *Which version of libmodbus are you using?* you can obtain this information
from your package manager or by running `pkg-config --modversion libmodbus`.
You can provide the sha1 of the commit if you have fetched the code with `git`.

- *Which operating system are you using?*

- *Describe the exact steps which reproduce the problem* in as many details as
possible. For example, the software/equipement which runs the Modbus server, how
the clients are connected (TCP, RTU, ASCII) and the source code you are using.

- *Enable the debug mode*, libmodbus provides a function to display the content
of the Modbus messages and it's very convenient to analyze issues
(http://libmodbus.org/docs/latest/modbus_set_debug.html).

Good bug reports provide right and quick fixes!
