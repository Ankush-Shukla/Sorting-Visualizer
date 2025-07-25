#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include "SortingAlgorithms.hpp"
#include "Sorting.hpp"
#include "Visualization.hpp"
#include "AudioManager.hpp"

enum class SortType {
    Bubble = 1,
    Selection = 2,
    Insertion = 3,
    Merge = 4
};

int main() {
    int n = 50;
    int min_n = 5, max_n = 200;
    int speed = 50;
    int min_speed = 1, max_speed = 500;

    std::vector<int> arr = rand_array(n);

    float win_width = 800;
    float win_height = 600;
    sf::RenderWindow window(sf::VideoMode({ (unsigned int)win_width, (unsigned int)win_height }), "Sorting Visualizer");

    std::vector<sf::RectangleShape> rectangles = draw_rectangles(arr.data(), n, win_width, win_height);

    // Load font
    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        std::cerr << "Error: Could not load font file 'arial.ttf'. Please ensure the file exists.\n";
        return -1;
    }

    // Sound
    sf::SoundBuffer beepBuffer;
    if (!loadBeepSound(beepBuffer, "beep.wav")) {
        std::cerr << "Failed to load beep.wav. Exiting.\n";
        return -1;
    }
    sf::Sound sortingSound(beepBuffer);

    bool sortingStarted = false;
    bool barsVisible = false;
    SortType selectedSort = SortType::Bubble;

    // --- Start Button ---
