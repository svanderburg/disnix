bin_SCRIPTS = disnix-instantiate disnix-manifest disnix-gendist-roundrobin disnix-copy-snapshots disnix-delegate disnix-env disnix-ssh-client disnix-reconstruct disnix-normalize-infra disnix-convert
pkgdata_SCRIPTS = checks
noinst_DATA = disnix-instantiate.1.xml disnix-manifest.1.xml disnix-gendist-roundrobin.1.xml disnix-copy-snapshots.1.xml disnix-delegate.1.xml disnix-env.1.xml disnix-ssh-client.1.xml disnix-reconstruct.1.xml disnix-normalize-infra.1.xml disnix-convert.1.xml

disnix-instantiate.1: disnix-instantiate.in
	$(HELP2MAN) --output=$@ --no-info --name 'Instantiate a distributed derivation from Disnix expressions' "$(SHELL) disnix-instantiate"

disnix-instantiate.1.xml: disnix-instantiate.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-manifest.1: disnix-manifest.in
	$(HELP2MAN) --output=$@ --no-info --name 'Generate a deployment manifest file from Disnix expressions' "$(SHELL) disnix-manifest"

disnix-manifest.1.xml: disnix-manifest.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-gendist-roundrobin.1: disnix-gendist-roundrobin.in
	$(HELP2MAN) --output=$@ --no-info --name 'Generate a distribution expression from a service and infrastructure expression' "$(SHELL) disnix-gendist-roundrobin"

disnix-gendist-roundrobin.1.xml: disnix-gendist-roundrobin.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-copy-snapshots.1: disnix-copy-snapshots.in
	$(HELP2MAN) --output=$@ --no-info --name 'Copy a set of snapshots from or to a remote machine through a Disnix interface' "$(SHELL) disnix-copy-snapshots"

disnix-copy-snapshots.1.xml: disnix-copy-snapshots.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-delegate.1: disnix-delegate.in
	$(HELP2MAN) --output=$@ --no-info --name 'Delegates service builds to the target machines' "$(SHELL) disnix-delegate"

disnix-delegate.1.xml: disnix-delegate.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-env.1: disnix-env.in
	$(HELP2MAN) --output=$@ --no-info --name 'Installs or updates the environment of a distributed system' "$(SHELL) disnix-env"

disnix-env.1.xml: disnix-env.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-ssh-client.1: disnix-ssh-client.in
	$(HELP2MAN) --output=$@ --no-info --name 'Provides access to the disnix-service through a SSH interface' "$(SHELL) disnix-ssh-client"

disnix-ssh-client.1.xml: disnix-ssh-client.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-reconstruct.1: disnix-reconstruct.in
	$(HELP2MAN) --output=$@ --no-info --name 'Reconstructs the deployment manifest on the coordinator machine from the manifests on the target machines' "$(SHELL) disnix-reconstruct"

disnix-reconstruct.1.xml: disnix-reconstruct.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-normalize-infra.1: disnix-normalize-infra.in
	$(HELP2MAN) --output=$@ --no-info --name 'Normalizes an infrastructure model with default settings and displays an XML representation of it' "$(SHELL) disnix-normalize-infra"

disnix-normalize-infra.1.xml: disnix-normalize-infra.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-convert.1: disnix-convert.in
	$(HELP2MAN) --output=$@ --no-info --name 'Converts a deployment using V1 manifests to V2' "$(SHELL) disnix-convert"

disnix-convert.1.xml: disnix-convert.1
	$(SHELL) ../maintenance/man2docbook.bash $<

man1_MANS = disnix-instantiate.1 disnix-manifest.1 disnix-gendist-roundrobin.1 disnix-copy-snapshots.1 disnix-delegate.1 disnix-env.1 disnix-ssh-client.1 disnix-reconstruct.1 disnix-normalize-infra.1 disnix-convert.1

EXTRA_DIST = disnix-instantiate.in disnix-manifest.in disnix-gendist-roundrobin.in disnix-copy-snapshots.in disnix-delegate.in disnix-env.in disnix-ssh-client.in disnix-normalize-infra.in disnix-convert.in \
  $(pkgdata_SCRIPTS) \
  $(man1_MANS) \
  $(noinst_DATA)
