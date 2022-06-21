#include <stdio.h>

char currWorld[80][24];
char nextWorld[80][24];

int main(int argc, char *argv[])
{

  /* These are re-used through the code */ 
  int i; /*Row*/
  int j; /*Column*/

  char curr;

  int neighbors;

  /* These nested for-loops updates each grid space in currWorld and places the result in
  nextWorld */
  for(i = 0; i < 80; i++)
  {
    for(j = 0; j < 24; j++)
    {
      neighbors = getNeighbors(i, j);
      curr = currWorld[i][j];
      
      if(curr == 'O')
      {
        if(neighbors < 2 || neighbors > 3)
        {
          /* Dead */
          nextWorld[i][j] = ' ';
        }
      }
      else if(neighbors == 3)
      {
        /* Keeps Living */
        nextWorld[i][j] = 'O';
      }
    }
  }
  
  return 0; 
}

int getNeighbors(int y, int x)
{
  int numNeighbors = 0;
  int i;
  int j;

  int tempI;
  int tempJ;


  for(i = y-1; i <= y + 1; i++)
  {
    for(j = x-1; j <= x + 1; j++)
    {

        /* These Switch statements check if the currently selected grid spot is on an edge
     this prevents any sort of invalid index errors*/
      switch(i)
      {
        case -1:
          tempI = 23;
          break;
        case 24:
          tempI = 0;
          break; 
        default:
          tempI = i;
      }

      switch(j)
      {
        case -1:
          tempJ = 79;
          break;
        case 80:
          tempJ = 0;
          break; 
        default:
          tempJ = j;
      }

      /* Make sure to not count the current grid space as a neighbor to itself. */
      if((i != y || j != x) && currWorld[i][j] == 'O') 
      {
        numNeighbors++;
      }
    }
  }

  return numNeighbors;
}

/*
Check each space in the current world and update the next world accordingly.

 */