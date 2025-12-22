#include "UILayoutLoader.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#include <json/json.hpp>

#include "../Config.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UITextField.h"

using json = nlohmann::json;

namespace OpenNFS {
    UILayoutLoader::UILayoutLoader(std::map<std::string, UIResource> const &resourceMap, std::map<std::string, UIFont> const &fontMap)
        : m_resourceMap(resourceMap), m_fontMap(fontMap) {
        // Register built-in variables
        RegisterVariable("ONFS_VERSION", ONFS_VERSION);
        RegisterVariable("resX", std::to_string(Config::get().resX));
        RegisterVariable("resY", std::to_string(Config::get().resY));
    }

    void UILayoutLoader::RegisterVariable(std::string const &name, std::string const &value) {
        m_variables[name] = value;
    }

    std::string UILayoutLoader::ProcessTextTemplate(std::string const &templateStr) const {
        std::string result = templateStr;
        std::regex varPattern(R"(\{([a-zA-Z_][a-zA-Z0-9_]*)\})");
        std::smatch match;

        size_t searchPos = 0;
        while (searchPos < result.length()) {
            std::string searchStr = result.substr(searchPos);
            if (std::regex_search(searchStr, match, varPattern)) {
                std::string const varName = match[1].str();
                auto const it = m_variables.find(varName);

                if (it != m_variables.end()) {
                    size_t const matchPos = searchPos + match.position(0);
                    result.replace(matchPos, match.length(0), it->second);
                    searchPos = matchPos + it->second.length();
                } else {
                    // Skip this match and continue searching
                    searchPos = searchPos + match.position(0) + match.length(0);
                }
            } else {
                break;
            }
        }

        return result;
    }

    float UILayoutLoader::EvaluateExpression(std::string const &expr) const {
        // First, process any variable templates
        std::string processed = ProcessTextTemplate(expr);

        // Remove whitespace
        processed.erase(std::remove_if(processed.begin(), processed.end(), ::isspace), processed.end());

        // Simple expression evaluator supporting +, -, *, /
        std::regex exprPattern(R"((-?\d+\.?\d*)\s*([+\-*/])\s*(-?\d+\.?\d*))");
        std::smatch match;

        while (std::regex_search(processed, match, exprPattern)) {
            float const left = std::stof(match[1].str());
            char const op = match[2].str()[0];
            float const right = std::stof(match[3].str());

            float result = 0.0f;
            switch (op) {
            case '+':
                result = left + right;
                break;
            case '-':
                result = left - right;
                break;
            case '*':
                result = left * right;
                break;
            case '/':
                result = right != 0.0f ? left / right : 0.0f;
                break;
            }

            processed = std::regex_replace(processed, exprPattern, std::to_string(result), std::regex_constants::format_first_only);
        }

        // If no operators found, just parse as a number
        try {
            return std::stof(processed);
        } catch (...) {
            LOG(WARNING) << "Failed to evaluate expression: " << expr;
            return 0.0f;
        }
    }

