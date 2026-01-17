#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "UIElement.h"
#include "UIFont.h"
#include "UIResource.h"

namespace OpenNFS {
    /**
     * Loads UI layouts from JSON files with support for templating.
     *
     * Templating features:
     * - Text variables: {ONFS_VERSION}, {resX}, {resY}
     * - Position expressions: {resX - 75}, {resY + 10}, etc.
     * - OnClick callbacks: maps string names to registered functions
     */
    class UILayoutLoader {
      public:
        using CallbackRegistry = std::map<std::string, std::function<void()>>;

        UILayoutLoader(std::map<std::string, UIResource> const &resourceMap, std::map<std::string, UIFont> const &fontMap);

        /**
         * Load a UI layout from a JSON file
         * @param jsonPath Path to the JSON layout file
         * @param callbacks Optional callback registry for OnClick handlers
         * @return Vector of UI elements created from the layout
         */
        std::vector<std::unique_ptr<UIElement>> LoadLayout(std::string const &jsonPath, CallbackRegistry const &callbacks = {}) const;

        /**
         * Register a global variable for text templating
         * @param name Variable name (without braces)
         * @param value Variable value
         */
        void RegisterVariable(std::string const &name, std::string const &value);

      private:
        std::map<std::string, UIResource> const &m_resourceMap;
        std::map<std::string, UIFont> const &m_fontMap;
        std::map<std::string, std::string> m_variables;

        /**
         * Process text templates by replacing {VARIABLE_NAME} with actual values
         */
        std::string ProcessTextTemplate(std::string const &templateStr) const;

        /**
         * Evaluate position expressions like {resX - 75}, {100}, etc.
         */
        float EvaluateExpression(std::string const &expr) const;

        /**
         * Parse a hex colour string (#RRGGBB or #RRGGBBAA) to glm::vec4
         */
        static glm::vec4 ParseColour(std::string const &hexColour);
    };
} // namespace OpenNFS