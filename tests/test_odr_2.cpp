#include "../src/providers/gemini_3.hpp"
#include "../src/providers/gemini_2_5.hpp"
#include "../src/providers/openai_5.hpp"
#include "../src/providers/openai_4o.hpp"
#include "../src/providers/openai_4.hpp"
#include "../src/providers/anthropic_4_5_opus.hpp"
#include "../src/providers/anthropic_4_5_sonnet.hpp"
#include <iostream>

extern void odr_test_1();

void odr_test_2() {
    using namespace jai::llm;
    using namespace jai::llm::providers;
    std::cout << "Unit 2 Gemini 3: " << to_string_view(gemini_3::Role::MODEL) << "\n";
    std::cout << "Unit 2 Gemini 2.5: " << to_string_view(gemini_2_5::Role::MODEL) << "\n";
    std::cout << "Unit 2 OpenAI 5: " << to_string_view(openai_5::Role::ASSISTANT) << "\n";
    std::cout << "Unit 2 OpenAI 4o: " << to_string_view(openai_4o::Role::ASSISTANT) << "\n";
    std::cout << "Unit 2 OpenAI 4: " << to_string_view(openai_4::Role::ASSISTANT) << "\n";
    std::cout << "Unit 2 Anthropic Opus: " << to_string_view(anthropic_4_5_opus::Role::ASSISTANT) << "\n";
    std::cout << "Unit 2 Anthropic Sonnet: " << to_string_view(anthropic_4_5_sonnet::Role::ASSISTANT) << "\n";
}

int main() {
    odr_test_1();
    odr_test_2();
    return 0;
}
