//
// clang++ -std=c++11 -framework SDL2 -framework SDL2_ttf -I ../../src main.cpp
//
// clang++ -std=c++11 -framework SDL2 -framework SDL2_ttf -I ../../src -I ../../../../ext/catch/include -D TEST main.cpp
//

#include "precomp.h"

/**
* Application
*
* These are the functions that define the functionality for this application.
* This application will draw some text in a window that follows the mouse position
* and is animated in a circle around the mouse position.
*
*/


/**
* Use time to generate a position in the unit circle and scale it in size and time
*
* @param period The number of seconds it takes to complete a circle
* @param radius The radius of the circle to generate
* @return observable<SDL_Point>
*/
const auto make_points_circling_zero = [](float period, int radius, rx::observable<t::milliseconds> updates) {
    return updates.
        map([=](t::milliseconds ms){
            // map the tick into the range 0.0-1.0
            return float_map(ms.count() % int(period * 1000), 0, int(period * 1000), 0.0, 1.0);
        }).
        map([=](float t){
            // map the time value to a point on a circle of radius
            return SDL_Point{int(radius * std::cos(t * 2 * 3.14)), int(radius * std::sin(t * 2 * 3.14))};
        });
};

/**
* Combine the current mouse position with another point and return the
* mouse position offset by the other point.
*
* @param mouse_moves The source of mouse move events
* @param offset_points The source of the points that will offset the mouse position
* @return observable<SDL_Point>
*/
const auto make_points_offset_from_mouse = [](rx::observable<SDL_MouseMotionEvent*> mouse_moves, rx::observable<SDL_Point> offset_points) {
    return mouse_moves.
        map([](SDL_MouseMotionEvent* m){
            return SDL_Point{m->x, m->y};
        }).
        combine_latest(std::plus<SDL_Point>(), offset_points);
};

/**
* Render Component - draws a texture in orbit of the most recent
* mouse position within the window. Returns a source that will
* send a new value when the component has changed appearance.
*
* @param texture The texture to render
* @param period The time in seconds to complete a circle around the mouse position
* @param radius The size of the circle around the mouse position
* @param events The source of SDL events
* @param updates The source of update opportunities
* @param renders The source of rendering oppotunities
* @return observable<int>
*/
const auto texture_circling_mouse = [](SDL_Texture* texture, float period, int radius, rx::observable<SDL_Event*> events, rx::observable<t::milliseconds> updates, rx::observable<SDL_Renderer *> renders) {

    auto points_circling_zero = make_points_circling_zero(period, radius, updates);

    auto mouse_moves = make_event_filter<SDL_MouseMotionEvent>(events, SDL_MOUSEMOTION);

    auto points_circling_mouse = std::make_shared<rx::subjects::behavior<SDL_Point>>(SDL_Point{0,0});

    auto lifetime = make_points_offset_from_mouse(mouse_moves, points_circling_zero).
        subscribe(points_circling_mouse->get_subscriber());

    //Render the texture on the circle around the mouse position
    renders.
        subscribe(lifetime, [=](SDL_Renderer *renderer){
            SDL_Rect dst = {points_circling_mouse->get_value().x, points_circling_mouse->get_value().y, 0, 0};
            SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
            SDL_RenderCopy(renderer, texture, NULL, &dst);
        });

    lifetime.
        add([=](){
            SDL_DestroyTexture(texture);
        });

    // new value when the texture needs to be moved
    return points_circling_mouse->
        get_observable().
        distinct_until_changed().
        map([](SDL_Point){return 1;});
};

const SDL_Color white = { 255, 255, 255, 255 };

/**
* Application - draws a two text in orbit of the most recent
* mouse position within the window. Returns a source that will
* send a new value when the component has changed appearance.
*
* @param renderer The renderer to use to create the textures
* @param events The source of SDL events
* @param updates The source of update opportunities
* @param renders The source of rendering oppotunities
* @return observable<int>
*/
const auto application = [](SDL_Renderer *renderer, rx::observable<SDL_Event*> events, rx::observable<t::milliseconds> updates, rx::observable<SDL_Renderer *> renders) {

    auto arrow = draw_text(renderer, "/Library/Fonts/Arial.ttf", 36, "Time flies like an arrow", white);

    auto dreadpirate = draw_text(renderer, "/Library/Fonts/Arial.ttf", 26, "Get used to disappointment", white);

    return texture_circling_mouse(arrow, 1.0, 50, events, updates, renders).
        merge(texture_circling_mouse(dreadpirate, 2.0, 100, events, updates, renders));
};


#ifdef TEST

