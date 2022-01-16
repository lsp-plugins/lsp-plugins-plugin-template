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

Creating a new project
======

Here are the following steps of creating new project using this template repository.
We will assume that the new project is called `lsp-plugins-example` and is created at the
same directory the `lsp-plugins-plugin-template` is placed.


* Create github repository.

  This can be done it two ways. The first one, just using `git init`:

      mkdir lsp-plugins-example
      cd lsp-plugins-example
      git init
      git checkout -b master
      git checkout -b devel

  The second way, you can create the repository on GitHub and clone it:

      git clone <project-base-url>/lsp-plugins-example.git
      cd lsp-plugins-example
      git checkout -b devel

* Copy contents of this template repository to the project folder.

      cd ../lsp-plugins-plugin-template/
      cp -r `ls -a | egrep -v '^\.$|^\.\.$|^\.git$'` ../lsp-plugins-example/

* Rename the project.

      cd ../lsp-plugins-example/
      ./rename.sh 'lsp-plugins' 'example' LSP_PLUGINS_EXAMPLE "LSP Example Plugin"
      rm rename.sh

* For Eclipse users, use `File -> Open Projects from File System...` menu to import the project.


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
