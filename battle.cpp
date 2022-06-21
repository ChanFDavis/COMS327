   #include "battle.h"

   void battle(dungeon_t *d, character *atk, character *def) // Returns 0 if player doesn't die and 0 if they do.
   {
      uint32_t damage, i;

      if (character_is_alive(def)) {
        if (atk != d->PC) {
          damage = atk->damage->roll();
          io_queue_message("The %s hits you for %d.", atk->name, damage);
        } else {
          for (i = damage = 0; i < num_eq_slots; i++) {
            if (i == eq_slot_weapon && !d->PC->eq[i]) {
              damage += atk->damage->roll();
            } else if (d->PC->eq[i]) {
              damage += d->PC->eq[i]->roll_dice();
            }
          }
          io_queue_message("You hit the %s for %d.", def->name, damage);
        }

        if (damage >= def->hp) {
          if (atk != d->PC) {
            io_queue_message("You die.");
            io_queue_message(""); /* Extra message to force pause on "more" */
          } else {
            io_queue_message("The %s dies.", def->name);
          }
          def->hp = 0;
          def->alive = 0;
          character_increment_dkills(atk);
          character_increment_ikills(atk, (character_get_dkills(def) +
                                           character_get_ikills(def)));
          if (def != d->PC) {
            d->num_monsters--;
          }
          charpair(def->position) = NULL;
        } else {
          def->hp -= damage;
        }

        if (def != d->PC) {
          d->num_monsters--;
        }
      }
   }

   void displayBattle()
   {

   }

   void diplayOptions()
   {

   }

   void displayEnemy(char symbol)
   {

   }	
