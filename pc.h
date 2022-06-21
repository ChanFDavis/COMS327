#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"
# include "character.h"
# include "dungeon.h"

/* TODO: Fix indentation. */

typedef enum equipment_slots
{
	WEAPON,
	OFFHAND,
	RANGED,
	ARMOR,
	HELMET,
	CLOAK,
	GLOVES,
	BOOTS,
	AMULET,
	LIGHT,
	RINGR,
	RINGL
} equipment_slots_t;

class pc : public character {
 public:
  terrain_type_t known_terrain[DUNGEON_Y][DUNGEON_X];
  unsigned char visible[DUNGEON_Y][DUNGEON_X];
  object *inventory[10];
  object *equipment[12];
};

void pc_delete(pc *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);
void pc_learn_terrain(pc *p, pair_t pos, terrain_type_t ter);
terrain_type_t pc_learned_terrain(pc *p, int16_t y, int16_t x);
void pc_init_known_terrain(pc *p);
void pc_observe_terrain(pc *p, dungeon_t *d);
int32_t is_illuminated(pc *p, int16_t y, int16_t x);
void pc_reset_visibility(pc *p);
void pc_see_object(character *the_pc, object *o);

// New Code!
void equip_item(pc *pc, int16_t item_slot);
void unequip_item(pc *pc, int16_t equipment_slot);
void drop_item(dungeon_t *d, int16_t item_slot);
void list_inventory(pc *pc);	
void list_equipment(pc *pc);
int16_t has_empty_slot(pc *pc);
bool is_wearable(object *o);


#endif
