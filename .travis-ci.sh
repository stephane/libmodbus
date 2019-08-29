#!/bin/bash

# This script is triggered from the script section of .travis.yml
# It runs the appropriate commands depending on the task requested.

set -e

CPP_LINT_URL="https://raw.githubusercontent.com/google/styleguide/gh-pages/cpplint/cpplint.py";

COVERITY_SCAN_BUILD_URL="https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh"

SPELLINGBLACKLIST=$(cat <<-BLACKLIST
      -wholename "./.codespellignore" -or \
      -wholename "./.git/*" -or \
      -wholename "./aclocal.m4" -or \
      -wholename "./config/config.guess" -or \
      -wholename "./config/config.sub" -or \
      -wholename "./config/depcomp" -or \
      -wholename "./config/install-sh" -or \
      -wholename "./config/libtool.m4" -or \
      -wholename "./config/ltmain.sh" -or \
      -wholename "./config/ltoptions.m4" -or \
      -wholename "./config/ltsugar.m4" -or \
      -wholename "./config/missing" -or \
      -wholename "./libtool" -or \
      -wholename "./config.log" -or \
      -wholename "./config.status" -or \
      -wholename "./Makefile" -or \
      -wholename "./Makefile.in" -or \
      -wholename "./autom4te.cache/*" -or \
      -wholename "./configure"
BLACKLIST
)

if [[ $TASK = 'lint' ]]; then
  # run the lint tool only if it is the requested task
  # first check we've not got any generic NOLINTs
  # count the number of generic NOLINTs
  nolints=$(grep -IR NOLINT * | grep -v "NOLINT(" | wc -l)
  if [[ $nolints -ne 0 ]]; then
    # print the output for info
    echo $(grep -IR NOLINT * | grep -v "NOLINT(")
    echo "Found $nolints generic NOLINTs"
    exit 1;
  else
    echo "Found $nolints generic NOLINTs"
  fi;
  # then fetch and run the main cpplint tool
  wget -O cpplint.py $CPP_LINT_URL;
  chmod u+x cpplint.py;
  ./cpplint.py \
    $(find ./ \( -name "*.h" -or -name "*.c" \) -and ! \( \
        -wholename "./config.h" \) | xargs)
  if [[ $? -ne 0 ]]; then
    exit 1;
  fi;
elif [[ $TASK = 'spellintian' ]]; then
  # run spellintian only if it is the requested task, ignoring duplicate words
  spellingfiles=$(eval "find ./ -type f -and ! \( \
      $SPELLINGBLACKLIST \
      \) | xargs")
  # count the number of spellintian errors, ignoring duplicate words
  spellingerrors=$(zrun spellintian $spellingfiles 2>&1 | grep -v "\(duplicate word\)" | wc -l)
  if [[ $spellingerrors -ne 0 ]]; then
    # print the output for info
    zrun spellintian $spellingfiles | grep -v "\(duplicate word\)"
    echo "Found $spellingerrors spelling errors via spellintian, ignoring duplicates"
    exit 1;
  else
    echo "Found $spellingerrors spelling errors via spellintian, ignoring duplicates"
  fi;
elif [[ $TASK = 'spellintian-duplicates' ]]; then
  # run spellintian only if it is the requested task
  spellingfiles=$(eval "find ./ -type f -and ! \( \
      $SPELLINGBLACKLIST \
      \) | xargs")
  # count the number of spellintian errors
  spellingerrors=$(zrun spellintian $spellingfiles 2>&1 | wc -l)
  if [[ $spellingerrors -ne 0 ]]; then
    # print the output for info
    zrun spellintian $spellingfiles
    echo "Found $spellingerrors spelling errors via spellintian"
    exit 1;
  else
    echo "Found $spellingerrors spelling errors via spellintian"
  fi;
elif [[ $TASK = 'codespell' ]]; then
  # run codespell only if it is the requested task
  spellingfiles=$(eval "find ./ -type f -and ! \( \
      $SPELLINGBLACKLIST \
      \) | xargs")
  # count the number of codespell errors
  spellingerrors=$(zrun codespell --check-filenames --check-hidden --quiet 2 --regex "[a-zA-Z0-9][\\-'a-zA-Z0-9]+[a-zA-Z0-9]" $spellingfiles 2>&1 | wc -l)
  if [[ $spellingerrors -ne 0 ]]; then
    # print the output for info
    zrun codespell --check-filenames --check-hidden --quiet 2 --regex "[a-zA-Z0-9][\\-'a-zA-Z0-9]+[a-zA-Z0-9]" $spellingfiles
    echo "Found $spellingerrors spelling errors via codespell"
    exit 1;
  else
    echo "Found $spellingerrors spelling errors via codespell"
  fi;
elif [[ $TASK = 'coverity' ]]; then
  # Run Coverity Scan unless token is zero length
  # The Coverity Scan script also relies on a number of other COVERITY_SCAN_
  # variables set in .travis.yml
  if [[ ${#COVERITY_SCAN_TOKEN} -ne 0 ]]; then
    curl -s $COVERITY_SCAN_BUILD_URL | bash
  else
    echo "Skipping Coverity Scan as no token found, probably a Pull Request"
  fi;
else
  # Otherwise compile and check as normal
#  travis_fold start "autogen"
  ./autogen.sh;
#  travis_fold end "autogen"
#  travis_fold start "configure"
  ./configure;
#  travis_fold end "configure"
#  travis_fold start "make"
  make;
#  travis_fold end "make"
#  travis_fold start "make_check"
  make check;
#  travis_fold end "make_check"
fi
