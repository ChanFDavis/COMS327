#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>

#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"

const char *victory =
	"\n                                       o\n"
	"                                      $\"\"$o\n"
	"                                     $\"  $$\n"
	"                                      $$$$\n"
	"                                      o \"$o\n"
	"                                     o\"  \"$\n"
	"                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
	"   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
	"\"oo   o o o o\n"
	"   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
	"   \"o$$\"    o$$\n"
	"     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
	"     o\"\"\n"
	"        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
	"   o\"\n"
	"         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
	"\"$$$  $\n"
	"           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
	"\"\"      \"\"\" \"\n"
	"            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
	"             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
	"\"$$$$\n"
	"              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
	"              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
	"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
	"              $\"                                                 \"$\n"
	"              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
	"$\"$\"$\"$\"$\"$\"$\"$\n"
	"                                   You win!\n\n";

const char *tombstone =
	"\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
	"               /     /         \\             __\n"
	"              /     /           \\            ||\n"
	"             /____ /   Rest in   \\           ||\n"
	"            |     |    Pieces     |          ||\n"
	"            |     |               |          ||\n"
	"            |     |   A. Luser    |          ||\n"
	"            |     |               |          ||\n"
	"            |     |     * *   * * |         _||_\n"
	"            |     |     *\\/* *\\/* |        | TT |\n"
	"            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
	"| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
	"            |     |         \\/..\"\"\"\"\"...\"\"\""
	"\\ || /.\"\"\".......\"\"\"\"...\n"
	"            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
	"\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
	"            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
	"..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
	"            You're dead to me.  Better luck in the next life.\n\n\n";

void usage(char *name)
{
	printw(stderr,
					"Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
					"          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
					"          [-p|--pc <y> <x>] [-n|--nummon <count>]\n",
					name);

	exit(-1);
}

void climbStairs(dungeon_t *d, int dir)
{
	if((mapxy(d->pc.position[dim_x], d->pc.position[dim_y]) == left_stair && (dir == 1)) || (mapxy(d->pc.position[dim_x], d->pc.position[dim_y])== right_stair && (dir == 0)))
	{
			uint32_t direct_kills = d->pc.kills[kill_direct];
			uint32_t vengence_kills = d->pc.kills[kill_avenged];

			init_dungeon(d);
			gen_dungeon(d);
			config_pc(d);

			d->pc.kills[kill_direct] = direct_kills;
			d->pc.kills[kill_avenged] = vengence_kills;

			gen_monsters(d);
			gen_stairs(d, 10);

			return 0;

	}
	else
	{
		mvprintw(0,0, "Error: There are no stairs to climb.");
	}
}



