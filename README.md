PiNOTAMs: Raspberry Pi Aviation NOTAMs Scraper
==============================================

IN PROGRESS. PiNOTAMs will be a configurable aviation NOTAMs scraper that
queries the ICAO API Data Service for new NOTAMs and delivers the text of
any new NOTAMs to a list of email addresses.

Building
--------

PiNotams uses CMake 3.6 to build. The following dependencies must be installed:

  * Bison (https://www.gnu.org/software/bison/)
  * Flex (https://github.com/westes/flex)
  * libcURL (https://curl.haxx.se/libcurl/)
  * jansson (http://www.digip.org/jansson/)
  * EtPan (https://github.com/dinhviethoa/libetpan)
  * SQLite3 (https://sqlite.org/index.html)

After cloning the repository, use the following commands to perform a simple
build:

    % git clone https://github.com/slakpi/pinotams.git
    % cd pinotams
    % mkdir build
    % cd build
    % cmake -DCMAKE_INSTALL_PREFIX="/usr/local/pinotams" -DCMAKE_BUILD_TYPE=Release ..
    % make
    % sudo make install

