@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall" x64

start /b "sublime" "C:\Program Files\Sublime Text 3\sublime_text.exe" --project NN.sublime-project -n

call devenv build\main.exe