#include "enum_converters.hpp"

namespace jai::llm::providers {

namespace gemini {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::MODEL: return "model";
        case Role::SYSTEM: return "system";
        default: return "";
    }
}
std::string_view to_string(Modality val) {
    switch (val) {
        case Modality::TEXT: return "text";
        case Modality::IMAGE: return "image";
        case Modality::VIDEO: return "video";
        case Modality::AUDIO: return "audio";
        default: return "";
    }
}
std::string_view to_string(ExecutionOutcome val) {
    switch (val) {
        case ExecutionOutcome::OK: return "OUTCOME_OK";
        case ExecutionOutcome::FAILED: return "OUTCOME_FAILED";
        case ExecutionOutcome::DEADLINE_EXCEEDED: return "OUTCOME_DEADLINE_EXCEEDED";
        default: return "OUTCOME_UNSPECIFIED";
    }
}
std::string_view to_string(HarmCategory val) {
    switch (val) {
        case HarmCategory::HARASSMENT: return "HARM_CATEGORY_HARASSMENT";
        case HarmCategory::HATE_SPEECH: return "HARM_CATEGORY_HATE_SPEECH";
        case HarmCategory::SEXUALLY_EXPLICIT: return "HARM_CATEGORY_SEXUALLY_EXPLICIT";
        case HarmCategory::DANGEROUS_CONTENT: return "HARM_CATEGORY_DANGEROUS_CONTENT";
        case HarmCategory::CIVIC_INTEGRITY: return "HARM_CATEGORY_CIVIC_INTEGRITY";
        case HarmCategory::MEDICAL: return "HARM_CATEGORY_MEDICAL";
        case HarmCategory::SEXUAL: return "HARM_CATEGORY_SEXUAL";
        case HarmCategory::PUBLIC_SAFETY: return "HARM_CATEGORY_PUBLIC_SAFETY";
        case HarmCategory::TOXICITY: return "HARM_CATEGORY_TOXICITY";
        case HarmCategory::DEROGATORY: return "HARM_CATEGORY_DEROGATORY";
        case HarmCategory::VIOLENT_CONTENT: return "HARM_CATEGORY_VIOLENT_CONTENT";
        case HarmCategory::SEXUAL_CONTENT: return "HARM_CATEGORY_SEXUAL_CONTENT";
        case HarmCategory::MEDICAL_ADVICE: return "HARM_CATEGORY_MEDICAL_ADVICE";
        case HarmCategory::CIVIC_INTEGRITY_ELECTIONS: return "HARM_CATEGORY_CIVIC_INTEGRITY_ELECTIONS";
        case HarmCategory::HATE_CONTENT: return "HARM_CATEGORY_HATE_CONTENT";
        case HarmCategory::HARASSMENT_CONTENT: return "HARM_CATEGORY_HARASSMENT_CONTENT";
        default: return "";
    }
}
std::string_view to_string(HarmProbability val) {
    switch (val) {
        case HarmProbability::NEGLIGIBLE: return "NEGLIGIBLE";
        case HarmProbability::LOW: return "LOW";
        case HarmProbability::MEDIUM: return "MEDIUM";
        case HarmProbability::HIGH: return "HIGH";
        default: return "UNSPECIFIED";
    }
}
std::string_view to_string(FinishReason val) {
    switch (val) {
        case FinishReason::STOP: return "STOP";
        case FinishReason::MAX_TOKENS: return "MAX_TOKENS";
        case FinishReason::SAFETY: return "SAFETY";
        case FinishReason::RECITATION: return "RECITATION";
        case FinishReason::OTHER: return "OTHER";
        case FinishReason::BLOCKLIST: return "BLOCKLIST";
        case FinishReason::PROHIBITED_CONTENT: return "PROHIBITED_CONTENT";
        case FinishReason::SPII: return "SPII";
        case FinishReason::MALFORMED_FUNCTION_CALL: return "MALFORMED_FUNCTION_CALL";
        default: return "FINISH_REASON_UNSPECIFIED";
    }
}
std::string_view to_string(ThinkingLevel val) {
    switch (val) {
        case ThinkingLevel::MINIMAL: return "MINIMAL";
        case ThinkingLevel::LOW: return "LOW";
        case ThinkingLevel::MEDIUM: return "MEDIUM";
        case ThinkingLevel::HIGH: return "HIGH";
        default: return "THINKING_LEVEL_UNSPECIFIED";
    }
}
std::string_view to_string(MediaResolution val) {
    switch (val) {
        case MediaResolution::LOW: return "LOW";
        case MediaResolution::MEDIUM: return "MEDIUM";
        case MediaResolution::HIGH: return "HIGH";
        default: return "MEDIA_RESOLUTION_UNSPECIFIED";
    }
}
std::string_view to_string(ResponseMimeType val) {
    switch (val) {
        case ResponseMimeType::APPLICATION_JSON: return "application/json";
        case ResponseMimeType::TEXT_X_ENUM: return "text/x.enum";
        case ResponseMimeType::TEXT_PLAIN:
        default: return "text/plain";
    }
}
std::string_view to_string(FunctionCallingMode val) {
    switch (val) {
        case FunctionCallingMode::AUTO: return "AUTO";
        case FunctionCallingMode::ANY: return "ANY";
        case FunctionCallingMode::NONE: return "NONE";
        default: return "MODE_UNSPECIFIED";
    }
}
} // namespace gemini

