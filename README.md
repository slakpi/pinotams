PiNOTAMs: Raspberry Pi Aviation NOTAMs Scraper
==============================================

PiNOTAMs is a configurable aviation NOTAMs scraper that queries the ICAO API
Data Service for new NOTAMs and delivers the text of any new NOTAMs to an email
address.

Building
--------

PiNotams uses CMake 3.6 to build. The following dependencies must be installed:

  * Bison (https://www.gnu.org/software/bison/)
  * Flex (https://github.com/westes/flex)
  * libcURL (https://curl.haxx.se/libcurl/)
  * jansson (http://www.digip.org/jansson/)
  * SQLite3 (https://sqlite.org/index.html)
  * libuuid (https://sourceforge.net/projects/libuuid/)

After cloning the repository, use the following commands to perform a simple
build:

    % git clone https://github.com/slakpi/pinotams.git
    % cd pinotams
    % mkdir build
    % cd build
    % cmake -DCMAKE_INSTALL_PREFIX="/usr/local/pinotams" -DCMAKE_BUILD_TYPE=Release ..
    % make
    % sudo make install

Configuration
-------------

Configuration is relatively simple. Refer to the sample configuration file that
installs to the `etc` directory under the install prefix. Simply copy the sample
to a new file called `pinotams.conf`.

Currently, PiNOTAMs only supports 10 locations in a query and a single recipient
email address.

You must register for an ICAO API key and have access to a SMTP server.

Running Automatically
---------------------

To run PiNOTAMs automatically on boot, refer to the sample service file in the
repository. This file must be placed in `/lib/systemd/system` and renamed to
`pinotams.service`.

Use `sudo systemctl start pinotams.service` to test starting PiNOTAMs and use
`sudo systemctl stop pinotams.service` to stop it. To enable automatically
starting PiNOTAMs on boot, use `sudo systemctl enable pinotams.service`.
