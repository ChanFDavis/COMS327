#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "utils.h"

using namespace std;

char* metadata;

int main(int argc, char const *argv[])
{

	ifstream f("monster_desc.txt");

	string str; // Current string at any time.
	string name;
	string desc;

	vector<string> colors;
	vector<string> abilities;
	vector<string> dice; // [base, dice, sides]
	string speed;
	string health;
	string damage;
	char symbol;

	// NOTE: Wasn't clear if you actually wanted us to save anything as integers or whether or not we should find
	// the values of the dice rolls.

	int param_count = 0;

	if(f.good())
	{
		getline(f, str);
		getline(f, str);
	}

	while(f.good()) //While the file still has lines to parse through.
	{
		getline(f, str);

		if(str.compare("BEGIN MONSTER") == 0) // Start a new monster.
		{

			name.clear();
			desc.clear();
			colors.clear();
			abilities.clear();
			dice.clear();
			speed.clear();
			health.clear();
			damage.clear();
			symbol = ' ';
			param_count = 0;

			while(str.compare("END") != 0) // Keep looping till it hits the end		
			{
				getline(f, str);

				if(str.substr(0,4).compare("NAME") == 0)
				{
					name = str.substr(5, str.length());

					param_count++;
				} 
				else if(str.substr(0,4).compare("DESC") == 0)
				{
					getline(f, str); // Get the line after the keyword.

					while(str.length() != 1 && str[0] != '.')
					{
						desc += str + "\n";
						getline(f, str);
					}

					param_count++;
				} 
				else if(str.substr(0,5).compare("COLOR") == 0)
				{
					istringstream iss(str);
					string word;

					while(iss >> word)
					{
						if(word.compare("COLOR") != 0)
						{
							colors.push_back(word);
						}
					}

					param_count++;
				} 
				else if(str.substr(0,5).compare("SPEED") == 0)
				{


					speed = str.substr(6, str.length());

					param_count++;

				} 
				else if(str.substr(0,4).compare("SYMB") == 0)
				{


					symbol = str[5];

					param_count++;

				} 
				else if(str.substr(0,4).compare("ABIL") == 0)
				{

					istringstream iss(str);
					string word;

					while(iss >> word)
					{
						if(word.compare("ABIL") != 0)
						{
							abilities.push_back(word);
						}
					}

					param_count++;
				} 
				else if(str.substr(0,2).compare("HP") == 0)
				{

					health = str.substr(3, str.length());


					param_count++;
				} 
				else if(str.substr(0,3).compare("DAM") == 0)
				{

					damage = str.substr(4, str.length());


					param_count++;
				}
			}

			if(param_count == 8 && name.length() > 0 && desc.length() > 0 && symbol != ' ' && speed.length() > 0 && health.length() > 0 && damage.length() > 0 && colors.size() > 0 && abilities.size() > 0)
			{
				int i;

				cout << name << endl;
				cout << desc;
				cout << symbol << endl;
				for (i = 0; i < (int)colors.size(); ++i)
				{
					cout << colors[i] << " ";
				}
				cout << endl;
				cout << speed << endl;
				for (i = 0; i < (int)abilities.size(); ++i)
				{
					cout << abilities[i] << " ";
				}
				cout << endl;
				cout << health << endl;
				cout << damage << endl;
				cout << endl;
			}
			else
			{
				cout << endl << "ERROR: Monster thrown because it wasn't spicy enough for this sauce" << endl;
			}

		}

		// If BEGIN MONSTER has not been found, keep going down the file.

	}

	return 0;
}

