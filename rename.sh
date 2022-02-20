#!/bin/bash

PACKAGE_NAME=$1
PLUGIN_NAME=$2
ARTIFACT_ID=$3
ARTIFACT_DESC=$4

if [ -z "$PACKAGE_NAME" ]; then
	echo "Package name is required, for example: lsp-plugins"
	exit 1;
fi;
if [ -z "$PLUGIN_NAME" ]; then
	echo "Plugin name is required, for example: comp-delay"
	exit 2;
fi;
if [ -z "$ARTIFACT_ID" ]; then
	echo "Artifact identifier is required, for example: LSP_PLUGINS_COMP_DELAY"
	exit 3;
fi;
if [ -z "$ARTIFACT_DESC" ]; then
	echo "Artifact description is required, for example: \"LSP Compensation Delay Plugin Series\""
	exit 4;
fi;

echo "PACKAGE_NAME = ${PACKAGE_NAME}"

PLUGIN_NAME_DASH=`echo "${PLUGIN_NAME}" | sed s/_/-/g`
PLUGIN_NAME_SCORE=`echo "${PLUGIN_NAME_DASH}" | sed s/-/_/g`
PLUGIN_NAME_SCORE_UPPER=`echo "${PLUGIN_NAME_SCORE}" | tr 'a-z' 'A-Z'`

FULL_NAME_DASH=`echo "${PACKAGE_NAME}-${PLUGIN_NAME}" | sed s/_/-/g`
FULL_NAME_SCORE=`echo "${FULL_NAME_DASH}" | sed s/-/_/g`

PLUGIN_NAME_DASH=`echo "${PLUGIN_NAME}" | sed s/_/-/g`
PLUGIN_NAME_SCORE=`echo "${PLUGIN_NAME_DASH}" | sed s/-/_/g`

echo "FULL_NAME_DASH = $FULL_NAME_DASH"
echo "FULL_NAME_SCORE = $FULL_NAME_SCORE"
echo "PLUGIN_NAME_DASH = $PLUGIN_NAME_DASH"
echo "PLUGIN_NAME_SCORE = $PLUGIN_NAME_SCORE"
echo "PLUGIN_NAME_SCORE_UPPER = $PLUGIN_NAME_SCORE_UPPER"

# plugins.mk update
sed -E -i "s/LSP_PLUGINS_PLUGIN_TEMPLATE/${ARTIFACT_ID}/g" "plugins.mk"
sed -E -i "s/^(PLUGIN_PACKAGE_NAME.*=.*)lsp-plugins(.*)/\\1${PACKAGE_NAME}\\2/g" "plugins.mk"
sed -E -i "s/lsp-plugins-plugin-template/${FULL_NAME_DASH}/g" "plugins.mk"

# project.mk update
sed -E -i "s/LSP_PLUGINS_PLUGIN_TEMPLATE/${ARTIFACT_ID}/g" "project.mk"
sed -E -i "s/lsp-plugins-plugin-template/${FULL_NAME_DASH}/g" "project.mk"
sed -E -i "s/^(ARTIFACT_DESC.*=.*)LSP Template Plugin(.*)/\\1${ARTIFACT_DESC}\\2/g" "project.mk"

MAKEFILES_TO_UPDATE="\
  Makefile \
  modules.mk \
  dependencies.mk \
  make/configure.mk \
  make/functions.mk \
  make/modules.mk \
  make/system.mk \
  make/tools.mk \
  src/Makefile"

for file in ${MAKEFILES_TO_UPDATE}
do
  echo "Processing $file"
  sed -E -i "s/lsp-plugins-plugin-template/${FULL_NAME_DASH}/g" "$file"
done;

SOURCE_FILES_TO_UPDATE=
SOURCE_FILES_TO_UPDATE+=".project .cproject"
SOURCE_FILES_TO_UPDATE+=" "
SOURCE_FILES_TO_UPDATE+=`find include -iname *plugin_template*.h`
SOURCE_FILES_TO_UPDATE+=" "
SOURCE_FILES_TO_UPDATE+=`find src -iname *plugin_template*.cpp`
SOURCE_FILES_TO_UPDATE+=" "
SOURCE_FILES_TO_UPDATE+=`find src -iname *plugin_template*.php`
SOURCE_FILES_TO_UPDATE+=" "
SOURCE_FILES_TO_UPDATE+=`find res -iname *.json`
SOURCE_FILES_TO_UPDATE+=" "
SOURCE_FILES_TO_UPDATE+=`find res -iname *.desktop`

echo "Files to update: ${SOURCE_FILES_TO_UPDATE}"

for file in ${SOURCE_FILES_TO_UPDATE}
do
  echo "Processing $file"
  sed -E -i "s/plugin_template/${PLUGIN_NAME_SCORE}/g" "$file"
  sed -E -i "s/lsp-plugins-plugin-template/${FULL_NAME_DASH}/g" "$file"
  sed -E -i "s/LSP_PLUGINS_PLUGIN_TEMPLATE/${ARTIFACT_ID}/g" "$file"
  sed -E -i "s/PLUGIN_TEMPLATE/${PLUGIN_NAME_SCORE_UPPER}/g" "$file"
  new_file=`echo "$file" | sed -E "s/plugin_template/${PLUGIN_NAME_SCORE}/g"`
  if [ "$file" != "$new_file" ]; then
    mv "$file" "$new_file"
  fi;
done;

