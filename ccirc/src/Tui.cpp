//#include <memory>  // for allocator, __shared_ptr_access
#include <string>  // for char_traits, operator+, string, basic_string
 
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

#include "Tui.h"

void Tui::start(){
 using namespace ftxui;
 
  // The basic input components:
    Component input_c = Input(&input, "Write something...");

    auto writer = [&](std::string newData){
        chat.push_back(newData);
    };

    input_c |= CatchEvent([&](Event event){
        if(ftxui::Event::Character('\n') == event){
            writer("You: " + input);
            if(ircClient && !input.empty()){
                ircClient->uiCmd(input);
            }
            input = "";
            return true;
        }
        return false;
    });

    auto chatComponent = Renderer([&]{
        Elements elements;
        for (const auto line : chat){
            elements.push_back(text(line));
        }
        return vbox(std::move(elements));
    });

    auto channelsRenderer = Renderer([&]{
        Elements elements;
        for (const auto channel : channels){
            elements.push_back(text(channel));
        }
        return vbox(std::move(elements));
    });

    auto usersRenderer = Renderer([&]{
        Elements elements;
        for (const auto user : users_){
            elements.push_back(text(user));
        }
        return vbox(std::move(elements));
    });

    auto title = Renderer([&]{
        return hbox(text("IRC Client") | bold);
    });

    // The component tree:
    auto component = Container::Vertical({
        title,
        chatComponent,
        channelsRenderer,
        usersRenderer,
        input_c
    });

// Tweak how the component tree is rendered:
auto main_renderer = Renderer(component, [&]{
    return vbox({
        title->Render() | hcenter,
        hbox({
            vbox({
                text("Channels:"),
                channelsRenderer->Render()
            }) | border,
            vbox({
                vbox({
                    chatComponent->Render(),
                    filler() | focus // Hack to make it scrollable :)
                }) | yframe | yflex | border,
                hbox(text("You: "), input_c->Render()) | border
            }) | flex,
            vbox({
                text("Users:"),
                usersRenderer->Render()
            }) | border,
        }) | flex,
    });
});
 
  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(main_renderer);
}