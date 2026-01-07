#pragma once

#include "gemini_3.hpp"
#include "gemini_2_5.hpp"
#include "openai_5.hpp"
#include "openai_4o.hpp"
#include "openai_4.hpp"
#include "anthropic_4_5_opus.hpp"
#include "anthropic_4_5_sonnet.hpp"
#include <string_view>

namespace jai::llm::providers {

namespace gemini {
std::string_view to_string(Role val);
std::string_view to_string(Modality val);
std::string_view to_string(CodeLanguage val);
std::string_view to_string(MediaType val);
std::string_view to_string(ExecutionOutcome val);
std::string_view to_string(HarmCategory val);
std::string_view to_string(HarmProbability val);
std::string_view to_string(FinishReason val); 
std::string_view to_string(ThinkingLevel val);
std::string_view to_string(MediaResolution val);
std::string_view to_string(ResponseMimeType val);
std::string_view to_string(FunctionCallingMode val);
}

namespace gemini_2_5 {
std::string_view to_string(Role val);
std::string_view to_string(Modality val);
std::string_view to_string(CodeLanguage val);
std::string_view to_string(MediaType val);
std::string_view to_string(ExecutionOutcome val);
std::string_view to_string(HarmCategory val);
std::string_view to_string(HarmProbability val);
std::string_view to_string(FinishReason val);
std::string_view to_string(MediaResolution val);
std::string_view to_string(ResponseMimeType val);
std::string_view to_string(FunctionCallingMode val);
}

namespace openai_5 {
std::string_view to_string(Role val);
std::string_view to_string(Modality val);
std::string_view to_string(FinishReason val);
std::string_view to_string(ImageDetail val);
std::string_view to_string(ContentPartType val);
std::string_view to_string(ToolType val);
std::string_view to_string(ReasoningEffort val);
std::string_view to_string(Verbosity val);
std::string_view to_string(CacheRetention val);
std::string_view to_string(ResponseFormatType val);
std::string_view to_string(PredictionType val);
std::string_view to_string(ServiceTier val);
std::string_view to_string(ObjectType val);
}

namespace openai_4o {
std::string_view to_string(Role val);
std::string_view to_string(Modality val);
std::string_view to_string(FinishReason val);
std::string_view to_string(ImageDetail val);
std::string_view to_string(ContentPartType val);
std::string_view to_string(ToolType val);
std::string_view to_string(CacheRetention val);
std::string_view to_string(ResponseFormatType val);
std::string_view to_string(PredictionType val);
std::string_view to_string(ServiceTier val);
std::string_view to_string(ObjectType val);
}

namespace openai_4 {
std::string_view to_string(Role val);
std::string_view to_string(ContentPartType val);
std::string_view to_string(ImageDetail val);
std::string_view to_string(FinishReason val);
std::string_view to_string(ToolType val);
std::string_view to_string(ResponseFormatType val);
std::string_view to_string(ObjectType val);
}

namespace anthropic_4_5_opus {
std::string_view to_string(Role val);
std::string_view to_string(SourceType val);
std::string_view to_string(MessageContentPartType val);
std::string_view to_string(CacheType val);
std::string_view to_string(ThinkingType val);
std::string_view to_string(EffortLevel val);
std::string_view to_string(ToolChoiceType val);
std::string_view to_string(MessageType val);
std::string_view to_string(ContentBlockType val);
std::string_view to_string(StopReason val);
std::string_view to_string(CitationType val);
}

namespace anthropic_4_5_sonnet {
std::string_view to_string(Role val);
std::string_view to_string(SourceType val);
std::string_view to_string(MessageContentPartType val);
std::string_view to_string(CacheType val);
std::string_view to_string(ThinkingType val);
std::string_view to_string(ToolChoiceType val);
std::string_view to_string(MessageType val);
std::string_view to_string(ContentBlockType val);
std::string_view to_string(StopReason val);
std::string_view to_string(CitationType val);
}

} // namespace jai::llm::providers
