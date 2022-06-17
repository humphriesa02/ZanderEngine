#include "Scene_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "Game_Engine.h"
#include "Components.h"
#include "Action.h"

Scene_Play::Scene_Play(Game_Engine *game_engine, const std::string &level_path)
	: Scene(game_engine)
	, m_level_path(level_path)
{
	initialize(m_level_path);
}

void Scene_Play::initialize(const std::string &level_path)
{
	register_action(sf::Keyboard::H, "CONSOLE");
	register_action(sf::Keyboard::P, "PAUSE");
	register_action(sf::Keyboard::Escape, "QUIT");
	register_action(sf::Keyboard::T, "TOGGLE_TEXTURE");		// Toggle drawing (T)extures
	register_action(sf::Keyboard::C, "TOGGLE_COLLISION");	// Toggle drawing (C)ollision Boxes
	register_action(sf::Keyboard::G, "TOGGLE_GRID");		// Toggle drawing (G)rid

	register_action(sf::Keyboard::D, "RIGHT");				// Toggle the player's right input
	register_action(sf::Keyboard::A, "LEFT");
	register_action(sf::Keyboard::W, "UP");
	register_action(sf::Keyboard::S, "DOWN");

	m_grid_text.setCharacterSize(12);
	m_grid_text.setFont(m_game->assets().get_font("Arial"));

	load_level(level_path);
}

c_Vec2 Scene_Play::grid_to_mid_pixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	// TODO: This function takes in a grid (x,y) position and an Entity
	//		 Return a c_Vec2 indicating where the CENTER position of the Entity should be
	//		 You must use the Entity's Animation size to position it correctly
	//		 The size of the grid widht and height is stored in m_grid_size.x and m_grid_size.y
	//		 The bottom-left corner of the Animation should align with the bottom left of the grid cell
	auto& entityAnimation = entity->get_component<c_Animation>();

	// Our grid is built of m_grid_size.x by m_grid_size.y blocks
	// we need to specify how big our entity needs to be drawn, and then its position

	float relativeX = gridX * m_grid_size.x;
	float relativeY = abs((gridY - 11) * m_grid_size.y);
	printf("RELATIVE X: %f", relativeX);
	printf("RELATIVE Y: %f", relativeY);

	return c_Vec2(relativeX + (entityAnimation.animation.get_size().x/2), relativeY + (entityAnimation.animation.get_size().y/2));
}

void Scene_Play::load_level(const std::string &file_name)
{
	// reset the entity manager every time we load a level
	m_entity_manager= Entity_Manager();

	// TODO: read in the level file and add the appropriate entities
	//		 use the PlayerConfig struct m_player_config to store player properties
	//		 this struct is defined at the top of Scene_Play.h

	std::ifstream file(file_name);
	std::string element;
	while (file >> element)
	{

		if (element == "Player")
		{
			file >> m_player_config.X >> m_player_config.Y >> m_player_config.CX >> m_player_config.CY >> m_player_config.SPEED >> m_player_config.JUMP >> m_player_config.MAXSPEED >> m_player_config.GRAVITY >> m_player_config.WEAPON;
		}
		else if (element == "Tile")
		{
			std::string animationName;
			size_t grid_x, grid_y;
			file >> animationName >> grid_x >> grid_y;
			auto tile = m_entity_manager.add_entity(e_Tag::Tile);
			tile->add_component<c_Animation>(m_game->assets().get_animation(animationName), true);
			tile->add_component<c_Transform>(grid_to_mid_pixel(grid_x, grid_y, tile));
			tile->add_component<c_Bounding_box>(m_game->assets().get_animation(animationName).get_size());
		}
		else if (element == "Dec")
		{
			std::string animationName;
			size_t grid_x, grid_y;
			file >> animationName >> grid_x >> grid_y;
			auto tile = m_entity_manager.add_entity(e_Tag::Tile);
			tile->add_component<c_Animation>(m_game->assets().get_animation(animationName), true);
			tile->add_component<c_Transform>(grid_to_mid_pixel(grid_x, grid_y, tile));
		}
		else
		{
			std::cerr << "Unknown Asset Type: " << element << std::endl;
		}
	}

	// NOTE: all of the code below is sample code which shows you how to
	//		 set up and use entities with the new syntax, it should be removed

	spawn_player();

	// some sample entities 
	auto brick= m_entity_manager.add_entity(e_Tag::Tile);
	// IMPORTANT: always add the c_Animation component first so that grid_to_mid_pixel can compute correctly
	brick->add_component<c_Animation>(m_game->assets().get_animation("Brick"), true);
	brick->add_component<c_Transform>(c_Vec2(96, 480));
	// NOTE: Your final code should position the entity with the grid x,y position read from the file
	// brick->add_component<c_Transform>(grid_to_mid_pixel(grid_x, grid_y, brick);

	if (brick->get_component<c_Animation>().animation.get_name() == "Brick")
	{
		std::cout << "This could be a good way of identifying if a tile is a brick!\n";
	}  

	auto block= m_entity_manager.add_entity(e_Tag::Tile);
	block->add_component<c_Animation>(m_game->assets().get_animation("Block"), true);
	block->add_component<c_Transform>(c_Vec2(224, 480));
	// add a bounding box, this will now show up if we press the 'C' key
	block->add_component<c_Bounding_box>(m_game->assets().get_animation("Block").get_size());

	auto question= m_entity_manager.add_entity(e_Tag::Tile);
	question->add_component<c_Animation>(m_game->assets().get_animation("Question"), true);
	question->add_component<c_Transform>(c_Vec2(352, 480));

	// NOTE: THIS IS INCREDIBLY IMPORTANT PLEASE READ THIS EXAMPLE
	//		 Componenets are now returned as references rather than pointers
	//		 If you do not specify a reference variable type, it will COPY the component
	//		 Here is an example:
	//
	//		 This will COPY the transform into the variable 'transform1' - it is INCORRECT
	//		 Any changes you make to transform1 will not be changed inside the entity
	//		 auto transform1= entity->get<c_Transform>()
	//
	//		 This will REFERENCE the transform with the variable 'transform2' - it is CORRECT
	//		 Now any changes you make to transform2 will be changed inside the entity
	//		 auto &transform2= entity->get<c_Transform>()
}

