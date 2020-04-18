#include <iostream>
#include <vector>
#include <random>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <vec2.hpp>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const float MAX_VEL = 1.0;
const float MAX_FORCE = 0.005;
const float APPROACH_RADIUS = 100;
const float WANDER_CIRCLE_DISTANCE = 100;
const float WANDER_CIRCLE_RADIUS = 30;
const float ANGLE_CHANGE = 4;

std::random_device rd;
std::mt19937 rand_gen(rd());
std::uniform_int_distribution<int> rand_angle(0, 360);
std::uniform_real_distribution<float> rand_float(-10, 10);

struct entity {
    Vector2f pos = Vector2f(200.0);
    Vector2f vel = Vector2f(0.01);
    float mass = 1.0;

    float wander_angle;

    std::vector<Vector2f> trail;

    void draw() {
        al_draw_filled_rectangle(pos.x - 10, pos.y - 10, pos.x + 10, pos.y + 10, al_map_rgb(240, 0, 0));
        al_draw_line(pos.x, pos.y, pos.x + vel.normalized().x * 40, pos.y + vel.normalized().y * 40, al_map_rgb(0, 240, 0), 1.0);

        // draw trail
        for (const auto v : trail) {
            al_draw_pixel(v.x, v.y, al_map_rgb(0, 240, 0));
        }
    }

    Vector2f seek(Vector2f target) {
        auto desired_vel = (target - pos).normalized() * MAX_VEL;

        // slow during approach
        float distance = pos.distance(target);
        if (distance < APPROACH_RADIUS) {
            desired_vel *= (distance / APPROACH_RADIUS);
        }

        return (desired_vel - vel);
    }

    Vector2f flee(Vector2f target) {
        auto desired_vel = (pos - target).normalized() * MAX_VEL;
        return (desired_vel - vel);
    }

    Vector2f wander() {
        Vector2f circle_pos = pos + vel.normalized() * WANDER_CIRCLE_DISTANCE;
        Vector2f target = circle_pos + vel.normalized() * WANDER_CIRCLE_RADIUS;
        //std::cout << "Wander circle: " << circle_pos.x << " " << circle_pos.y << std::endl;
        //std::cout << "Wander target: " << target.x << " " << target.y << std::endl;

        Vector2f displacement = target - circle_pos;
        displacement.set_angle(wander_angle);
        wander_angle += rand_float(rand_gen);

        al_draw_circle(circle_pos.x, circle_pos.y, WANDER_CIRCLE_RADIUS, al_map_rgb(240, 240, 240), 1.0);
        al_draw_line(circle_pos.x, circle_pos.y, circle_pos.x + displacement.x, circle_pos.y + displacement.y, al_map_rgb(0, 240, 0), 1.0);

        return displacement;
    }

    void apply_steering(Vector2f force) {
        force = force.truncate(MAX_FORCE);
        force /= mass;
        vel += force;
        vel = vel.truncate(MAX_VEL);

        //std::cout << "Steering: " << force.x << ", " << force.y << std::endl;
        //std::cout << "Velocity: " << vel.x << ", " << vel.y << std::endl;
        //std::cout << "Position: " << pos.x << ", " << pos.y << std::endl;

        pos += vel;
    }

    void leave_trail() {
        trail.push_back(pos);

        if (trail.size() > 25) {
            trail.erase(trail.begin());
        }
    }
};

int main(int argc, char** argv) {
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();

    ALLEGRO_DISPLAY* display;
    ALLEGRO_TIMER* loop_timer;
    ALLEGRO_TIMER* trail_timer;
    ALLEGRO_EVENT_QUEUE* ev_queue;

    display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);

    const int FPS = 60;
    loop_timer = al_create_timer(1.0 / FPS);
    trail_timer = al_create_timer(1.0 / 2);

    ev_queue = al_create_event_queue();
    al_register_event_source(ev_queue, al_get_keyboard_event_source());
    al_register_event_source(ev_queue, al_get_mouse_event_source());
    al_register_event_source(ev_queue, al_get_display_event_source(display));
    al_register_event_source(ev_queue, al_get_timer_event_source(loop_timer));
    al_register_event_source(ev_queue, al_get_timer_event_source(trail_timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT ev;
    ALLEGRO_MOUSE_STATE mouse;

    // demo entity
    entity follower;
    follower.wander_angle = rand_angle(rand_gen);
    Vector2f center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    al_start_timer(loop_timer);
    al_start_timer(trail_timer);

    while (!done) {
        al_wait_for_event(ev_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == loop_timer) {
                redraw = true;
            }
            else if (ev.timer.source == trail_timer) {
                follower.leave_trail();
            }
        }

        if (redraw) {
            redraw = false;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            /*
            al_get_mouse_state(&mouse);
            auto seek_force = follower.seek(Vector2f(mouse.x, mouse.y));
            auto flee_force = Vector2f(0.0);

            if (follower.pos.distance(center) < 100) {
                flee_force = follower.flee(center);
            }

            follower.apply_steering(seek_force + flee_force);
            */
            follower.apply_steering(follower.wander());
            follower.draw();

            al_draw_filled_circle(center.x, center.y, 10, al_map_rgb(0, 0, 240));

            if (follower.pos.x < 0) {
                follower.pos.x += SCREEN_WIDTH;
            }
            if (follower.pos.y < 0) {
                follower.pos.y += SCREEN_HEIGHT;
            }

            if (follower.pos.x > SCREEN_WIDTH) {
                follower.pos.x = SCREEN_WIDTH - follower.pos.x;
            }
            if (follower.pos.y > SCREEN_HEIGHT) {
                follower.pos.y = SCREEN_HEIGHT - follower.pos.y;
            }

            al_flip_display();
        }
    }

    al_destroy_timer(loop_timer);
    al_destroy_event_queue(ev_queue);
    al_destroy_display(display);

    return 0;
}
