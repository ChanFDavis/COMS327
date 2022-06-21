#include <stdlib.h>
#include <ncurses.h>
#include <string>
#include <vector>

#include "dungeon.h"
#include "pc.h"
#include "utils.h"
#include "move.h"
#include "path.h"
#include "io.h"
#include "object.h"

/* TODO: Fix indentation. */

void pc_delete(pc *pc)
{
	if (pc) {
		free(pc);
	}
}

uint32_t pc_is_alive(dungeon_t *d)
{
	return d->PC->alive;
}

void place_pc(dungeon_t *d)
{
	character_set_y(d->PC, rand_range(d->rooms->position[dim_y],
		(d->rooms->position[dim_y] +
			d->rooms->size[dim_y] - 1)));
	character_set_x(d->PC, rand_range(d->rooms->position[dim_x],
		(d->rooms->position[dim_x] +
			d->rooms->size[dim_x] - 1))); 
	io_calculate_offset(d);
	io_update_offset(d);

	pc_init_known_terrain(d->PC);

	io_display(d);
}

void config_pc(dungeon_t *d)
{
	static dice pc_dice(0, 1, 4);

	d->PC = new pc;

	d->PC->symbol = '@';

	place_pc(d);

	d->PC->hp = PC_HEALTH;
	d->PC->speed = PC_SPEED;
	d->PC->alive = 1;
	d->PC->sequence_number = 0;
	d->PC->kills[kill_direct] = d->PC->kills[kill_avenged] = 0;
	d->PC->color.push_back(COLOR_WHITE);
	d->PC->damage = &pc_dice;
	d->PC->name = "Isabella Garcia-Shapiro";

	d->character_map[character_get_y(d->PC)][character_get_x(d->PC)] = d->PC;

	dijkstra(d);
	dijkstra_tunnel(d);

	io_calculate_offset(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t dir)
{
	static uint32_t have_seen_corner = 0;
	static uint32_t count = 0;

	dir[dim_y] = dir[dim_x] = 0;

	if (in_corner(d, d->PC)) {
		if (!count) {
			count = 1;
		}
		have_seen_corner = 1;
	}

	/* First, eat anybody standing next to us. */
	if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC) - 1)) {
		dir[dim_y] = -1;
		dir[dim_x] = -1;
	} else if (charxy(character_get_x(d->PC), character_get_y(d->PC) - 1)) {
		dir[dim_y] = -1;
	} else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC) - 1)) {
		dir[dim_y] = -1;
		dir[dim_x] = 1;
	} else if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC))) {
		dir[dim_x] = -1;
	} else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC))) {
		dir[dim_x] = 1;
	} else if (charxy(character_get_x(d->PC) - 1, character_get_y(d->PC) + 1)) {
		dir[dim_y] = 1;
		dir[dim_x] = -1;
	} else if (charxy(character_get_x(d->PC), character_get_y(d->PC) + 1)) {
		dir[dim_y] = 1;
	} else if (charxy(character_get_x(d->PC) + 1, character_get_y(d->PC) + 1)) {
		dir[dim_y] = 1;
		dir[dim_x] = 1;
	} else if (!have_seen_corner || count < 250) {
		/* Head to a corner and let most of the NPCs kill each other off */
		if (count) {
			count++;
		}
		if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111)) {
			dir[dim_x] = (rand() % 3) - 1;
			dir[dim_y] = (rand() % 3) - 1;
		} else {
			dir_nearest_wall(d, d->PC, dir);
		}
	}else {
		/* And after we've been there, let's head toward the center of the map. */
		if (!against_wall(d, d->PC) && ((rand() & 0x111) == 0x111)) {
			dir[dim_x] = (rand() % 3) - 1;
			dir[dim_y] = (rand() % 3) - 1;
		} else {
			dir[dim_x] = ((character_get_x(d->PC) > DUNGEON_X / 2) ? -1 : 1);
			dir[dim_y] = ((character_get_y(d->PC) > DUNGEON_Y / 2) ? -1 : 1);
		}
	}

	/* Don't move to an unoccupied location if that places us next to a monster */
	if (!charxy(character_get_x(d->PC) + dir[dim_x],
		character_get_y(d->PC) + dir[dim_y]) &&
		((charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y] - 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y]) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y]) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y] + 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] - 1,
			character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x],
			character_get_y(d->PC) + dir[dim_y] - 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x],
			character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x],
			character_get_y(d->PC) + dir[dim_y] + 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x],
			character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y] - 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y] - 1) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y]) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y]) != d->PC)) ||
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y] + 1) &&
		(charxy(character_get_x(d->PC) + dir[dim_x] + 1,
			character_get_y(d->PC) + dir[dim_y] + 1) != d->PC)))) {
		dir[dim_x] = dir[dim_y] = 0;
	}

	return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
	if ((room < d->num_rooms)                                     &&
		(character_get_x(d->PC) >= d->rooms[room].position[dim_x]) &&
		(character_get_x(d->PC) < (d->rooms[room].position[dim_x] +
			d->rooms[room].size[dim_x]))    &&
		(character_get_y(d->PC) >= d->rooms[room].position[dim_y]) &&
		(character_get_y(d->PC) < (d->rooms[room].position[dim_y] +
			d->rooms[room].size[dim_y]))) {
		return 1;
}

