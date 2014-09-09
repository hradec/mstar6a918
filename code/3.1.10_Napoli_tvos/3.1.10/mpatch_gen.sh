#!/bin/bash 

rm -rf mpatch_Kconfig
../mstar/mpatch/mpatch.bin mpatch.config mpatch_macro.h mpatch_Kconfig ../mstar/mpatch/doc/
if [ "$?" == "1" ]; then
    exit 1
fi
mkdir ./include/mstar 2> /dev/null

diff mpatch_macro.h include/mstar/mpatch_macro.h > /dev/null 2>&1
if [ "$?" != "0" ]; then
	# src file: mpatch_macro.h
	# dst file: include/mstar/mpatch_macro.h
	# if dst file does not exist -> $? = 2
	# if dst file is different from src file -> $? = 1
	cp -af mpatch_macro.h ./include/mstar
fi

rm -rf mpatch_macro.h
cd ..
exit $mpatch_return_value

