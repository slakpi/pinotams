PiNOTAMs: Raspberry Pi Aviation NOTAMs Scraper
==============================================

PiNOTAMs is a configurable aviation NOTAMs scraper that queries the ICAO API
Data Service for new NOTAMs and delivers the text of any new NOTAMs to an email
address.

NOTICE
------

Use of this program in no way satisfies any regulatory requirement for pre-
flight planning action in any country.

Building
--------

PiNotams uses CMake 3.6 to build. The following dependencies must be installed:

  * Bison (https://www.gnu.org/software/bison/)
  * Flex (https://github.com/westes/flex)
  * libcURL (https://curl.haxx.se/libcurl/)
  * jansson (http://www.digip.org/jansson/)
  * SQLite3 (https://sqlite.org/index.html)
  * libuuid (https://sourceforge.net/projects/libuuid/)
  * libmhash (https://sourceforge.net/projects/mhash/)
  * pcre2 (https://pcre.org)

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

    # ICAO API Data Service key. Register here:
    # https://www.icao.int/safety/iStars/Pages/API-Data-Service.aspx
    api-key="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";

    # 4-letter ICAO identifiers. For US ARTCC NOTAMs, use KZ**, e.g. KZSE for
    # Seattle ARTCC. For US regulatory NOTAMs, use KZZZ. For US non-ICAO
    # identifiers such as 7S3, use K***, e.g. K7S3.
    #
    # Multiple location directives are allowed, and each can have up to 10
    # locations. Each is sent as a different query to the ICAO API Data
    # Service, and the ICAO API limits each query to 10 locations.
    locations="K7S3,KHIO,KMMV,KUAO,KSLE,KVUO,KSPB,KZSE";
    locations=...more locations...;

    # Interval in minutes for NOTAM queries. To be kind to the ICAO service,
    # keep this larger than 60 minutes. PiNOTAMs updates at 0000Z + this time
    # rather than startup + this time. So, if this is set for 6 hours and it
    # is 0555Z when PiNOTAMs starts, the first update will be at 0600Z, not
    # 1155Z.
    refresh-rate=360;

    # Specify custom regular expression filters for NOTAMs. Multiple filter
    # directives are allowed.
    filter="^!SUAW";
    fitler=...another filter...

    # Non-0 to log debug information.
    debug-log=0;

    # SMTP outgoing mail server settings. Refer to your mail service's
    # settings. NOT tested with Gmail's application permissions. Multiple
    # smtp-recipient directives are allowed.
    smtp-server="my.mail-server.com";
    smtp-port=587;
    smtp-user="sender@mydomain.com";
    smtp-pwd="secret";
    smtp-sender="sender@mydomain.com";
    smtp-sender-name="My NOTAMs Service";
    smtp-recipient="you@yourdomain.com";
    smtp-recipient=...another email address...;

    # Non-0 to use SSL/TLS and user/password authentication.
    smtp-tls=1;

Running Automatically
---------------------

To run PiNOTAMs automatically on boot, refer to the sample service file in the
repository. This file must be placed in `/lib/systemd/system` and renamed to
`pinotams.service`.

Use `sudo systemctl start pinotams.service` to test starting PiNOTAMs and use
`sudo systemctl stop pinotams.service` to stop it. To enable automatically
starting PiNOTAMs on boot, use `sudo systemctl enable pinotams.service`.

Docker
------

PiNOTAMs includes two Dockerfiles: `Dockerfile.build` and `Dockerfile`.
`Dockerfile.build` creates a Docker container that can build PiNOTAMs with
Ubuntu 20.04. The following command will build the image:

    % docker build . -f Dockerfile.build -t pinotams-build-base:latest

With the build image, run the following command to build the final image:

    % docker build . -t pinotams:latest

The build uses `/opt/pinotams` as the install prefix inside the container. To
store the configuration, cache database, and log files outside of the container,
use the following volume maps:

    <Host etc>:/opt/pinotams/etc
    <Host var>:/opt/pinotams/var/pinotams

For example:

    % mkdir -p /opt/pinotams/etc
    % mkdir -p /opt/pinotams/var
    % cp pinotams.conf.example /opt/pinotams/etc/pinotams.conf
    % vi /opt/pinotams/pinotams.conf
      ...edit the configuration...
    % docker run -d --name=pinotams --hostname=pinotams --restart=always \
    -v /opt/pinotams/etc:/opt/pinotams/etc \
    -v /opt/pinotams/var:/opt/pinotams/var/pinotams \
    pinotams:latest