return 0;
}

void pc_learn_terrain(pc *p, pair_t pos, terrain_type_t ter)
{
	p->known_terrain[pos[dim_y]][pos[dim_x]] = ter;
	p->visible[pos[dim_y]][pos[dim_x]] = 1;
}

void pc_reset_visibility(pc *p)
{
	uint32_t y, x;

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			p->visible[y][x] = 0;
		}
	}
}

terrain_type_t pc_learned_terrain(pc *p, int16_t y, int16_t x)
{
	if (y < 0 || y >= DUNGEON_Y || x < 0 || x >= DUNGEON_X) {
		io_queue_message("Invalid value to %s: %d, %d", __FUNCTION__, y, x);
	}

	return p->known_terrain[y][x];
}

void pc_init_known_terrain(pc *p)
{
	uint32_t y, x;

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			p->known_terrain[y][x] = ter_unknown;
			p->visible[y][x] = 0;
		}
	}
}

void pc_observe_terrain(pc *p, dungeon_t *d)
{
	pair_t where;
	int16_t y_min, y_max, x_min, x_max;

	y_min = p->position[dim_y] - PC_VISUAL_RANGE;
	if (y_min < 0) {
		y_min = 0;
	}
	y_max = p->position[dim_y] + PC_VISUAL_RANGE;
	if (y_max > DUNGEON_Y - 1) {
		y_max = DUNGEON_Y - 1;
	}
	x_min = p->position[dim_x] - PC_VISUAL_RANGE;
	if (x_min < 0) {
		x_min = 0;
	}
	x_max = p->position[dim_x] + PC_VISUAL_RANGE;
	if (x_max > DUNGEON_X - 1) {
		x_max = DUNGEON_X - 1;
	}

	for (where[dim_y] = y_min; where[dim_y] <= y_max; where[dim_y]++) {
		where[dim_x] = x_min;
		can_see(d, p->position, where, 1, 1);
		where[dim_x] = x_max;
		can_see(d, p->position, where, 1, 1);
	}
	/* Take one off the x range because we alreay hit the corners above. */
	for (where[dim_x] = x_min - 1; where[dim_x] <= x_max - 1; where[dim_x]++) {
		where[dim_y] = y_min;
		can_see(d, p->position, where, 1, 1);
		where[dim_y] = y_max;
		can_see(d, p->position, where, 1, 1);
	}       
}

int32_t is_illuminated(pc *p, int16_t y, int16_t x)
{
	return p->visible[y][x];
}

void pc_see_object(character *the_pc, object *o)
{
	if (o) {
		o->has_been_seen();
	}
}

/* New code! */

