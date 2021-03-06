#include "shadermanager.h"

#include <iostream>
#include <type_traits>

#include <spdlog/spdlog.h>

namespace {

std::unique_ptr<GL::ShaderProgram>
loadProgram(ShaderManager::Program id)
{
    enum class VertexType { Solid,
                            Colored };
    struct ProgramSource {
        const char *vertexShader;
        const char *fragmentShader;
        VertexType vertexType;
    };
    static const ProgramSource programSources[] = {
        { "assets/shaders/debug.vert", "assets/shaders/debug.frag" }, // Debug
        { "assets/shaders/phong.vert", "assets/shaders/phong.frag" }, // Phong
        { "assets/shaders/shadow.vert", "assets/shaders/shadow.frag" }, // Shadow
    };
    static_assert(std::extent_v<decltype(programSources)> == ShaderManager::NumPrograms, "expected number of programs to match");
    const auto &sources = programSources[id];
    std::unique_ptr<GL::ShaderProgram> program(new GL::ShaderProgram);
    if (!program->addShader(GL_VERTEX_SHADER, sources.vertexShader)) {
        spdlog::warn("Failed to add vertex shader for program {}: {}", id, program->log());
        return {};
    }
    if (!program->addShader(GL_FRAGMENT_SHADER, sources.fragmentShader)) {
        spdlog::warn("Failed to add fragment shader for program {}: {}", id, program->log());
        return {};
    }
    if (!program->link()) {
        spdlog::warn("Failed to link program {}: {}", id, program->log());
        return {};
    }
    return program;
}

} // namespace

ShaderManager::~ShaderManager() = default;

void ShaderManager::useProgram(Program id)
{
    auto &cachedProgram = m_cachedPrograms[id];
    if (!cachedProgram) {
        cachedProgram.reset(new CachedProgram);
        cachedProgram->program = loadProgram(id);
        auto &uniforms = cachedProgram->uniformLocations;
        std::fill(uniforms.begin(), uniforms.end(), -1);
    }
    if (cachedProgram.get() == m_currentProgram) {
        return;
    }
    if (cachedProgram->program) {
        cachedProgram->program->bind();
    }
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(Uniform id)
{
    if (!m_currentProgram || !m_currentProgram->program) {
        return -1;
    }
    auto location = m_currentProgram->uniformLocations[id];
    if (location == -1) {
        static constexpr const char *uniformNames[] = {
            // clang-format off
            "mvp",
            "projectionMatrix",
            "viewMatrix",
            "modelMatrix",
            "normalMatrix",
            "lightViewProjection",
            "eyePosition",
            "lightPosition",
            "shadowMapTexture",
            // clang-format on
        };
        static_assert(std::extent_v<decltype(uniformNames)> == NumUniforms, "expected number of uniforms to match");

        location = m_currentProgram->program->uniformLocation(uniformNames[id]);
        m_currentProgram->uniformLocations[id] = location;
    }
    return location;
}
