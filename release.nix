{ nixpkgs ? <nixpkgs>
, systems ? [ "i686-linux" "x86_64-linux" ]
, fetchDependenciesFromNixpkgs ? false
, disnix ? { outPath = ./.; rev = 1234; }
, officialRelease ? false
}:

let
  pkgs = import nixpkgs {};
  
  # Refer either to dysnomia in the parent folder, or to the one in Nixpkgs
  dysnomiaJobset = if fetchDependenciesFromNixpkgs then {
    build = pkgs.lib.genAttrs systems (system:
      (import nixpkgs { inherit system; }).dysnomia
    );
  } else import ../dysnomia/release.nix { inherit nixpkgs systems officialRelease; };
  
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

        buildInputs = [ pkgconfig glib libxml2 libxslt getopt nixUnstable dblatex (dblatex.tex or tetex) doxygen nukeReferences help2man doclifter dysnomia ];
        
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

          buildInputs = [ pkgconfig glib libxml2 libxslt getopt nixUnstable dysnomia ]
            ++ lib.optionals (!stdenv.isLinux) [ libiconv gettext ];
            
          CFLAGS = "-Wall";
        });
      
    tests =
      let
        dysnomia = builtins.getAttr (builtins.currentSystem) (dysnomiaJobset.build);
        disnix = builtins.getAttr (builtins.currentSystem) (jobs.build);
      in
      {
        install = import ./tests/install.nix {
          inherit nixpkgs dysnomia disnix;
        };
        deployment = import ./tests/deployment.nix {
          inherit nixpkgs dysnomia disnix;
        };
        distbuild = import ./tests/distbuild.nix {
          inherit nixpkgs dysnomia disnix;
        };
        snapshots = import ./tests/snapshots.nix {
          inherit nixpkgs dysnomia disnix;
          inherit (pkgs) stdenv;
        };
        datamigration = import ./tests/datamigration.nix {
          inherit nixpkgs dysnomia disnix;
        };
        pkgs = import ./tests/pkgs.nix {
          inherit nixpkgs dysnomia disnix;
        };
      };
    
    release = pkgs.releaseTools.aggregate {
      name = "disnix-${tarball.version}";
      constituents = [
        tarball
      ]
      ++ map (system: builtins.getAttr system build) systems
      ++ [
        tests.install
        tests.deployment
        tests.distbuild
        tests.snapshots
        tests.datamigration
        tests.pkgs
      ];
      meta.description = "Release-critical builds";
    };
  };
in
jobs