namespace gemini_2_5 {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::MODEL: return "model";
        case Role::SYSTEM: return "system";
        default: return "";
    }
}
std::string_view to_string(Modality val) {
    switch (val) {
        case Modality::TEXT: return "text";
        case Modality::IMAGE: return "image";
        case Modality::VIDEO: return "video";
        case Modality::AUDIO: return "audio";
        default: return "";
    }
}
std::string_view to_string(ExecutionOutcome val) {
    switch (val) {
        case ExecutionOutcome::OK: return "OUTCOME_OK";
        case ExecutionOutcome::FAILED: return "OUTCOME_FAILED";
        case ExecutionOutcome::DEADLINE_EXCEEDED: return "OUTCOME_DEADLINE_EXCEEDED";
        default: return "OUTCOME_UNSPECIFIED";
    }
}
std::string_view to_string(HarmCategory val) {
    switch (val) {
        case HarmCategory::HARASSMENT: return "HARM_CATEGORY_HARASSMENT";
        case HarmCategory::HATE_SPEECH: return "HARM_CATEGORY_HATE_SPEECH";
        case HarmCategory::SEXUALLY_EXPLICIT: return "HARM_CATEGORY_SEXUALLY_EXPLICIT";
        case HarmCategory::DANGEROUS_CONTENT: return "HARM_CATEGORY_DANGEROUS_CONTENT";
        case HarmCategory::CIVIC_INTEGRITY: return "HARM_CATEGORY_CIVIC_INTEGRITY";
        case HarmCategory::MEDICAL: return "HARM_CATEGORY_MEDICAL";
        case HarmCategory::SEXUAL: return "HARM_CATEGORY_SEXUAL";
        case HarmCategory::PUBLIC_SAFETY: return "HARM_CATEGORY_PUBLIC_SAFETY";
        case HarmCategory::TOXICITY: return "HARM_CATEGORY_TOXICITY";
        case HarmCategory::DEROGATORY: return "HARM_CATEGORY_DEROGATORY";
        case HarmCategory::VIOLENT_CONTENT: return "HARM_CATEGORY_VIOLENT_CONTENT";
        case HarmCategory::SEXUAL_CONTENT: return "HARM_CATEGORY_SEXUAL_CONTENT";
        case HarmCategory::MEDICAL_ADVICE: return "HARM_CATEGORY_MEDICAL_ADVICE";
        case HarmCategory::CIVIC_INTEGRITY_ELECTIONS: return "HARM_CATEGORY_CIVIC_INTEGRITY_ELECTIONS";
        case HarmCategory::HATE_CONTENT: return "HARM_CATEGORY_HATE_CONTENT";
        case HarmCategory::HARASSMENT_CONTENT: return "HARM_CATEGORY_HARASSMENT_CONTENT";
        default: return "";
    }
}
std::string_view to_string(HarmProbability val) {
    switch (val) {
        case HarmProbability::NEGLIGIBLE: return "NEGLIGIBLE";
        case HarmProbability::LOW: return "LOW";
        case HarmProbability::MEDIUM: return "MEDIUM";
        case HarmProbability::HIGH: return "HIGH";
        default: return "UNSPECIFIED";
    }
}
std::string_view to_string(FinishReason val) {
    switch (val) {
        case FinishReason::STOP: return "STOP";
        case FinishReason::MAX_TOKENS: return "MAX_TOKENS";
        case FinishReason::SAFETY: return "SAFETY";
        case FinishReason::RECITATION: return "RECITATION";
        case FinishReason::OTHER: return "OTHER";
        case FinishReason::BLOCKLIST: return "BLOCKLIST";
        case FinishReason::PROHIBITED_CONTENT: return "PROHIBITED_CONTENT";
        case FinishReason::SPII: return "SPII";
        case FinishReason::MALFORMED_FUNCTION_CALL: return "MALFORMED_FUNCTION_CALL";
        default: return "FINISH_REASON_UNSPECIFIED";
    }
}
std::string_view to_string(MediaResolution val) {
    switch (val) {
        case MediaResolution::LOW: return "LOW";
        case MediaResolution::MEDIUM: return "MEDIUM";
        case MediaResolution::HIGH: return "HIGH";
        default: return "MEDIA_RESOLUTION_UNSPECIFIED";
    }
}
std::string_view to_string(ResponseMimeType val) {
    switch (val) {
        case ResponseMimeType::APPLICATION_JSON: return "application/json";
        case ResponseMimeType::TEXT_X_ENUM: return "text/x.enum";
        case ResponseMimeType::TEXT_PLAIN:
        default: return "text/plain";
    }
}
std::string_view to_string(FunctionCallingMode val) {
    switch (val) {
        case FunctionCallingMode::AUTO: return "AUTO";
        case FunctionCallingMode::ANY: return "ANY";
        case FunctionCallingMode::NONE: return "NONE";
        default: return "MODE_UNSPECIFIED";
    }
}
} // namespace gemini_2_5

