/*    SPDX-License-Identifier: MIT    */

#include "game_entity.hpp"
#include "game_map.hpp"

#include <random>
#include <array>
#include <algorithm>

namespace
{
	// We only want so-many enemies in a room, max.
	constexpr auto max_enemies_per_room = 5;

	// Visual representation of each type of character we have
	// Order maps to entity_type enum.
	const auto entity_faces = std::array{
		std::pair{ '@', TCODColor{ 255, 255, 255 } },
		std::pair{ 'o', TCODColor{ 255, 197,   0 } },
		std::pair{ 'g', TCODColor{  18, 102,   0 } },
	};
}

void game_entity::move_by(const position &offset)
{
	pos.x += offset.x;
	pos.y += offset.y;
}

auto game_entity::face() const -> std::pair<char, TCODColor>
{
	// Just return the face from our list of faces at index
	return entity_faces.at(static_cast<int>(type));
}

auto generate_enemies(const std::vector<room> &rooms) -> std::vector<game_entity>
{
	// List of enemies to return
	auto enemies = std::vector<game_entity>{};

	std::random_device rd{};      // Random number generator
	std::mt19937 gen(rd());       // generation algorithm we want to use
	std::uniform_int_distribution d_en(0, max_enemies_per_room); // distribution for number of enemies per room.
	std::bernoulli_distribution d_et(0.8); // goblins or ogres distribution. 80% goblins, 20% ogres

	// Lambda will add to enemies list for a given room
	auto place_enemies_in_room = [&](const room &r, const int count)
	{
		// thing to keep in mind, for a given room
		// ######  the enemies can only exist in the
		// #    #  blank spaces inside. But the room's
		// #    #  dimensions include the walls.
		// ######  Hence why x1 and y1 are +1, and x2 and y2 are -2.
		auto x1 = r.p.x + 1, x2 = r.p.x + r.size.width - 2;
		auto y1 = r.p.y + 1, y2 = r.p.y + r.size.height - 2;
		std::uniform_int_distribution d_x(x1, x2), // X and Y distribution
		                              d_y(y1, y2); // for enemies
		
		for (auto i = enemies.size();        // Take enemy count till now
		     enemies.size() < (i + count); ) // loop till it's increased by count.
		{
			int x = d_x(gen), // x and y coordinates
			    y = d_y(gen); // to put the enemy at.
			
			// Make sure no other enemies are in the same spot.
			auto exists = std::find_if(std::begin(enemies), std::end(enemies), [&](const auto &e)
			{
				return (x == e.pos.x) and (x == e.pos.y);
			});

			// If there are no others add to the list.
			if (exists == std::end(enemies))
			{
				// Figure out what's the type of this enemy.
				auto type = d_et(gen) ? entity_type::ogre : entity_type::goblin;
				enemies.push_back({
					{x, y},
					type
				});
			}
		}
	};

	// Loop through all the rooms
	for (auto &r : rooms)
	{
		// For this room generate random number of enemies.
		place_enemies_in_room(r, d_en(gen));
	}

	return enemies;
}