//Cases:
//1. Item slot is empty
//2. Item in given slot is not wearable
//3. There is something in that item slot
//4. There is nothing in the equipment slot for that kind of item
void equip_item(pc *pc, int16_t item_slot)
{
	object *o = pc->inventory[item_slot];
	object *temp;

	if(!pc->inventory[item_slot]) //Case 1
	{
		io_queue_message("There is not an item in given slot: %d. Please try the command again.", item_slot);
	}
	else if(is_wearable(o)) //Case 2
	{
		if(pc->equipment[o->get_type()]) //Case 3
		{
			temp = pc->equipment[o->get_type()];
			pc->equipment[o->get_type()] = o;
			pc->inventory[item_slot] = temp;

			io_queue_message("You've swapped out the select item with the one you were wearing");

		}
		else //Case 2
		{
			pc->equipment[o->get_type()] = pc->inventory[item_slot];
			pc->inventory[item_slot] = NULL;
			io_queue_message("You've equipped the item from your inventory");
		}
	}
	else
	{
		io_queue_message("You can't equip the selected item");

	}

}

void unequip_item(pc *pc, int16_t equipment_slot)
{
	int16_t empty_slot = has_empty_slot(pc);

	if(!pc->equipment[equipment_slot])
	{
		io_queue_message("There is no item in that slot to unequip!");
	}
	else if(empty_slot >= 0)
	{
		pc->inventory[empty_slot] = pc->equipment[equipment_slot];
		pc->equipment[equipment_slot] = NULL;
		io_queue_message("You've unequipped the selected item and it's now in your inventory");
	}
	else
	{
		io_queue_message("There is no room in your inventory for you for you to put the item you want to unequip");
	}

}

void drop_item(dungeon_t *d, int16_t item_slot)
{
	if(d->PC->inventory[item_slot])
	{
		if(objpair(d->PC->position))
		{
			io_queue_message("Sorry, but there is already an item on the floor. Please find an empty spot.");
		}
		else
		{
			objpair(d->PC->position) = d->PC->inventory[item_slot];
			d->PC->inventory[item_slot] = NULL;
			io_queue_message("You have dropped the item on the floor");
		}
	}
	else
	{
		io_queue_message("There is no item in the selected slot to drop.");
	}
}

void list_inventory(pc *pc)
{
	uint16_t i;
	clear();
	for(i = 0; i < arr_size(pc->inventory); i++)
	{
		if(pc->inventory[i])
		{
			mvprintw(i+1, 0, "%d : %s", i, pc->inventory[i]->get_name());
		}
		else
		{
			mvprintw(i+1, 0, "%d : EMPTY", i);

		}
	}

}

void list_equipment(pc *pc)
{
	uint16_t i;

	std::string equipment_slot_names[] = {"WEAPON", "OFFHAND", "RANGED", "ARMOR", "HELMET", "CLOAK", "GLOVES", "BOOTS", "AMULET", "LIGHT", "RINGR", "RINGL"};
	std::string equipment_indices = "abcdefghijkl";

	clear();
	for(i = 0; i < arr_size(pc->equipment); i++)
	{
		if(pc->equipment[i])
		{
			mvprintw(i+1, 0, "%s (%c) : %s", equipment_slot_names[i].c_str(), equipment_indices[i], pc->equipment[i]->get_name());
		}
		else
		{
			mvprintw(i+1, 0, "%s (%c) : EMPTY", equipment_slot_names[i].c_str(), equipment_indices[i]);
		}
	}
}

bool is_wearable(object *o)
{
	switch(o->get_type())
	{
		case objtype_WEAPON:
		case objtype_OFFHAND:
		case objtype_RANGED:
		case objtype_LIGHT:
		case objtype_ARMOR:
		case objtype_HELMET:
		case objtype_CLOAK:
		case objtype_GLOVES:
		case objtype_BOOTS:
		case objtype_AMULET:
		case objtype_RING:
		case objtype_AMMUNITION:
		case objtype_WAND:
			return true;
		case objtype_CONTAINER:
			mvprintw(0, 0, "You try to wear the container, but you fail. Grow up.");
			return false;
		default:
			mvprintw(0, 0, "That item is not wearable. Please try the command again.");
			return false;
	}
}

int16_t has_empty_slot(pc *pc)
{
	uint16_t i;

	for (i = 0; i < sizeof(pc->inventory); i++)
	{
		if(!pc->inventory[i])
		{
			return i;
		}
	}

	return -1;
}