namespace openai_5 {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::SYSTEM: return "system";
        case Role::ASSISTANT: return "assistant";
        case Role::TOOL: return "tool";
        case Role::DEVELOPER: return "developer";
        default: return "";
    }
}
std::string_view to_string(Modality val) {
    switch (val) {
        case Modality::TEXT: return "text";
        case Modality::IMAGE: return "image";
        case Modality::VIDEO: return "video";
        case Modality::AUDIO: return "audio";
        default: return "";
    }
}
std::string_view to_string(FinishReason val) {
    switch (val) {
        case FinishReason::STOP: return "stop";
        case FinishReason::LENGTH: return "length";
        case FinishReason::CONTENT_FILTER: return "content_filter";
        case FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}
std::string_view to_string(ImageDetail val) {
    switch (val) {
        case ImageDetail::AUTO: return "auto";
        case ImageDetail::LOW: return "low";
        case ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}
std::string_view to_string(ContentPartType val) {
    switch (val) {
        case ContentPartType::TEXT: return "text";
        case ContentPartType::IMAGE_URL: return "image_url";
        case ContentPartType::AUDIO: return "audio";
        case ContentPartType::VIDEO: return "video";
        default: return "";
    }
}
std::string_view to_string(ToolType val) {
    switch (val) {
        case ToolType::FUNCTION: return "function";
        case ToolType::CODE_INTERPRETER: return "code_interpreter";
        case ToolType::FILE_SEARCH: return "file_search";
        default: return "function";
    }
}
std::string_view to_string(ReasoningEffort val) {
    switch (val) {
        case ReasoningEffort::NONE: return "none";
        case ReasoningEffort::MINIMAL: return "minimal";
        case ReasoningEffort::LOW: return "low";
        case ReasoningEffort::MEDIUM: return "medium";
        case ReasoningEffort::HIGH: return "high";
        case ReasoningEffort::XHIGH: return "xhigh";
        default: return "medium";
    }
}
std::string_view to_string(Verbosity val) {
    switch (val) {
        case Verbosity::CONCISE: return "concise";
        case Verbosity::MEDIUM: return "medium";
        case Verbosity::DETAILED: return "detailed";
        default: return "medium";
    }
}
std::string_view to_string(CacheRetention val) {
    switch (val) {
        case CacheRetention::IN_MEMORY: return "in_memory";
        case CacheRetention::HOURS_24: return "24h";
        default: return "in_memory";
    }
}
std::string_view to_string(ResponseFormatType val) {
    switch (val) {
        case ResponseFormatType::TEXT: return "text";
        case ResponseFormatType::JSON_OBJECT: return "json_object";
        case ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}
std::string_view to_string(PredictionType val) {
    switch (val) {
        case PredictionType::CONTENT: return "content";
        default: return "content";
    }
}
std::string_view to_string(ServiceTier val) {
    switch (val) {
        case ServiceTier::SCALE: return "scale";
        case ServiceTier::DEFAULT: return "default";
        default: return "default";
    }
}
std::string_view to_string(ObjectType val) {
    switch (val) {
        case ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}
} // namespace openai_5

namespace openai_4o {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::SYSTEM: return "system";
        case Role::ASSISTANT: return "assistant";
        case Role::TOOL: return "tool";
        case Role::DEVELOPER: return "developer";
        default: return "";
    }
}
std::string_view to_string(Modality val) {
    switch (val) {
        case Modality::TEXT: return "text";
        case Modality::IMAGE: return "image";
        case Modality::VIDEO: return "video";
        case Modality::AUDIO: return "audio";
        default: return "";
    }
}
std::string_view to_string(FinishReason val) {
    switch (val) {
        case FinishReason::STOP: return "stop";
        case FinishReason::LENGTH: return "length";
        case FinishReason::CONTENT_FILTER: return "content_filter";
        case FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}
std::string_view to_string(ImageDetail val) {
    switch (val) {
        case ImageDetail::AUTO: return "auto";
        case ImageDetail::LOW: return "low";
        case ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}
std::string_view to_string(ContentPartType val) {
    switch (val) {
        case ContentPartType::TEXT: return "text";
        case ContentPartType::IMAGE_URL: return "image_url";
        case ContentPartType::AUDIO: return "audio";
        case ContentPartType::VIDEO: return "video";
        default: return "";
    }
}
std::string_view to_string(ToolType val) {
    switch (val) {
        case ToolType::FUNCTION: return "function";
        case ToolType::CODE_INTERPRETER: return "code_interpreter";
        case ToolType::FILE_SEARCH: return "file_search";
        default: return "function";
    }
}
std::string_view to_string(CacheRetention val) {
    switch (val) {
        case CacheRetention::IN_MEMORY: return "in_memory";
        case CacheRetention::HOURS_24: return "24h";
        default: return "in_memory";
    }
}
std::string_view to_string(ResponseFormatType val) {
    switch (val) {
        case ResponseFormatType::TEXT: return "text";
        case ResponseFormatType::JSON_OBJECT: return "json_object";
        case ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}
std::string_view to_string(PredictionType val) {
    switch (val) {
        case PredictionType::CONTENT: return "content";
        default: return "content";
    }
}
std::string_view to_string(ServiceTier val) {
    switch (val) {
        case ServiceTier::SCALE: return "scale";
        case ServiceTier::DEFAULT: return "default";
        default: return "default";
    }
}
std::string_view to_string(ObjectType val) {
    switch (val) {
        case ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}
} // namespace openai_4o

namespace openai_4 {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::SYSTEM: return "system";
        case Role::ASSISTANT: return "assistant";
        case Role::TOOL: return "tool";
        case Role::DEVELOPER: return "developer";
        default: return "";
    }
}
std::string_view to_string(ContentPartType val) {
    switch (val) {
        case ContentPartType::TEXT: return "text";
        case ContentPartType::IMAGE_URL: return "image_url";
        default: return "";
    }
}
std::string_view to_string(ImageDetail val) {
    switch (val) {
        case ImageDetail::AUTO: return "auto";
        case ImageDetail::LOW: return "low";
        case ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}
std::string_view to_string(FinishReason val) {
    switch (val) {
        case FinishReason::STOP: return "stop";
        case FinishReason::LENGTH: return "length";
        case FinishReason::CONTENT_FILTER: return "content_filter";
        case FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}
std::string_view to_string(ToolType val) {
    switch (val) {
        case ToolType::FUNCTION: return "function";
        default: return "function";
    }
}
std::string_view to_string(ResponseFormatType val) {
    switch (val) {
        case ResponseFormatType::TEXT: return "text";
        case ResponseFormatType::JSON_OBJECT: return "json_object";
        case ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}
std::string_view to_string(ObjectType val) {
    switch (val) {
        case ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}
} // namespace openai_4

namespace anthropic_4_5_opus {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::ASSISTANT: return "assistant";
        default: return "";
    }
}
std::string_view to_string(SourceType val) {
    switch (val) {
        case SourceType::BASE64: return "base64";
        case SourceType::URL: return "url";
        default: return "base64";
    }
}
std::string_view to_string(MessageContentPartType val) {
    switch (val) {
        case MessageContentPartType::TEXT: return "text";
        case MessageContentPartType::IMAGE: return "image";
        case MessageContentPartType::AUDIO: return "audio";
        case MessageContentPartType::DOCUMENT: return "document";
        case MessageContentPartType::TOOL_USE: return "tool_use";
        case MessageContentPartType::TOOL_RESULT: return "tool_result";
        default: return "";
    }
}
std::string_view to_string(CacheType val) {
    switch (val) {
        case CacheType::EPHEMERAL: return "ephemeral";
        default: return "ephemeral";
    }
}
std::string_view to_string(ThinkingType val) {
    switch (val) {
        case ThinkingType::ENABLED: return "enabled";
        default: return "enabled";
    }
}
std::string_view to_string(EffortLevel val) {
    switch (val) {
        case EffortLevel::LOW: return "low";
        case EffortLevel::MEDIUM: return "medium";
        case EffortLevel::HIGH: return "high";
        default: return "medium";
    }
}
std::string_view to_string(ToolChoiceType val) {
    switch (val) {
        case ToolChoiceType::AUTO: return "auto";
        case ToolChoiceType::ANY: return "any";
        case ToolChoiceType::TOOL: return "tool";
        default: return "auto";
    }
}
std::string_view to_string(MessageType val) {
    switch (val) {
        case MessageType::MESSAGE: return "message";
        case MessageType::ERROR: return "error";
        default: return "message";
    }
}
std::string_view to_string(ContentBlockType val) {
    switch (val) {
        case ContentBlockType::TEXT: return "text";
        case ContentBlockType::THINKING: return "thinking";
        case ContentBlockType::TOOL_USE: return "tool_use";
        default: return "";
    }
}
std::string_view to_string(StopReason val) {
    switch (val) {
        case StopReason::END_TURN: return "end_turn";
        case StopReason::MAX_TOKENS: return "max_tokens";
        case StopReason::STOP_SEQUENCE: return "stop_sequence";
        case StopReason::TOOL_USE: return "tool_use";
        default: return "";
    }
}
std::string_view to_string(CitationType val) {
    switch (val) {
        case CitationType::CHAR_LOCATION: return "char_location";
        case CitationType::PAGE_LOCATION: return "page_location";
        default: return "";
    }
}
} // namespace anthropic_4_5_opus

namespace anthropic_4_5_sonnet {
std::string_view to_string(Role val) {
    switch (val) {
        case Role::USER: return "user";
        case Role::ASSISTANT: return "assistant";
        default: return "";
    }
}
std::string_view to_string(SourceType val) {
    switch (val) {
        case SourceType::BASE64: return "base64";
        case SourceType::URL: return "url";
        default: return "base64";
    }
}
std::string_view to_string(MessageContentPartType val) {
    switch (val) {
        case MessageContentPartType::TEXT: return "text";
        case MessageContentPartType::IMAGE: return "image";
        case MessageContentPartType::AUDIO: return "audio";
        case MessageContentPartType::DOCUMENT: return "document";
        case MessageContentPartType::TOOL_USE: return "tool_use";
        case MessageContentPartType::TOOL_RESULT: return "tool_result";
        default: return "";
    }
}
std::string_view to_string(CacheType val) {
    switch (val) {
        case CacheType::EPHEMERAL: return "ephemeral";
        default: return "ephemeral";
    }
}
std::string_view to_string(ThinkingType val) {
    switch (val) {
        case ThinkingType::ENABLED: return "enabled";
        default: return "enabled";
    }
}
std::string_view to_string(ToolChoiceType val) {
    switch (val) {
        case ToolChoiceType::AUTO: return "auto";
        case ToolChoiceType::ANY: return "any";
        case ToolChoiceType::TOOL: return "tool";
        default: return "auto";
    }
}
std::string_view to_string(MessageType val) {
    switch (val) {
        case MessageType::MESSAGE: return "message";
        case MessageType::ERROR: return "error";
        default: return "message";
    }
}
std::string_view to_string(ContentBlockType val) {
    switch (val) {
        case ContentBlockType::TEXT: return "text";
        case ContentBlockType::THINKING: return "thinking";
        case ContentBlockType::TOOL_USE: return "tool_use";
        default: return "";
    }
}
std::string_view to_string(StopReason val) {
    switch (val) {
        case StopReason::END_TURN: return "end_turn";
        case StopReason::MAX_TOKENS: return "max_tokens";
        case StopReason::STOP_SEQUENCE: return "stop_sequence";
        case StopReason::TOOL_USE: return "tool_use";
        default: return "";
    }
}
std::string_view to_string(CitationType val) {
    switch (val) {
        case CitationType::CHAR_LOCATION: return "char_location";
        case CitationType::PAGE_LOCATION: return "page_location";
        default: return "";
    }
}
} // namespace anthropic_4_5_sonnet

} // namespace jai::llm::providers
