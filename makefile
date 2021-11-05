all:
	gcc play.c attack.c init.c bitboards.c hashkeys.c board.c data.c io.c movegen.c validate.c makemove.c perft.c playsearch.c misc.c pvtable.c evaluate.c -o play