#root name for binary and .lx file
target := rover

#slot to download to on RCX
PNUM := 1

#extra parameters for emuLegos executable
emu_params := -sen1 touch -sen3 touch

#source file which includes main
MAINSRC := rover.c

#all sources but the one including main
CSRC := 