/**
* Testing in virtual time
*
* rxcpp is based on schedulers that are replaceable and provide a local source of time.
* this can be used to test code that is a function of time without any delays.
* A timer that fires every second can be fired 30 times in less than a second
* in a test and still be correct.
*
*/

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

namespace rxsc=rxcpp::schedulers;

#include "rxcpp/rx-test.hpp"

SCENARIO("points circling zero", "[zero][circle]"){
    GIVEN("a source of time"){
        auto sc = rxsc::make_test();
        auto w = sc.create_worker();
        const rxsc::test::messages<t::milliseconds> on;
        const rxsc::test::messages<SDL_Point> onp;

        auto xs = sc.make_hot_observable({
            // not delivered because there is no subscriber
            on.next(70, t::milliseconds(-200)),
            on.next(150, t::milliseconds(-100)),

            // delivered values
            on.next(210, t::milliseconds(0)),
            on.next(230, t::milliseconds(100)),
            on.next(270, t::milliseconds(200)),
            on.next(280, t::milliseconds(300)),
            on.next(300, t::milliseconds(400)),
            on.next(310, t::milliseconds(500)),
            on.next(340, t::milliseconds(600)),
            on.next(370, t::milliseconds(700)),
            on.next(410, t::milliseconds(800)),
            on.next(415, t::milliseconds(900)),
            on.next(460, t::milliseconds(1000)),
            on.next(510, t::milliseconds(1100)),
            on.next(560, t::milliseconds(1200)),
            on.next(570, t::milliseconds(1300)),
            on.next(580, t::milliseconds(1400)),
            on.next(590, t::milliseconds(1500)),
            on.next(630, t::milliseconds(1600)),
            on.completed(690)
        });

        WHEN("points are generated from the time"){

            auto res = w.start(
                [xs]() {
                    return make_points_circling_zero(1.0, 50, xs)
                        // forget type to workaround lambda deduction bug on msvc 2013
                        .as_dynamic();
                }
            );

            THEN("the output contains points around zero"){
                auto required = rxu::to_vector({
                    onp.next(210, SDL_Point{50, 0}),
                    onp.next(230, SDL_Point{40, 29}),
                    onp.next(270, SDL_Point{15, 47}),
                    onp.next(280, SDL_Point{-15, 47}),
                    onp.next(300, SDL_Point{-40, 29}),
                    onp.next(310, SDL_Point{-49, 0}),
                    onp.next(340, SDL_Point{-40, -29}),
                    onp.next(370, SDL_Point{-15, -47}),
                    onp.next(410, SDL_Point{15, -47}),
                    onp.next(415, SDL_Point{40, -29}),
                    onp.next(460, SDL_Point{50, 0}),
                    onp.next(510, SDL_Point{40, 29}),
                    onp.next(560, SDL_Point{15, 47}),
                    onp.next(570, SDL_Point{-15, 47}),
                    onp.next(580, SDL_Point{-40, 29}),
                    onp.next(590, SDL_Point{-49, 0}),
                    onp.next(630, SDL_Point{-40, -29}),
                    onp.completed(690)
                });
                auto actual = res.get_observer().messages();
                REQUIRE(required == actual);
            }

            THEN("there was 1 subscription/unsubscription to the source"){
                auto required = rxu::to_vector({
                    on.subscribe(200, 690)
                });
                auto actual = xs.subscriptions();
                REQUIRE(required == actual);
            }
        }
    }
}

#else

int main(int, char**){

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }

    if (TTF_Init() != 0){
        logSDLError(std::cout, "TTF_Init");
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Circling Text", 100, 100, 800,
        600, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        logSDLError(std::cout, "CreateWindow");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        logSDLError(std::cout, "CreateRenderer");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool dirty = false;

    application(renderer, events.get_observable(), updates.get_observable(), renders.get_observable()).
        subscribe([&](int){dirty=true;});

    bool done = false;

    make_event_filter<SDL_Event>(events.get_observable(), SDL_QUIT).
        subscribe([&](SDL_Event*){
            event.on_completed();
            update.on_completed();
            render.on_completed();
            done = true;
        });

    while (!done) {

        SDL_Event sdlevent;
        while (SDL_PollEvent(&sdlevent)) {
            event.on_next(&sdlevent);
        }

        update.on_next(t::milliseconds(SDL_GetTicks()));

        if (dirty) {
            SDL_RenderClear(renderer);
            render.on_next(renderer);
            SDL_RenderPresent(renderer);
            dirty = false;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    std::cout << "clean exit" << std::endl;
    return 0;
}

#endif
