{ nixpkgs ? <nixpkgs>
, systems ? [ "i686-linux" "x86_64-linux" ]
, disnix ? { outPath = ./.; rev = 1234; }
, dysnomia ? { outPath = ../dysnomia; rev = 1234; }
, officialRelease ? false
}:

let
  pkgs = import nixpkgs {};

  dysnomiaJobset = import "${dysnomia}/release.nix" {
    inherit nixpkgs systems officialRelease dysnomia;
  };

  jobs = rec {
    tarball =
      with pkgs;

      let
        dysnomia = builtins.getAttr (builtins.currentSystem) (dysnomiaJobset.build);
      in
      releaseTools.sourceTarball {
        name = "disnix-tarball";
        version = builtins.readFile ./version;
        src = disnix;
        inherit officialRelease;
        dontBuild = false;

        buildInputs = [ pkgconfig glib libxml2 libxslt getopt dblatex (dblatex.tex or tetex) doxygen nukeReferences help2man doclifter dysnomia ];

        CFLAGS = "-Wall";

        # Add documentation in the tarball
        configureFlags = ''
          --with-docbook-rng=${docbook5}/xml/rng/docbook
          --with-docbook-xsl=${docbook5_xsl}/xml/xsl/docbook
        '';

        preConfigure = ''
          # TeX needs a writable font cache.
          export VARTEXFONTS=$TMPDIR/texfonts
        '';

        preDist = ''
          make -C doc/manual install prefix=$out

          make -C doc/manual index.pdf prefix=$out
          cp doc/manual/index.pdf $out/index.pdf

          make -C src apidox
          cp -av doc/apidox $out/share/doc/disnix

          # The PDF containes filenames of included graphics (see
          # http://www.tug.org/pipermail/pdftex/2007-August/007290.html).
          # This causes a retained dependency on dblatex, which Hydra
          # doesn't like (the output of the tarball job is distributed
          # to Windows and Macs, so there should be no Linux binaries
          # in the closure).
          nuke-refs $out/index.pdf

          echo "doc-pdf manual $out/index.pdf" >> $out/nix-support/hydra-build-products
          echo "doc manual $out/share/doc/disnix/manual" >> $out/nix-support/hydra-build-products
          echo "doc api $out/share/doc/disnix/apidox/html" >> $out/nix-support/hydra-build-products
        '';
      };

    build =
      pkgs.lib.genAttrs systems (system:
        let
          dysnomia = builtins.getAttr system (dysnomiaJobset.build);
        in
        with import nixpkgs { inherit system; };

        releaseTools.nixBuild {
          name = "disnix";
          src = tarball;

          buildInputs = [ pkgconfig glib libxml2 libxslt getopt dysnomia ]
            ++ lib.optionals (!stdenv.isLinux) [ libiconv gettext ];

          CFLAGS = "-Wall";
        });

    tests =
      let
        dysnomia = builtins.getAttr (builtins.currentSystem) (dysnomiaJobset.build);
        disnix = builtins.getAttr (builtins.currentSystem) (jobs.build);
      in
      {
        runactivities = import ./tests/runactivities.nix {
          inherit nixpkgs dysnomia disnix;
        };
        dbus = import ./tests/dbus.nix {
          inherit nixpkgs dysnomia disnix;
        };
        ssh-to-runactivity = import ./tests/ssh.nix {
          inherit nixpkgs dysnomia disnix;
          disnixRemoteClient = "disnix-run-activity";
        };
        ssh-to-dbus = import ./tests/ssh.nix {
          inherit nixpkgs dysnomia disnix;
          disnixRemoteClient = "disnix-client";
        };
        install = import ./tests/install.nix {
          inherit nixpkgs dysnomia disnix;
        };
        deployment = import ./tests/deployment.nix {
          inherit nixpkgs dysnomia disnix;
        };
        distbuild = import ./tests/distbuild.nix {
          inherit nixpkgs dysnomia disnix;
        };
        snapshots-via-runactivity = import ./tests/snapshots.nix {
          inherit nixpkgs dysnomia disnix;
          inherit (pkgs) stdenv;
          disnixRemoteClient = "disnix-run-activity";
        };
        snapshots-via-dbus = import ./tests/snapshots.nix {
          inherit nixpkgs dysnomia disnix;
          inherit (pkgs) stdenv;
          disnixRemoteClient = "disnix-client";
        };
        datamigration = import ./tests/datamigration.nix {
          inherit nixpkgs dysnomia disnix;
        };
        commands = import ./tests/commands.nix {
          inherit nixpkgs dysnomia disnix;
        };
        locking = import ./tests/locking.nix {
          inherit nixpkgs dysnomia disnix;
        };
        pkgs = import ./tests/pkgs.nix {
          inherit nixpkgs dysnomia disnix;
        };
        daemon = import ./tests/daemon.nix {
          inherit nixpkgs disnix;
        };
      };

    release = pkgs.releaseTools.aggregate {
      name = "disnix-${tarball.version}";
      constituents = [
        tarball
      ]
      ++ map (system: builtins.getAttr system build) systems
      ++ [
        tests.runactivities
        tests.dbus
        tests.ssh-to-runactivity
        tests.ssh-to-dbus
        tests.install
        tests.commands
        tests.deployment
        tests.distbuild
        tests.snapshots-via-runactivity
        tests.snapshots-via-dbus
        tests.datamigration
        tests.locking
        tests.pkgs
        tests.daemon
      ];
      meta.description = "Release-critical builds";
    };
  };
in
jobs