int main(int argc, char *argv[])
{
	dungeon_t d;
	time_t seed;
	struct timeval tv;
	uint32_t i;
	uint32_t do_load, do_save, do_seed, do_image, do_place_pc;
	uint32_t long_arg;
	char *save_file;
	char *load_file;
	char *pgm_file;

	initscr(); // Start ncurses mode

	memset(&d, 0, sizeof (d));

	/* Default behavior: Seed with the time, generate a new dungeon, *
	 * and don't write to disk.                                      */
	do_load = do_save = do_image = do_place_pc = 0;
	do_seed = 1;
	save_file = load_file = NULL;
	d.max_monsters = MAX_MONSTERS;

	/* The project spec requires '--load' and '--save'.  It's common  *
	 * to have short and long forms of most switches (assuming you    *
	 * don't run out of letters).  For now, we've got plenty.  Long   *
	 * forms use whole words and take two dashes.  Short forms use an *
		* abbreviation after a single dash.  We'll add '--rand' (to     *
	 * specify a random seed), which will take an argument of it's    *
	 * own, and we'll add short forms for all three commands, '-l',   *
	 * '-s', and '-r', respectively.  We're also going to allow an    *
	 * optional argument to load to allow us to load non-default save *
	 * files.  No means to save to non-default locations, however.    *
	 * And the final switch, '--image', allows me to create a dungeon *
	 * from a PGM image, so that I was able to create those more      *
	 * interesting test dungeons for you.                             */

 	if (argc > 1) {
		for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
			if (argv[i][0] == '-') { /* All switches start with a dash */
				if (argv[i][1] == '-') {
					argv[i]++;    /* Make the argument have a single dash so we can */
					long_arg = 1; /* handle long and short args at the same place.  */
				}
				switch (argv[i][1]) {
				case 'r':
					if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-rand")) ||
							argc < ++i + 1 /* No more arguments */ ||
							!sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
						usage(argv[0]);
					}
					do_seed = 0;
					break;
				case 'l':
					if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-load"))) {
						usage(argv[0]);
					}
					do_load = 1;
					if ((argc > i + 1) && argv[i + 1][0] != '-') {
						/* There is another argument, and it's not a switch, so *
						 * we'll treat it as a save file and try to load it.    */
						load_file = argv[++i];
					}
					break;
				case 's':
					if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-save"))) {
						usage(argv[0]);
					}
					do_save = 1;
					if ((argc > i + 1) && argv[i + 1][0] != '-') {
						/* There is another argument, and it's not a switch, so *
						 * we'll treat it as a save file and try to load it.    */
						save_file = argv[++i];
					}
					break;
				case 'i':
					if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-image"))) {
						usage(argv[0]);
					}
					do_image = 1;
					if ((argc > i + 1) && argv[i + 1][0] != '-') {
						/* There is another argument, and it's not a switch, so *
						 * we'll treat it as a save file and try to load it.    */
						pgm_file = argv[++i];
					}
					break;
				case 'n':
					if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-nummon")) ||
							argc < ++i + 1 /* No more arguments */ ||
							!sscanf(argv[i], "%hu", &d.max_monsters)) {
						usage(argv[0]);
					}
					break;
				case 'p':
					if ((!long_arg && argv[i][2])            ||
							(long_arg && strcmp(argv[i], "-pc")) ||
							argc <= i + 2                        ||
							argv[i + 1][0] == '-'                ||
							argv[i + 2][0] == '-') {
						usage(argv[0]);
					}
					do_place_pc = 1;
					if ((d.pc.position[dim_y] = atoi(argv[++i])) < 1 ||
							d.pc.position[dim_y] > DUNGEON_Y - 2         ||
							(d.pc.position[dim_x] = atoi(argv[++i])) < 1 ||
							d.pc.position[dim_x] > DUNGEON_X - 2) {
						fprintf(stderr, "Invalid PC position.\n");
						usage(argv[0]);
					}
					break;
				default:
					usage(argv[0]);
				}
			} else { /* No dash */
				usage(argv[0]);
			}
		}
	}

	if (do_seed) {
		/* Allows me to generate more than one dungeon *
		 * per second, as opposed to time().           */
		gettimeofday(&tv, NULL);
		seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
	}

	printf("Seed is %ld.\n", seed);
	srand(seed);

	init_dungeon(&d);

	if (do_load) {
		read_dungeon(&d, load_file);
	} else if (do_image) {
		read_pgm(&d, pgm_file);
	} else {
		gen_dungeon(&d);
	}

	config_pc(&d);
	gen_monsters(&d);

	gen_stairs(&d, 40);

	pair_t next;

	int isViewing = false;

	while (pc_is_alive(&d) && dungeon_has_npcs(&d)) {

		if(isViewing)
		{
			look_render(&d);
		}
		else
		{
			render_dungeon(&d);			
		}

		switch(getch())
		{
			case '7': // top-left
			case 'y':
				next[dim_x] = d.pc.position[dim_x] - 1;
				next[dim_y] = d.pc.position[dim_y] - 1;

				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}

				break;
			
			case '8': // up
			case 'k':
				next[dim_x] = d.pc.position[dim_x];
				next[dim_y] = d.pc.position[dim_y] - 1;
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}else
				{
					viewCenter[dim_x] = viewCenter[dim_x];
					viewCenter[dim_y] = viewCenter[dim_y] - 1;
				}

				break;
			
			case '9':
			case 'u':
				next[dim_x] = d.pc.position[dim_x] + 1;
				next[dim_y] = d.pc.position[dim_y] -1;
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}

				break;
			
			case '6':
			case 'l':
				next[dim_x] = d.pc.position[dim_x] +1;
				next[dim_y] = d.pc.position[dim_y];
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}else
				{
					viewCenter[dim_x] = viewCenter[dim_x] +1;
					viewCenter[dim_y] = viewCenter[dim_y];
		
				}

				break;
			
			case '3':
			case 'n':
				next[dim_x] = d.pc.position[dim_x] +1;
				next[dim_y] = d.pc.position[dim_y] +1;
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}
				break;
			
			case '2':
			case 'j':
				next[dim_x] = d.pc.position[dim_x];
				next[dim_y] = d.pc.position[dim_y] +1;
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}else
				{
					viewCenter[dim_x] = viewCenter[dim_x];
					viewCenter[dim_y] = viewCenter[dim_y] +1;
				}

				break;
			
			case '1':
			case 'b':
				next[dim_x] = d.pc.position[dim_x] -1;
				next[dim_y] = d.pc.position[dim_y] + 1;
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}

				break;
			
			case '4':
			case 'h':
				next[dim_x] = d.pc.position[dim_x] -1;
				next[dim_y] = d.pc.position[dim_y];
				
				if(d.map[next[dim_y]][next[dim_x]] != ter_wall && d.map[next[dim_y]][next[dim_x]] != ter_wall_immutable && !isViewing)
				{
					move_character(&d, &d.pc, next);
				}else
				{
					viewCenter[dim_x] = viewCenter[dim_x] -1;
					viewCenter[dim_y] = viewCenter[dim_y];
		
				}

				break;
			
			case '5':
			case 42:
				break;
			case '.':
			case '>':
				climbStairs(&d, 0); //Go down
				break;
			case ',':
			case '<':
				climbStairs(&d, 1); // Go up
				break;

			case 'L':
				// Enter look mode
				isViewing = true;
				mvprintw(0,0, "Now in look mode.	");
				viewCenter[dim_y] = d.pc.position[dim_y];
				viewCenter[dim_x] = d.pc.position[dim_x];
				break;
			// Turn into escape key
			case (27):
				isViewing = false;
				mvprintw(0,0, "Now in control mode.");

				break;
			case 'Q':
			case 'q':
				clear();
				mvprintw(10, 17, "Woah, why'd you quit? Was I not good enough!?");
				mvprintw(11, 21, "Pfft. My mother was right about you...");
				goto END;
				break;
			case '`':
				render_all(&d);
			default:
				break;
		}
		refresh();

		if(!isViewing)
		{
			do_moves(&d);
		}
	}

	clear();

	mvprintw(0,0, pc_is_alive(&d) ? victory : tombstone);

	END: 

	if (do_save) {
		write_dungeon(&d, save_file);
	}

	mvprintw(20, 0, "\nYou defended your life in the face of %u deadly beasts.\n"
				 "You avenged the cruel and untimely murders of %u peaceful dungeon residents.\n Click any key to close...\n",
				 d.pc.kills[kill_direct], d.pc.kills[kill_avenged]);

	pc_delete(d.pc.pc);

	delete_dungeon(&d);

	getch();
	endwin(); // Close the window

	return 0;
}