void Scene_Play::spawn_player()
{
	// here is a sample player entity which you can use to construct other entities
	m_player= m_entity_manager.add_entity(e_Tag::Player);
	m_player->add_component<c_Animation>(m_game->assets().get_animation("Stand"), true);
	m_player->add_component<c_Transform>(grid_to_mid_pixel(m_player_config.X, m_player_config.Y, m_player));
	m_player->add_component<c_Bounding_box>(c_Vec2(m_player_config.CX, m_player_config.CY));

	// TODO: be sure to add the remaining components to the player
}

void Scene_Play::spawn_bullet(std::shared_ptr<Entity> entity)
{
	// TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
}

void Scene_Play::update()
{
	m_entity_manager.update();

	// TODO: implement pause functionality

	s_movement();
	s_lifespan();
	s_collision();
	s_animation();
	s_render();
}

void Scene_Play::s_movement()
{
	// Basic U/D/L/R movement implemented for you
	auto &player_transform= m_player->get_component<c_Transform>();
	auto &player_input= m_player->get_component<c_Input>();

	c_Vec2 player_velocity(0, 0);

	if (player_input.right)	{ player_velocity.x+= 3; }
	if (player_input.left)	{ player_velocity.x-= 3; }
	if (player_input.up)	{ player_velocity.y-= 3; }
	if (player_input.down)	{ player_velocity.y+= 3; }

	player_transform.velocity= player_velocity;
	player_transform.position+= player_transform.velocity;

	// TODO: Implement player movement / jumping based on its c_Input component
	// TODO: Implement gravity's effect on the player
	// TODO: Implement the maximum player speed in both X and Y directions
	// NOTE: Setting an entity's scale.x to -1/1 will make it face to the left/right
}

void Scene_Play::s_lifespan()
{
	// TODO: Check lifespan of entities that have them, and destroy them if they go over
}

void Scene_Play::s_collision()
{
	// REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
	//			 This means jumping will have a negative y-component
	//			 and gravity will have a positive y-component
	//			 Also, something BELOW something else will have a y value GREATER than it
	//			 Also, something ABOVE something else will have a y value LESS than it

	// TODO: Implement Physics::get_overlap() function, use it isnide this function

	// TODO: Implement bullet / tile collision
	//		 Destroy the tile if it has a Brick animation
	// TODO: Implement player / tile collisions and resolution
	//		 Update the c_state component of the player to store whether
	//		 it is currently on the ground or in the air. This will be
	//		 used by the Animation system
	// TODO: Check to see if the player has falled down a hole (y > height())
	// TODO: Don't let the player walk off the left side of the map
}

