#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp> // for pausing and resuming the game
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/cocos.hpp> // for FLAlertLayer
#include <Geode/loader/Mod.hpp> // for getting config directory
#include <Geode/cocos/actions/CCActionManager.h> // include for action pausing
#include "json.hpp" // Include nlohmann::json for JSON parsing

#include <fstream> // for file reading and writing
#include <string> // for std::string
#include <random> // for random number generation

using json = nlohmann::json;
using namespace geode::prelude;

class $modify(MyPlayerObject, PlayerObject) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    // Function to read and validate configuration values from the JSON file
    json readConfig() {
        auto mod = Mod::get(); // Get the current mod instance
        auto configDir = mod->getConfigDir(true); // Get the mod's config directory

        std::ifstream configFile(configDir / "settings.json");
        if (!configFile.is_open()) {
            log::error("Failed to open settings.json file in config directory");
            showPopupMessage("Error: Missing config file! Please read the manual for instructions.");
            return {};
        }

        json configJson;
        try {
            configFile >> configJson; // Parse the JSON file
        } catch (const std::exception& e) {
            log::error("Error parsing JSON file: {}", e.what());
            showPopupMessage("Error: Invalid config file! Please read the manual for instructions.");
            return {};
        }

        // Validate duration and intensity ranges
        int minDuration = configJson.value("minDuration", 300);
        int maxDuration = configJson.value("maxDuration", 30000);
        int minIntensity = configJson.value("minIntensity", 1);
        int maxIntensity = configJson.value("maxIntensity", 100);

        if (minDuration < 300 || maxDuration > 30000 || minDuration > maxDuration) {
            log::error("Invalid duration range in config: minDuration={}, maxDuration={}", minDuration, maxDuration);
            showPopupMessage("Error: Invalid config file! Please read the manual for instructions.");
            return {};
        }

        if (minIntensity < 1 || maxIntensity > 100 || minIntensity > maxIntensity) {
            log::error("Invalid intensity range in config: minIntensity={}, maxIntensity={}", minIntensity, maxIntensity);
            showPopupMessage("Error: Invalid config file! Please read the manual for instructions.");
            return {};
        }

        return configJson;
    }

    // Function to generate a random value within a range
    int generateRandomValue(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }

    // Hook into the player's death effect
    void playDeathEffect() {
        // Call the original death effect function to keep the default behavior
        PlayerObject::playDeathEffect();

        // Immediately pause the game and show "Shocking..."
        pauseGame();
        //showPopupMessage("Shocking...");

        // Execute the custom web request after showing the message
        sendShockPOSTRequest();
    }

    // Function to send a POST request with JSON data
    void sendShockPOSTRequest() {
        // Read the configuration from the JSON file
        json config = readConfig();
        if (config.empty()) {
            return; // Exit if the configuration couldn't be read or is invalid
        }

        // Extract the min and max values for random generation
        int minDuration = config.value("minDuration", 300);
        int maxDuration = config.value("maxDuration", 30000);
        int minIntensity = config.value("minIntensity", 1);
        int maxIntensity = config.value("maxIntensity", 100);
        auto shockerID = config.value("shockerID", "");
        auto openShockToken = config.value("OpenShockToken", "");
        auto customName = config.value("customName", "");

        // Get the endpoint domain, default to api.openshock.app if missing or empty
        std::string endpointDomain = config.value("endpointDomain", "api.openshock.app");
        if (endpointDomain.empty()) {
            endpointDomain = "api.openshock.app";
        }

        if (shockerID.empty() || openShockToken.empty() || customName.empty()) {
            log::error("Missing required fields in JSON configuration");
            showPopupMessage("Error: Missing required fields in config file! Please read the manual for instructions.");
            return;
        }

        // Generate random intensity and duration within the valid ranges
        int randomIntensity = generateRandomValue(minIntensity, maxIntensity);
        int randomDurationMs = generateRandomValue(minDuration, maxDuration);

        // Bind the listener to handle the response
        m_fields->m_listener.bind([this](web::WebTask::Event* e) {
            if (web::WebResponse* res = e->getValue()) {
                // Get the server response as a string
                std::string response = res->string().unwrapOr("No response from the server");

                // Show the response in pop-up message DEBUG
                //showPopupMessage(response);

            // } else if (web::WebProgress* p = e->getProgress()) {
            //     // Log the progress of the request if it's still in progress
            //     log::info("Request in progress... Download progress: {}%", p->downloadProgress().value_or(0.f) * 100);
            } else if (e->isCancelled()) {
                // Show a cancellation message in the pop-up
                showPopupMessage("Request was cancelled.");
            }
        });

        // Create the web request object
        auto req = web::WebRequest();

        // Set the request body as JSON (as a string), using the generated values
        json requestBody = {
            { "shocks", {{
                { "id", shockerID },
                { "type", "Shock" },
                { "intensity", randomIntensity },
                { "duration", randomDurationMs },
                { "exclusive", true }
            }}},
            { "customName", customName }
        };

        // Add the JSON body to the request
        req.bodyString(requestBody.dump());

        // Set the headers
        req.header("Content-Type", "application/json");
        req.header("accept", "application/json");
        req.header("User-Agent", "OpenShock-GD/1.0 (jayden@jaydenha.uk)");

        // Add the OpenShockToken header
        req.header("OpenShockToken", openShockToken);

        // Construct the full URL with the endpoint domain
        std::string url = "https://" + endpointDomain + "/2/shockers/control";
        m_fields->m_listener.setFilter(req.post(url));

        // Show the duration and intensity in a pop-up message
        showPopupMessage("Duration: " + std::to_string(randomDurationMs / 1000) + "s" + "     " + "Intensity: " + std::to_string(randomIntensity));
    }

    // Function to send a POST request with JSON data
    void stopShockPOSTRequest() {
        // Read the configuration from the JSON file
        json config = readConfig();
        if (config.empty()) {
            return; // Exit if the configuration couldn't be read or is invalid
        }

        // Extract the min and max values for random generation
        auto shockerID = config.value("shockerID", "");
        auto openShockToken = config.value("OpenShockToken", "");
        auto customName = config.value("customName", "");

        // Get the endpoint domain, default to api.openshock.app if missing or empty
        std::string endpointDomain = config.value("endpointDomain", "api.openshock.app");
        if (endpointDomain.empty()) {
            endpointDomain = "api.openshock.app";
        }

        if (shockerID.empty() || openShockToken.empty() || customName.empty()) {
            log::error("Missing required fields in JSON configuration");
            showPopupMessage("Error: Missing required fields in config file! Please read the manual for instructions.");
            return;
        }

        // Bind the listener to handle the response
        m_fields->m_listener.bind([this](web::WebTask::Event* e) {
            if (web::WebResponse* res = e->getValue()) {
                // Get the server response as a string
                std::string response = res->string().unwrapOr("No response from the server");

                // Show the response in pop-up message DEBUG
                //showPopupMessage(response);

                // } else if (web::WebProgress* p = e->getProgress()) {
                //     // Log the progress of the request if it's still in progress
                //     log::info("Request in progress... Download progress: {}%", p->downloadProgress().value_or(0.f) * 100);
            } else if (e->isCancelled()) {
                // Show a cancellation message in the pop-up
                showPopupMessage("Request was cancelled.");
            }
        });

        // Create the web request object
        auto req = web::WebRequest();

        // Set the request body as JSON (as a string), using the generated values
        json requestBody = {
            { "shocks", {{
                { "id", shockerID },
                { "type", "Stop" },
                { "intensity", "1" },
                { "duration", "1" },
                { "exclusive", true }
            }}},
            { "customName", customName }
        };

        // Add the JSON body to the request
        req.bodyString(requestBody.dump());

        // Set the headers
        req.header("Content-Type", "application/json");
        req.header("accept", "application/json");
        req.header("User-Agent", "OpenShock-GD/1.0 (jayden@jaydenha.uk)");

        // Add the OpenShockToken header
        req.header("OpenShockToken", openShockToken);

        // Construct the full URL with the endpoint domain
        std::string url = "https://" + endpointDomain + "/2/shockers/control";
        m_fields->m_listener.setFilter(req.post(url));
    }

    // Function to pause the game
    void pauseGame() {
        if (auto playLayer = PlayLayer::get()) {
            // Pause the PlayLayer
            playLayer->pauseGame(true); 

            // Pause all running actions in the game
            if (auto actionManager = cocos2d::CCDirector::sharedDirector()->getActionManager()) {
                actionManager->pauseAllRunningActions();
            }
        }
    }

    // Function to show a pop-up message
    void showPopupMessage(const std::string& message) {
        geode::createQuickPopup(
            "Message",        // Title
            message.c_str(),  // Content (ensure it's a C-style string for createQuickPopup)
        "Continue",          // Text for button 1
        nullptr,             // No second button
        [](auto, bool btn2) {
            // Nothing here
        }
        );
    }

    // Function to show a pop-up message
    void showStopPopup(const std::string& message) {
        geode::createQuickPopup(
            "EMERGENCY STOP",           // Title
            message.c_str(),     // Content (ensure it's a C-style string for createQuickPopup)
        "STOP",          // Text for button 1
        nullptr,             // No second button
        [](auto, bool btn2) {
            // WIP
        }
        );
    }

};
