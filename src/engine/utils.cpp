#include <glm/ext/matrix_clip_space.hpp>
#include <furredengine/utils.h>

using namespace FurredEngine;

std::string utils::toLowerCase(std::string str) {
    std::ranges::transform(
        str, str.begin(),
        [&](char ch) { return std::use_facet<std::ctype<char>>(::_locale).tolower(ch); }
    );
    return str;
}

std::string utils::toUpperCase(std::string str) {
    std::ranges::transform(
        str, str.begin(),
        [&](char ch) { return std::use_facet<std::ctype<char>>(::_locale).toupper(ch); }
    );
    return str;
}

std::vector<std::string> utils::splitString(std::string string, std::regex delimiter) {
    std::vector<std::string> tokens;
    std::sregex_token_iterator iter(string.begin(), string.end(), delimiter, -1);
    std::sregex_token_iterator end;
    while (end != iter) {
        tokens.push_back(*iter++);
    }
    return tokens;
}

bool utils::caseInsensitiveCompare(std::string first, std::string second) {
    return std::ranges::equal(first, second, [](char a, char b) { return tolower(a) == tolower(b); });
}

long double utils::dot(Point &p1, Point &p2) {
    return (p1.x * p2.x) + (p1.y * p2.y);
};
long double utils::dot(Point &p1, Size &p2) {
    return (p1.x * p2.width) + (p1.y * p2.height);
};
long double utils::dot(Size &p1, Point &p2) {
    return (p1.width * p2.x) + (p1.height * p2.y);
};
long double utils::dot(Size &p1, Size &p2) {
    return (p1.width * p2.width) + (p1.height * p2.height);
};

// Clockwise rotation
Point utils::rotatePointByCenter(Point& point, Point& center, long double& degrees) {
    auto radians = -degrees * radDivisor;

    auto cos = std::cos(radians);
    auto sin = std::sin(radians);

    auto translatedX = point.x - center.x;
    auto translatedY = point.y - center.y;

    return {
        (translatedX * cos - translatedY * sin) + center.x,
        (translatedX * sin + translatedY * cos) + center.y
    };
}

const char* utils::sha512(const char* input) {
    unsigned char hash[SHA512_DIGEST_LENGTH];

    SHA512(
        reinterpret_cast<const unsigned char*>(input),
        strlen(input),
        hash
    );

    auto out = reinterpret_cast<char*>(calloc(SHA512_DIGEST_LENGTH + 1, sizeof(char)));
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(out + (i * 2), SHA512_DIGEST_LENGTH + 1, "%02x", hash[i]);
    }

    return const_cast<const char*>(out);
}

std::string utils::sha512(std::string& input) {
    return sha512(input.c_str());
}

utils::GLStatus utils::checkShaderCompile(GLuint shader) {
    GLStatus status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status.success);
    glGetShaderInfoLog(shader, sizeof(status.message), NULL, status.message);

    return status;
}

utils::GLStatus utils::checkProgramLink(GLuint program) {
    GLStatus status;

    glGetProgramiv(program, GL_LINK_STATUS, &status.success);
    glGetProgramInfoLog(program, sizeof(status.message), NULL, status.message);

    return status;
}

utils::GLStatus utils::linkAndUseProgram(GLuint program) {
    glLinkProgram(program);

    auto status = checkProgramLink(program);
    if (!status.success) {
        LogError(fmt::format("Error linking GL program: {}", status.message));
        return status;
    }
    
    glUseProgram(program);

    return status;
}

glm::mat4 utils::createOrthoMat(long double framebufferWidth, long double framebufferHeight) {
    return glm::ortho(
        0.l, framebufferWidth,
        0.l, framebufferHeight,
        0.l, 1.l
    );
}

glm::mat4 utils::createOrthoMat(Size framebufferSize) {
    return createOrthoMat(framebufferSize.width, framebufferSize.height);
}

httplib::Result utils::getURL(std::string url) {
    auto _url = ada::parse(url);
    httplib::Client _client(_url->get_origin());
    return _client.Get(static_cast<std::string>(_url->get_pathname()));
}