#!/bin/bash
set -e
set -x


case "$OSTYPE" in
  darwin*)
      PLATFORM=darwin
      ;;
  *)
      PLATFORM=linux
      ;;
esac

source `dirname $0`/ndk_definitions.sh

export NDK_FILENAME=${NDK_VERSION}-${PLATFORM}-x86_64.zip

#sha256_file=5dfbbdc2d3ba859fed90d0e978af87c71a91a5be1f6e1c40ba697503d48ccecd

mkdir ./developer-tools/android-ndk-gulden | true
cd ./developer-tools/android-ndk-gulden
curl -sSO https://dl.google.com/android/repository/${NDK_FILENAME} &> /dev/null
#echo "${sha256_file}  ${NDK_FILENAME}" | shasum -a 256 --check
unzip -qq ${NDK_FILENAME} &> /dev/null
rm ${NDK_FILENAME}
cd -
