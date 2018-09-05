#!/bin/sh

/opt/sublime_text/sublime_text --project KSOM.sublime-project -n

(qtcreator -lastsession &)

DIR="$( dirname "$(readlink -f "$0")")"
cd $DIR/build/linux
exec "$SHELL"