void Scene_Play::s_do_action(const Action &action)
{
	if (action.type() == "START")
	{
			 if (action.name() == "TOGGLE_TEXTURE")		{ m_draw_textures= !m_draw_textures; }
		else if (action.name() == "TOGGLE_COLLISION")	{ m_draw_collision= !m_draw_collision; }
		else if (action.name() == "TOGGLE_GRID")		{ m_draw_grid= !m_draw_grid; }
		else if (action.name() == "PAUSE")				{ set_paused(); }
		else if (action.name() == "QUIT")				{ on_end(); }
		else if (action.name() == "CONSOLE")			{ std::cout << "Hello, World!\n"; }
		else if (action.name() == "RIGHT")				{ m_player->get_component<c_Input>().right= true; }
		else if (action.name() == "LEFT")				{ m_player->get_component<c_Input>().left= true; }
		else if (action.name() == "UP")					{ m_player->get_component<c_Input>().up= true; }
		else if (action.name() == "DOWN")				{ m_player->get_component<c_Input>().down= true; }
	}
	else if (action.type() == "END")
	{
		if (action.name() == "RIGHT")					{ m_player->get_component<c_Input>().right= false; }
		if (action.name() == "LEFT")					{ m_player->get_component<c_Input>().left= false; }
		if (action.name() == "UP")						{ m_player->get_component<c_Input>().up= false; }
		if (action.name() == "DOWN")					{ m_player->get_component<c_Input>().down= false; }
	}
}

void Scene_Play::s_animation()
{
	// TODO: Complete the Animation class code first

	// Adding a component like this will override the existing component
	/*
	
	m_player->add_component<c_Animation>(...); // creates component

	m_player->add_component<c_Animation>(...); // overrides existing component

	*/

	// TODO: set the animation of the player based on its c_State component
	// TODO: for each entity with an animation, call entity->get_component<c_Animation>().animation.update()
	//		 if the animation is not repeated, and it has ended, destroy the entity
}

void Scene_Play::on_end()
{
	// TODO: When the scene ends, change back to the MENU scene
	m_game->change_scene("MENU", nullptr, true);
}

void Scene_Play::draw_line(const c_Vec2 &p1, const c_Vec2 &p2)
{
	sf::Vertex line[]={ sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
	m_game->window().draw(line, 2, sf::Lines);
}

void Scene_Play::s_render()
{
	// color the background darker so you know that the game is paused
	if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
	else		   { m_game->window().clear(sf::Color(50, 50, 150)); }

	// set the viewpoint of the window to be centered on the player if it's far enough right
	auto &player_position= m_player->get_component<c_Transform>().position;
	float window_center_x= std::max(m_game->window().getSize().x / 2.0f, player_position.x);
	sf::View view= m_game->window().getView();
	view.setCenter(window_center_x, m_game->window().getSize().y - view.getCenter().y);
	m_game->window().setView(view);

	// draw all Entity textures / animations
	if (m_draw_textures)
	{
		for (auto e : m_entity_manager.get_entities())
		{
			auto &transform= e->get_component<c_Transform>();

			if (e->has_component<c_Animation>())
			{
				auto &animation= e->get_component<c_Animation>().animation;
				animation.get_sprite().setRotation(transform.angle);
				animation.get_sprite().setPosition(transform.position.x, transform.position.y);
				animation.get_sprite().setScale(transform.scale.x, transform.scale.y);
				m_game->window().draw(animation.get_sprite());
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangle shape
	if (m_draw_collision)
	{
		for (auto e : m_entity_manager.get_entities())
		{
			if (e->has_component<c_Bounding_box>())
			{
				auto &box= e->get_component<c_Bounding_box>();
				auto &transform= e->get_component<c_Transform>();
				sf::RectangleShape rectangle;
				rectangle.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
				rectangle.setOrigin(sf::Vector2f(box.half_size.x, box.half_size.y));
				rectangle.setPosition(transform.position.x, transform.position.y);
				rectangle.setFillColor(sf::Color(0, 0, 0, 0));
				rectangle.setOutlineColor(sf::Color(255, 255, 255, 255));
				rectangle.setOutlineThickness(1);
				m_game->window().draw(rectangle);
			}
		}
	}

	// draw the grid so that students can easily debug
	if (m_draw_grid)
	{
		float left_x= m_game->window().getView().getCenter().x - width() / 2;
		float right_x= left_x + width() + m_grid_size.x;
		float next_grid_x= left_x - ((int)left_x % (int)m_grid_size.x);

		for (float x= next_grid_x; x < right_x; x+= m_grid_size.x)
		{
			draw_line(c_Vec2(x, 0), c_Vec2(x, height()));
		}

		for (float y= 0; y < height(); y+= m_grid_size.y)
		{
			draw_line(c_Vec2(left_x, height() - y), c_Vec2(right_x, height() - y));

			for (float x= next_grid_x; x < right_x; x+= m_grid_size.x)
			{
				std::string x_cell= std::to_string((int)x / (int)m_grid_size.x);
				std::string y_cell= std::to_string((int)y / (int)m_grid_size.y);
				m_grid_text.setString("( " + x_cell + " , " + y_cell + " )");
				m_grid_text.setPosition(x + 3, height() - y - m_grid_size.y + 2);
				m_game->window().draw(m_grid_text);
			}
		}
	}
}