// Buttons: Start, Randomize
    sf::RectangleShape startButton({ 120, 40 });
    startButton.setPosition({ win_width / 2 - 120 , 20 });
    startButton.setFillColor(sf::Color::White);
    startButton.setOutlineThickness(2);
    startButton.setOutlineColor(sf::Color(100, 200, 255));

    sf::Text startText(font);
    startText.setString("Start");
    startText.setCharacterSize(22);
    startText.setFillColor(sf::Color::Black);
    startText.setPosition({ startButton.getPosition().x + 30, startButton.getPosition().y + 8 });

    sf::RectangleShape randomButton({ 120, 40 });
    randomButton.setPosition({ win_width / 2 + 10 , 20 });
    randomButton.setFillColor(sf::Color(200, 255, 200));
    randomButton.setOutlineThickness(2);
    randomButton.setOutlineColor(sf::Color::Green);

    sf::Text randomText(font);
    randomText.setString("Randomize");
    randomText.setCharacterSize(18);
    randomText.setFillColor(sf::Color::Black);

    randomText.setPosition({ randomButton.getPosition().x + 10, randomButton.getPosition().y + 10 });


    // --- Sorting algorithm buttons (bottom row) ---
    std::vector<std::pair<sf::RectangleShape, sf::Text>> sortButtons;
    std::vector<std::string> sortNames = { "Bubble", "Selection", "Insertion", "Merge" };
    float buttonWidth = 150.f;
    float buttonHeight = 40.f;
    float spacing = 20.f;
    float totalWidth = 4 * buttonWidth + 3 * spacing;
    float xStart = (win_width - totalWidth) / 2;
    float yBottom = win_height - 70;

    for (int i = 0; i < 4; ++i) {
        sf::RectangleShape btn({ buttonWidth, buttonHeight });
        btn.setFillColor(sf::Color(220, 220, 220));
        btn.setPosition({ xStart + i * (buttonWidth + spacing), yBottom });

        sf::Text txt(font);
        txt.setString(sortNames[i] + " Sort");
        txt.setCharacterSize(20);
        txt.setFillColor(sf::Color::Black);
        txt.setPosition({ btn.getPosition().x + 15, btn.getPosition().y + 8 });

        sortButtons.emplace_back(btn, txt);
    }

    // --- Sliders (center of window) ---
    float sliderWidth = 300.f;
    float sliderHeight = 8.f;
    float sliderKnobRadius = 12.f;
    float sliderCenterY = win_height / 2.f;
    float sliderCenterX = win_width / 2.f;

    const float sliderX = sliderCenterX - sliderWidth / 2.f; // LEFT EDGE (use everywhere)
    const float sizeY = sliderCenterY - 40.f;
    const float speedY = sliderCenterY + 40.f;
    // Size
    sf::RectangleShape sizeSliderBg({ sliderWidth, sliderHeight });
    sizeSliderBg.setFillColor(sf::Color(80, 80, 120));
    sizeSliderBg.setPosition({ sliderX, sizeY });

    sf::Text sizeSliderLabel(font);
    sizeSliderLabel.setCharacterSize(18);
    sizeSliderLabel.setFillColor(sf::Color::White);
    sizeSliderLabel.setPosition({ sliderX, sizeY - 30 });

    sf::CircleShape sizeKnob(sliderKnobRadius);
    sizeKnob.setFillColor(sf::Color(100, 200, 255));
    sizeKnob.setOrigin({ sliderKnobRadius, sliderKnobRadius });

    // Speed
    sf::RectangleShape speedSliderBg({ sliderWidth, sliderHeight });
    speedSliderBg.setFillColor(sf::Color(80, 80, 120));
    speedSliderBg.setPosition({ sliderX, speedY });

    sf::Text speedSliderLabel(font);
    speedSliderLabel.setCharacterSize(18);
    speedSliderLabel.setFillColor(sf::Color::White);
    speedSliderLabel.setPosition({ sliderX, speedY - 30 });

    sf::CircleShape speedKnob(sliderKnobRadius);
    speedKnob.setFillColor(sf::Color(180, 120, 255));
    speedKnob.setOrigin({ sliderKnobRadius, sliderKnobRadius });

    bool draggingSize = false, draggingSpeed = false;

    auto sliderValue = [](float pos, float min, float max, float width) {
        float t = std::clamp(pos / width, 0.f, 1.f);
        return static_cast<int>(min + t * (max - min));
        };

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()){ window.close(); return 0; };

            if (const auto* m = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (m->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                    // Handle button clicks
                    if (startButton.getGlobalBounds().contains(mousePos)) {
                        sortingStarted = true;
                    }

                    if (randomButton.getGlobalBounds().contains(mousePos)) {
                        // Random array size
                        n = min_n + rand() % (max_n - min_n + 1);

                        // Random speed
                        speed = min_speed + rand() % (max_speed - min_speed + 1);

                        // Random sorting algorithm
                        selectedSort = static_cast<SortType>(1 + rand() % 4);  // values from 1 to 4

                        // Re-generate array and update visuals
                        arr = rand_array(n);
                        rectangles = draw_rectangles(arr.data(), n, win_width, win_height);
                        barsVisible = false;
                    }

                    // Sorting algorithm buttons
                    for (int i = 0; i < 4; ++i) {
                        if (sortButtons[i].first.getGlobalBounds().contains(mousePos)) {
                            selectedSort = static_cast<SortType>(i + 1);
                            break;
                        }
                    }

                    // knob hit-tests
                    if (std::hypot(mousePos.x - sizeKnob.getPosition().x,
                        mousePos.y - sizeKnob.getPosition().y) < sliderKnobRadius + 2)
                        draggingSize = true;

                    if (std::hypot(mousePos.x - speedKnob.getPosition().x,
                        mousePos.y - speedKnob.getPosition().y) < sliderKnobRadius + 2)
                        draggingSpeed = true;
                }
            }

            if (event->is<sf::Event::MouseButtonReleased>()) {
                draggingSize = draggingSpeed = false;
            }

            if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                sf::Vector2f m(mm->position.x, mm->position.y);

                if (draggingSize) {
                    float relX = std::clamp(m.x - sliderX, 0.f, sliderWidth);
                    int new_n = sliderValue(relX, min_n, max_n, sliderWidth);
                    if (new_n != n) {
                        n = new_n;
                        arr = rand_array(n);
                        rectangles = draw_rectangles(arr.data(), n, win_width, win_height);
                        barsVisible = false;
                    }
                }

                if (draggingSpeed) {
                    float relX = std::clamp(m.x - sliderX, 0.f, sliderWidth);
                    int new_speed = sliderValue(relX, min_speed, max_speed, sliderWidth);
                    if (new_speed != speed) speed = new_speed;
                }
            }
        }

        // Update slider knob positions
        float t_size = float(n - min_n) / float(max_n - min_n);
        float sizeX = sliderX + t_size * sliderWidth;
        sizeKnob.setPosition({ sizeX, sizeY + sliderHeight / 2.f });

        float t_speed = float(speed - min_speed) / float(max_speed - min_speed);
        float speedX = sliderX + t_speed * sliderWidth;
        speedKnob.setPosition({ speedX, speedY + sliderHeight / 2.f });

        // --- DRAWING ---
        window.clear(sf::Color(0, 0, 0));

        window.draw(startButton);
        window.draw(startText);

        window.draw(randomButton);
        window.draw(randomText);

        sizeSliderLabel.setString("Array Size: " + std::to_string(n));
        window.draw(sizeSliderBg);
        window.draw(sizeSliderLabel);
        window.draw(sizeKnob);

        speedSliderLabel.setString("Speed (low = fast): " + std::to_string(speed));
        window.draw(speedSliderBg);
        window.draw(speedSliderLabel);
        window.draw(speedKnob);

        for (int i = 0; i < 4; ++i) {
            auto& btn = sortButtons[i].first;
            auto& txt = sortButtons[i].second;
            btn.setFillColor(selectedSort == static_cast<SortType>(i + 1)
                ? sf::Color(100, 200, 255, 220)
                : sf::Color(220, 220, 220));
            window.draw(btn);
            window.draw(txt);
        }

        if (barsVisible) {
            for (const auto& rect : rectangles) {
                window.draw(rect);
            }
        }

        window.display();

        if (sortingStarted) {
            barsVisible = true;
            switch (selectedSort) {
            case SortType::Bubble:
                bubble_sort(arr.data(), rectangles, window, win_height, sortingSound, speed); break;
            case SortType::Selection:
                selection_sort(arr.data(), rectangles, window, win_height, sortingSound, speed); break;
            case SortType::Insertion:
                insertion_sort(arr.data(), rectangles, window, win_height, sortingSound, speed); break;
            case SortType::Merge:
                merge_sort_visualization(arr.data(), rectangles, window, win_height, 0, n - 1, sortingSound, speed); break;
            default:
                bubble_sort(arr.data(), rectangles, window, win_height, sortingSound, speed); break;
            }
            barsVisible = false;
            sortingStarted = false;
        }
    }

    return 0;
}
