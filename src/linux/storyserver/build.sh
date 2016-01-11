#!/bin/sh


#######################################################################
# STORYMOD
# 
# Website: https://sites.google.com/site/storymodsite
# GitHub: https://github.com/lcudenne/storymod
# Author: Loic Cudennec <loic@cudennec.fr>
#
# This program is a story server for Euro Truck Simulator 2 and
# American Truck Simulator. It reads a driving trace from either a
# local file or a UDP stream connected to the simulator and
# periodically updates a local HTML file to display the current story
# state to the player. Stories are loaded from a given directory.
#
#######################################################################
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################


LOC=`find . -type f -name "*.c" -o -name "*.h" -o -name "*.sh" | xargs cat | sed '/^\s*$/d' | wc -l`

CC="gcc"
CFLAGS="-g -Wall -Werror -Wno-variadic-macros"
CFLAGSLIBXML2=`xml2-config --cflags`
ANSI="-ansi -pedantic"
LDFLAGS="-lm"
LDFLAGSLIBXML2=`xml2-config --libs`

exitOnErr() {
 if [ ${?} -ne 0 ]; then
  printf "${1} failed\n"
  exit 1
 fi
}


cmd="${CC} -c utils.c ${CFLAGS} -o utils.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c udpstream.c ${CFLAGS} -o udpstream.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storystruct.c ${CFLAGS} ${ANSI} -o storystruct.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storylibxml2.c ${CFLAGS} ${ANSI} ${CFLAGSLIBXML2} -o storylibxml2.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storyposixload.c ${CFLAGS} -o storyposixload.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storyhtml.c ${CFLAGS} ${ANSI} -o storyhtml.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storydot.c ${CFLAGS} ${ANSI} -o storydot.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storyautomata.c ${CFLAGS} ${ANSI} -o storyautomata.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} -c storyserver.c ${CFLAGS} ${ANSI} -o storyserver.o"
echo $cmd
`$cmd`
exitOnErr "Compilation of source files"

cmd="${CC} utils.o udpstream.o storystruct.o storylibxml2.o storyposixload.o storyhtml.o storydot.o storyautomata.o storyserver.o -o storyserver ${LDFLAGS} ${LDFLAGSLIBXML2}"
echo $cmd
`$cmd`
exitOnErr "Building binary file"

echo "=== storymod (${LOC} lines of code)"

