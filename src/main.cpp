#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstdint>

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include "env.hpp"
#include "renderer.hpp"
#include "script.hpp"
#include "acmp.hpp"

using namespace std;
using namespace sf;

uint64_t timeSinceEpochMillisec() {
    using namespace std::chrono;
    return duration_cast<chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
}

bool lkm = 0;

bool is_press(sf::Text txt, sf::Vector2f mouse ){
    if(!lkm && sf::Mouse::isButtonPressed(sf::Mouse::Left)  && txt.getGlobalBounds().contains(mouse)){
        lkm = 1;
        return 1;
    }
    return 0;
}

void choose(std::map < std::string, sf::Text > &mp, sf::Vector2f mouse){
    for(auto &p : mp) {
        sf::FloatRect bounds = p.second.getGlobalBounds();
        if (bounds.contains(mouse))
            p.second.setFillColor(Color::Red);
        else
            p.second.setFillColor(Color::Black);
    }
}


int main() {
    Environment* env   = new Environment(RenderType::Lobby);
    Renderer* renderer = new Renderer(env);
    Script* script     = new Script(env);
/*
    env->screen.background = "front";
    env->screen.characters = {"giorno"};
    env->screen.say        = "wry";
    env->screen.speaker    = "giorno";
    env->screen.type       = ScreenType::Monolog;
    */

    int64_t d_next            = 50;
    int64_t d_update          = 1000/30;
    int text_speed            = 100;
    int64_t string_will_shown = 0;

    int64_t start_time        = timeSinceEpochMillisec();
    int64_t textstart_time    = timeSinceEpochMillisec() + d_next;
    int64_t next_time         = timeSinceEpochMillisec();
    int64_t update_time       = timeSinceEpochMillisec();

    while(renderer->window.isOpen()) {
        sf::Event event;
        int64_t now_time = timeSinceEpochMillisec();

        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) lkm = 0;

        while(renderer->window.pollEvent(event)) {

            string_will_shown = min((int64_t)(now_time - textstart_time) / text_speed, (int64_t)env->strings[env->screen.say].size());
            if(next_time <= now_time) {
                if (event.type == sf::Event::Closed) {
                    renderer->window.close();
                }
                if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space || sf::Mouse::isButtonPressed(sf::Mouse::Left))) {
                    if (string_will_shown == (int) env->strings[env->screen.say].size()) {
                        // TODO: make LUA script
                        //if (env->screen.child == "-1") window.close();
                        script->next(1);
                        textstart_time = now_time + d_next;
                    } else {
                        textstart_time = -10000000;
                    }

                }
                if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) {
                    RenderType pre       = env->pre_render_type;
                    env->pre_render_type = env->    render_type;
                         if(env->render_type == RenderType::Pause)      env->render_type = RenderType::Game;
                    else if(env->render_type == RenderType::Game)       env->render_type = RenderType::Pause;
                    else if(env->render_type == RenderType::Settings)   env->render_type = pre;
                }
                next_time = now_time + d_next;
            }
        }

        if(now_time > update_time){

            string_will_shown = min((int64_t)(now_time - textstart_time) / text_speed, (int64_t)env->strings[env->screen.say].size());
            string_will_shown = max(0ll, string_will_shown);

            if(string_will_shown != env->string_shown){
                env->string_shown = string_will_shown;
            }

            sf::Vector2f mouse = renderer->window.mapPixelToCoords(sf::Mouse::getPosition(renderer->window));

            // hit test6
            if(env->render_type == RenderType::Pause) {
                choose(renderer->UI_pause, mouse);

                // transform the mouse position from window coordinates to world coordinates
                if (is_press(renderer->UI_pause["exit"], mouse)) {
                    renderer->window.close();
                }
                // TODO: Make saves for scripts
                /*
                for(int i = 1; i <= 3; i++) {
                    if (UI_esc["menu_load" + to_string(i)].getGlobalBounds().contains(mouse)) {
                        ifstream in("saves/save" + to_string(i) + ".txt");
                        string s;
                        in >> s;
                        env->screen = env->screens[s];
                        env->render_type = RenderType::Game;
                    }
                }
                for(int i = 1; i <= 3; i++) {
                    if (UI_esc["menu_save" + to_string(i)].getGlobalBounds().contains(mouse)) {
                        ofstream out("saves/save" + to_string(i) + ".txt");
                        out << env->screen.name;
                        env->render_type = RenderType::Game;
                    }
                }
                */

                if (is_press(renderer->UI_pause["resume"], mouse)) {
                    env->render_type = RenderType::Game;
                }
                if (is_press(renderer->UI_pause["settings"], mouse)) {
                    env->render_type = RenderType::Settings;
                }

            } else if(env->render_type == RenderType::Game) {
                choose(renderer->UI_game, mouse);

                if (is_press(renderer->UI_game["menu_button"], mouse)) {
                    env->render_type = RenderType::Pause;
                }
            } else if(env->render_type == RenderType::Lobby){
                choose(renderer->UI_lobby, mouse);

                if (is_press(renderer->UI_lobby["new_game"], mouse)) {
                    script->new_game();
                    textstart_time = now_time;
                    env->render_type = RenderType::Game;
                }
                if (is_press(renderer->UI_lobby["settings"], mouse)) {
                    env->render_type = RenderType::Settings;
                }
            } else if(env->render_type == RenderType::Settings) {
                choose(renderer->UI_settings, mouse);

                if (is_press(renderer->UI_settings["resume"], mouse)) {
                    env->render_type = RenderType::Pause;
                }

                if (is_press(renderer->UI_settings["fullscreen"], mouse)) {
                    renderer->make_fullscreen();
                }
                if (is_press(renderer->UI_settings["defaultscreen"], mouse)) {
                    renderer->make_defaultscreen();
                }
                if (is_press(renderer->UI_settings["fullscreen_bordered"], mouse)) {
                    renderer->make_fullscreen_bordered();
                }
            }else if(env->render_type == RenderType::Acmp) {
                choose(renderer->UI_acmp, mouse);

            }

            renderer->draw();
            update_time = now_time + d_update;
        }

    }
    return 0;
}