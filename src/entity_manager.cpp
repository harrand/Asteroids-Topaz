#include "entity_manager.hpp"
#include "graphics/gui/gui.hpp"

EntityManager::EntityManager(Window& wnd): Scene(), key_listener(wnd), mouse_listener(wnd), sprite_collection(), quad(tz::util::gui::gui_quad()), screen_wrapping_bounds(std::nullopt){}

bool EntityManager::has_player() const
{
    bool contains = false;
    for(const auto& sprite_ptr : this->heap_sprites)
        if(dynamic_cast<Player*>(sprite_ptr.get()) != nullptr)
            contains = true;
    return contains;
}

Player& EntityManager::spawn_player(Vector2I position, float rotation, Vector2F scale)
{
    return this->emplace<Player>(position, rotation, scale, &this->sprite_collection.get_player());
}

void EntityManager::update(float delta)
{
    for(Player* player : this->get_players())
    {
        if(this->key_listener.is_key_pressed("A"))
            player->set_rotation(player->get_rotation() + delta * 10);
        if(this->key_listener.is_key_pressed("D"))
            player->set_rotation(player->get_rotation() - delta * 10);
        if(this->key_listener.is_key_pressed("Space"))
        {
            Vector2F forward = player->forward() * 250.0f;
            player->position_screenspace.x += forward.x * delta;
            player->position_screenspace.y += forward.y * delta;
        }
        if(this->key_listener.catch_key_pressed("F"))
            player->shoot();
        player->update(delta);
    }
    this->handle_screen_wrapping();
    Scene::update(delta);
}

bool EntityManager::screen_wrapping_enabled() const
{
    return this->screen_wrapping_bounds.has_value();
}

std::optional<Vector2I> EntityManager::get_screen_wapping_bounds() const
{
    return this->screen_wrapping_bounds;
}

void EntityManager::enable_screen_wrapping(Vector2I bounds)
{
    this->screen_wrapping_bounds = bounds;
}

void EntityManager::disable_screen_wrapping()
{
    this->screen_wrapping_bounds = std::nullopt;
}

void EntityManager::handle_screen_wrapping()
{
    if(!this->screen_wrapping_enabled())
        return;
    auto wrap_bounds = this->screen_wrapping_bounds.value();
    for(Sprite& sprite : this->get_mutable_sprites())
    {
        auto& position = sprite.position_screenspace;
        if(position.x < 0)
            position.x = wrap_bounds.x ;
        else if(position.x > wrap_bounds.x)
            position.x = 0;
        if(position.y < 0)
            position.y = wrap_bounds.y;
        else if(position.y > wrap_bounds.y)
            position.y = 0;
    }
}

std::vector<Player*> EntityManager::get_players()
{
    std::vector<Player*> players;
    for(auto& sprite_ptr : this->heap_sprites)
    {
        Player* player_component = dynamic_cast<Player*>(sprite_ptr.get());
        if(player_component != nullptr)
            players.push_back(player_component);
    }
    for(Sprite& sprite : this->stack_sprites)
    {
        Player* player_component = dynamic_cast<Player*>(&sprite);
        if(player_component != nullptr)
            players.push_back(player_component);
    }
    return players;
}