@call common.bat
rem start quake2.exe -nosound -logfile=2 -developer -game=dirty
rem start quake2.exe -nosound -r_drawfps=1 -r_stats=1 -developer -game=dirty
start quake2.exe -nosound -r_drawfps -r_stats -game=dirty -nointro -cheats  %1 %2 %3 %4 %5 %6 %7
rem BUG: start quake2.exe -nosound -r_drawfps -r_stats -game=dirty -nointro -cheats -map ground3 -r_showBrush=1585
