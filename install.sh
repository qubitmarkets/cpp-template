#!/bin/bash

set -u

force=0
skip_diff=0
quiet=0
copy=0
diff_flag="-q"
for arg in $@; do
    if [[ ${arg:0:1} == '-' ]]; then
        if [[ $arg == "-f" ]]; then
            force=1
        elif [[ $arg == "-s" ]]; then
            skip_diff=1
        elif [[ $arg == "-c" ]]; then
            copy=1
        elif [[ $arg == "-v" ]]; then
            diff_flag=""
        fi
    else
        dest=${arg}
    fi
done

if [[ ! -d "${dest-}" ]]; then
    echo "cpp-template : Installs C++ development tooling to a destination dir"
    echo "Updating: Safe to run multiple times.  Will check & stop if you update/edit the cpp-template files in the template dir"
    echo "Usage : $0 <destination dir>"
    exit 1
fi
dest=$(readlink -f $dest)

src=$(dirname $BASH_SOURCE)
src=$(readlink -f $src)
cd $src

count=0
if [[ $skip_diff == 0 ]]; then
  for f in $(cat ./filelist); do
    if [[ -e $dest/$f ]]; then
        diff -r $diff_flag $src/$f $dest/$f
        if [[ $? != 0 ]]; then
            echo "  diff -r $src/$f $dest/$f"
            count=$((count + 1))
        fi
    fi
  done
fi
if [[ $skip_diff == 0 && $force == 0 && $count -gt 0 ]]; then
    echo "Files in $dest differ to cpp-template.  Fix this or use -f to force or -s to skip those files"
    exit 1
fi

# links to dirs
for f in 3rdparty qbuild etc; do
    if [[ -e $dest/f && $skip_diff == 1 ]]; then
      if ! diff -q -r $src/$f $dest/$f; then
        continue;
      fi
    fi
    # Copy or symlink the destination
    if [[ $copy == 0 ]]; then
      echo "ln -nfs $src/$f $dest/$f"
      ln -nfs $src/$f $dest/$f
    else
      rsync -a $src/$f/ $dest/$f/
    fi
done

# copy files
for f in $(cat $src/filelist); do
    if [[ -e $dest/f && $skip_diff == 1 ]]; then
      if ! diff -q -r $src/$f $dest/$f; then
        continue;
      fi
    fi

    echo cp -r $src/$f $dest/$f
    cp -r $src/$f $dest/$f
done

# Done
echo
echo "Updated $dest from cpp-template"
echo
