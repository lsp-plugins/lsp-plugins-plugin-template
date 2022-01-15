# lsp-plugins-plugin-template

Summary
======

Template plugin repository.

This repository has all necessary information to start development of the plugin as a part of
[LSP Plugin Framework](https://github.com/lsp-plugins/lsp-plugin-fw).

The project can be renamed by applying the script from the root:

```
./rename.sh <package-name> <plugin-name> <PLUGIN_ARTIFACT_ID> <Plugin Description>
```

For example:

```
./rename.sh lsp-plugins comp-delay LSP_PLUGINS_COMP_DELAY "LSP Compensation Delay Plugin Series"
```

Building
======

To build the library, perform the following commands:

```bash
make config # Configure the build
make fetch # Fetch dependencies from Git repository
make
sudo make install
```

To get more build options, run:

```bash
make help
```

To uninstall library, simply issue:

```bash
make uninstall
```

To clean all binary files, run:

```bash
make clean
```

To clean the whole project tree including configuration files, run:

```bash
make prune
```

To fetch all possible dependencies and make the source code tree portable between
different architectures and platforms, run:

```bash
make tree
```

To build source code archive with all possible dependencies, run:

```bash
make distsrc
```