    glm::vec4 UILayoutLoader::ParseColor(std::string const &hexColor) {
        std::string hex = hexColor;
        if (hex[0] == '#') {
            hex = hex.substr(1);
        }

        // Default to opaque white if parsing fails
        if (hex.length() != 6 && hex.length() != 8) {
            LOG(WARNING) << "Invalid color format: " << hexColor;
            return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        auto const r = static_cast<float>(std::stoi(hex.substr(0, 2), nullptr, 16)) / 255.0f;
        auto const g = static_cast<float>(std::stoi(hex.substr(2, 2), nullptr, 16)) / 255.0f;
        auto const b = static_cast<float>(std::stoi(hex.substr(4, 2), nullptr, 16)) / 255.0f;
        auto const a = hex.length() == 8 ? static_cast<float>(std::stoi(hex.substr(6, 2), nullptr, 16)) / 255.0f : 1.0f;

        return glm::vec4(r, g, b, a);
    }

    std::vector<std::unique_ptr<UIElement>> UILayoutLoader::LoadLayout(std::string const &jsonPath,
                                                                       CallbackRegistry const &callbacks) const {
        std::vector<std::unique_ptr<UIElement>> elements;

        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            LOG(WARNING) << "Failed to open UI layout file: " << jsonPath;
            return elements;
        }

        try {
            json const layoutJson = json::parse(file);

            if (!layoutJson.contains("elements") || !layoutJson["elements"].is_array()) {
                LOG(WARNING) << "Invalid layout JSON: missing 'elements' array";
                return elements;
            }

            for (auto const &elemJson : layoutJson["elements"]) {
                try {
                    std::string const type = elemJson.value("type", "");
                    auto const scale = elemJson.value("scale", 1.0f);
                    auto const layer = elemJson.value("layer", 0u);

                    // Parse position
                    glm::vec2 position(0.0f, 0.0f);
                    if (elemJson.contains("position")) {
                        auto const &posJson = elemJson["position"];
                        if (posJson.contains("x")) {
                            std::string const xStr =
                                posJson["x"].is_string() ? posJson["x"].get<std::string>() : std::to_string(posJson["x"].get<float>());
                            position.x = EvaluateExpression(xStr);
                        }
                        if (posJson.contains("y")) {
                            std::string const yStr =
                                posJson["y"].is_string() ? posJson["y"].get<std::string>() : std::to_string(posJson["y"].get<float>());
                            position.y = EvaluateExpression(yStr);
                        }
                    }

                    glm::vec2 textOffset(0.0f, 0.0f);
                    if (elemJson.contains("textOffset")) {
                        auto const &ofsJson = elemJson["textOffset"];
                        if (ofsJson.contains("x")) {
                            std::string const xStr =
                                ofsJson["x"].is_string() ? ofsJson["x"].get<std::string>() : std::to_string(ofsJson["x"].get<float>());
                            textOffset.x = EvaluateExpression(xStr);
                        }
                        if (ofsJson.contains("y")) {
                            std::string const yStr =
                                ofsJson["y"].is_string() ? ofsJson["y"].get<std::string>() : std::to_string(ofsJson["y"].get<float>());
                            textOffset.y = EvaluateExpression(yStr);
                        }
                    }

                    // Create element based on type
                    std::unique_ptr<UIElement> element;

                    if (type == "image") {
                        std::string const resourceName = elemJson.value("resource", "");
                        auto const it = m_resourceMap.find(resourceName);
                        if (it != m_resourceMap.end()) {
                            element = std::make_unique<UIImage>(it->second, scale, layer, position);
                        } else {
                            LOG(WARNING) << "Resource not found: " << resourceName;
                            continue;
                        }
                    } else if (type == "textfield") {
                        std::string const textTemplate = elemJson.value("text", "");
                        std::string const text = ProcessTextTemplate(textTemplate);
                        std::string const colorStr = elemJson.value("color", "#FFFFFF");
                        glm::vec4 const color = ParseColor(colorStr);
                        std::string const font = elemJson.value("font", "default");

                        element = std::make_unique<UITextField>(text, color, scale, layer, position, font);
                    } else if (type == "button") {
                        std::string const resourceName = elemJson.value("resource", "missingResource");
                        auto const it = m_resourceMap.find(resourceName);
                        std::string const textTemplate = elemJson.value("text", "<No Text>");
                        std::string const text = ProcessTextTemplate(textTemplate);
                        std::string const colorStr = elemJson.value("color", "#FFFFFF");
                        glm::vec4 const color = ParseColor(colorStr);
                        std::string const hoverColorStr = elemJson.value("hoverColor", "#FFFFFF");
                        glm::vec4 const hoverColor = ParseColor(hoverColorStr);
                        std::string const font = elemJson.value("font", "default");
                        element = std::make_unique<UIButton>(it->second, text, color, hoverColor, scale, layer, position, textOffset, font);
                    } else {
                        LOG(WARNING) << "Unknown UI element type: " << type;
                        continue;
                    }

                    // Set up OnClick/OnHover callbacks if specified
                    if (elemJson.contains("onClick")) {
                        CHECK_F(type != "textfield", "Textfields can't have callbacks, use a button");
                        std::string const callbackName = elemJson["onClick"];
                        if (auto const it = callbacks.find(callbackName); it != callbacks.end()) {
                            element->SetOnClick(it->second);
                        } else {
                            LOG(WARNING) << "Callback not found: " << callbackName;
                        }
                    }

                    if (elemJson.contains("onHover")) {
                        CHECK_F(type != "textfield", "Textfields can't have callbacks, use a button");
                        std::string const callbackName = elemJson["onHover"];
                        if (auto const it = callbacks.find(callbackName); it != callbacks.end()) {
                            element->SetOnHover(it->second);
                        } else {
                            LOG(WARNING) << "Callback not found: " << callbackName;
                        }
                    }

                    elements.push_back(std::move(element));
                } catch (std::exception const &e) {
                    LOG(WARNING) << "Error parsing UI element: " << e.what();
                }
            }

            LOG(INFO) << "Loaded " << elements.size() << " UI elements from " << jsonPath;
        } catch (json::exception const &e) {
            LOG(WARNING) << "JSON parsing error in " << jsonPath << ": " << e.what();
        }

        // Sort the elements by layer number, so that alpha renders correctly
        std::ranges::sort(elements, [](auto const &el1, auto const &el2) { return el1->layer > el2->layer; });

        return elements;
    }
} // namespace OpenNFS