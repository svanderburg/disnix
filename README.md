Disnix
======
Disnix is a distributed service deployment extension for the Nix package manager.

Nix builds packages from Nix expressions and manages intra-dependencies on 
single systems. Disnix extends the Nix approach to distributed service-oriented
systems by managing inter-dependencies  of a distributed system and performs the
distribution and activation of distributed system components.

It uses Nix expressions that capture the services, the infrastructure and the
distribution of services to machines to automate the entire deployment process.

Prerequisites
=============
In order to build Disnix from source code, the following packages are required:

* [libxml2](http://xmlsoft.org)
* [libxslt](http://xmlsoft.org)
* [glib](https://developer.gnome.org/glib)
* [DBus](http://www.freedesktop.org/wiki/Software/dbus)
* [DBus-GLib](http://www.freedesktop.org/wiki/Software/dbus)

To be able to build software with Disnix, Nix and Nixpkgs are required:

* [Nix](http://www.nixos.org/nix)
* [Nixpkgs](http://www.nixos.org/nixpkgs)

To run the Disnix service you need the following packages:

* [Dysnomia](https://github.com/svanderburg/dysnomia), for activation and deactivation of services
* [OpenSSH](http://www.openssh.org), for using the `disnix-ssh-client`

These dependencies can be acquired with the Nix package manager, your host
system's package manager or be compiled from sources. Consult the documentation
of your distribution or the corresponding packages for more information.

Installation:
=============
Disnix is a typical autotools based package which can be compiled and installed
by running the following commands in a shell session:

    $ ./configure
    $ make
    $ make install

For more information about using the autotools setup or for customizing the
configuration, take a look at the `./INSTALL` file.

Disnix must be installed on a *coordinator* machine that initiates deployment as
well as all *target* machines in the network to which service components can be
deployed.

On the target machines, you must also run the `disnix-service` to make deployment
operations remotely accessible. The Disnix service requires a protocol wrapper
to actually do this. The default wrapper in the Disnix distribution is the SSH
wrapper. More information on this can be found in the Disnix manual.

Moreover, it also requires Dysnomia to be installed so that services can
activated and deactivated.

The coordinator machine requires the presence of a copy of Nixpkgs to make
building of service components possible. In order to find the location of
Nixpkgs, the `NIX_PATH` environment variable must be refer to the location where
Nixpkgs is stored. This can be done by running the following command-line
instruction:

$ export NIX_PATH="nixpkgs=/path/to/nixpkgs"

On NixOS, this environment variable has already been configured.

Usage
=====
In order to deploy a service-oriented system, a developer has to write Nix
expressions that capture the *services* of which a distributed system consists,
the *infrastructure* that descibes the machines in a network (including their
properties) and a *distribution* of service components to machines in the
network.

By invoking 'disnix-env' with these Nix expressions as parameters, services are
automatically built from source code (including all its intra-dependencies),
distributed to the target machines and activated in the right order. In case of a
failure a rollback is performed:

    $ disnix-env -s services.nix -i infrastructure.nix -d distribution.nix
 
See the tutorials on the webpage for more information on deploying a
service-oriented system with Disnix.

Disnix itself has a modular architecture, which supports various extensions that
can be used to make integration with the host environment better and deployment
activities more convenient. Refer to the Disnix manual to see what extensions can
be used.

Manual
======
Disnix has a nice Docbook manual that can be compiled yourself. However, it is
also available [online](http://hydra.nixos.org/job/disnix/disnix-trunk/tarball/latest/download-by-type/doc/manual).

License
=======
Disnix is free software; you can redistribute it and/or modify it under the terms
of the [GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)
as published by the Free Software Foundation](http://www.fsf.org) either version
2.1 of the License, or (at your option) any later version. Disnix is distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.
